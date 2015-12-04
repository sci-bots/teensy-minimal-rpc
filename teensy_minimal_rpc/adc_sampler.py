# -*- coding: utf-8 -*-
import numpy as np
import pandas as pd
import arduino_helpers.hardware.teensy.adc as adc
import arduino_helpers.hardware.teensy.dma as dma
import arduino_helpers.hardware.teensy.pdb as pdb
import arduino_helpers.hardware.teensy as teensy
import teensy_minimal_rpc.DMA as DMA
import teensy_minimal_rpc.ADC as ADC
import teensy_minimal_rpc.SIM as SIM


def get_adc_configs(F_BUS=48e6, ADC_CLK=22e6):
    '''
    Return `pandas.DataFrame` containing one ADC configuration per row.

    The conversion time is calculated according to the "ConversionTime"
    equation (see 31.4.5.5/681) in `K20P64M72SF1RM` manual.

    **TODO** The conversion times calculated in this function seem to
    differ from those calculated by the Kinetis ADC conversion time
    calculator.  For now, we assume that they are close enough for
    practical use, but there might be some edge cases where
    inappropriate ADC settings may be chosen as a result.
    '''
    from . import package_path

    # Read serialized (HDF) table of all possible ADC configurations (not all
    # valid).
    df_adc_configs = pd.read_hdf(package_path().joinpath('static', 'data',
                                                         'adc_configs.h5'))

    df_adc_configs = (df_adc_configs
                      .loc[(df_adc_configs['CFG2[ADACKEN]'] == 0) &
                           (df_adc_configs['CFG1[ADICLK]'] == 1)])
    df_adc_configs.insert(3, 'CFG1[ADIV]', 0)
    df_adc_configs['ADC_CLK'] = ADC_CLK

    # Maximum ADC clock for 16-bit conversion is 11MHz.
    df_adc_configs.loc[(df_adc_configs['Bit-width'] >= 16) &
                       (df_adc_configs['ADC_CLK'] > 11e6),
                       'ADC_CLK'] = 11e6
    # If the ADC clock is 8MHz or higher, ADHSC (high-speed clock) bit
    # must be set.
    df_adc_configs.loc[df_adc_configs.ADC_CLK >= 8e6, 'CFG2[ADHSC]'] = 1

    conversion_time = (df_adc_configs['bus clock'] / F_BUS +
                       df_adc_configs.extra_us * 1e-6 +
                       1 / df_adc_configs.ADC_CLK
                       * (df_adc_configs.ADCK
                          + df_adc_configs.AverageNum
                          * (df_adc_configs.ADCK_bct +
                             df_adc_configs.ADCK_lst_adder +
                             df_adc_configs.ADCK_hsc_adder)))
    df_adc_configs['conversion_time'] = conversion_time
    df_adc_configs['conversion_rate'] = 1 / conversion_time
    return (df_adc_configs.reset_index(drop=True).drop_duplicates()
            .sort_values('conversion_time'))


DEFAULT_ADC_CONFIGS = get_adc_configs()


class AdcSampler(object):
    def __init__(self, proxy, channels, sample_count,
                 dma_channels=None, adc_number=teensy.ADC_0):
        self.proxy = proxy
        self.channels = channels
        # The number of samples to record for each ADC channel.
        self.sample_count = sample_count
        if dma_channels is None:
            dma_channels = pd.Series([0, 1, 2],
                                     index=['scatter', 'i', 'ii'])
        self.dma_channels = dma_channels
        self.adc_number = adc_number

        # Map Teensy analog channel labels to channels in
        # `ADC_SC1x` format.
        self.channel_sc1as = np.array(adc.SC1A_PINS[channels].tolist(),
                                      dtype='uint32')

        # Enable PDB clock (DMA and ADC clocks should already be enabled).
        self.proxy.update_sim_SCGC6(SIM.R_SCGC6(PDB=True))

        self.allocate_device_arrays()
        self.reset()

        self.configure_adc()
        self.configure_timer(1)

        self.configure_dma_channel_ii_mux()
        self.assert_no_dma_error()

        self.configure_dma_channel_scatter()
        self.assert_no_dma_error()

        self.configure_dma_channel_i()
        self.assert_no_dma_error()

        self.configure_dma_channel_ii()
        self.assert_no_dma_error()

        self.configure_dma_channel_i_mux()
        self.assert_no_dma_error()

    def assert_no_dma_error(self):
        df_dma_registers = self.proxy.DMA_registers()
        assert(df_dma_registers.loc[df_dma_registers
                                    .full_name == 'ERR',
                                    'value'].sum() == 0)

    def allocate_device_arrays(self):
        # Calculate total number of bytes for single scan of ADC channels.
        self.N = np.dtype('uint16').itemsize * self.channel_sc1as.size

        # Use series to store all allocations.  This makes it easy
        # to, e.g., free allocated device memory on clean up.
        self.allocs = pd.Series()

        # Allocate device memory for results from single ADC scan.
        self.allocs['scan_result'] = self.proxy.mem_alloc(self.N)

        # Allocate and copy channel SC1A configurations to device memory.
        self.allocs['sc1as'] =             (self.proxy
             .mem_aligned_alloc_and_set(4,
                                        self.channel_sc1as
                                        .view('uint8')))

        # Allocate device memory for sample buffer for each
        # ADC channel.
        self.allocs['samples'] =             self.proxy.mem_alloc(self.sample_count * self.N)

        # Allocate device memory for DMA TCD configurations.
        # __N.B.,__ Transfer control descriptors are 32 bytes each
        # and MUST be aligned to 0-modulo-32 address.
        self.allocs['tcds'] =             self.proxy.mem_aligned_alloc(32, self.sample_count * 32)
        # Store list of device TCD configuration addresses.
        self.tcd_addrs = [self.allocs.tcds + 32 * i
                          for i in xrange(self.sample_count)]
        # Store list of device TCD register addresses.
        # __N.B.,__ There are 16 DMA channels on the device.
        # __TODO__ Query `proxy` to determine number of DMA channels.
        self.hw_tcd_addrs = [dma.HW_TCDS_ADDR + 32 * i for i in xrange(16)]

    def reset(self):
        # Fill result arrays with zeros
        self.proxy.mem_fill_uint8(self.allocs.scan_result, 0, self.N)
        self.proxy.mem_fill_uint8(self.allocs.samples, 0,
                                  self.sample_count * self.N)

    def configure_dma_channel_scatter(self):
        # Create Transfer Control Descriptor configuration for first chunk, encoded
        # as a Protocol Buffer message.
        tcd0_msg = DMA.TCD(CITER_ELINKNO=DMA.R_TCD_ITER_ELINKNO(ITER=1),
                           BITER_ELINKNO=DMA.R_TCD_ITER_ELINKNO(ITER=1),
                           ATTR=DMA.R_TCD_ATTR(SSIZE=DMA.R_TCD_ATTR._16_BIT,
                                               DSIZE=DMA.R_TCD_ATTR._16_BIT),
                           NBYTES_MLNO=self.N,
                           SADDR=int(self.allocs.scan_result),
                           SOFF=2,
                           SLAST=-self.N,
                           DADDR=int(self.allocs.samples),
                           DOFF=2 * self.sample_count,
                           DLASTSGA=int(self.tcd_addrs[1]),
                           CSR=DMA.R_TCD_CSR(START=0, DONE=False, ESG=True))

        # Convert Protocol Buffer encoded TCD to bytes structure.
        tcd0 = self.proxy.tcd_msg_to_struct(tcd0_msg)

        # Create binary TCD struct for each TCD protobuf message and copy to device
        # memory.
        for i in xrange(self.sample_count):
            tcd_i = tcd0.copy()
            tcd_i['SADDR'] = self.allocs.scan_result
            tcd_i['DADDR'] = self.allocs.samples + 2 * i
            tcd_i['DLASTSGA'] = self.tcd_addrs[(i + 1)
                                               % len(self.tcd_addrs)]
            tcd_i['CSR'] |= (1 << 4)
            if i == (self.sample_count - 1):  # Last sample, so trigger major loop interrupt
                tcd_i['CSR'] |= (1 << 1)  # Set `INTMAJOR` (21.3.29/426)
            self.proxy.mem_cpy_host_to_device(self.tcd_addrs[i],
                                              tcd_i.tostring())

        # Load initial TCD in scatter chain to DMA channel chosen to handle scattering.
        self.proxy.mem_cpy_host_to_device(self.hw_tcd_addrs
                                          [self.dma_channels.scatter],
                                          tcd0.tostring())
        self.proxy.attach_dma_interrupt(self.dma_channels.scatter)

    def configure_dma_channel_i(self):
        sca1_tcd_msg =            DMA.TCD(CITER_ELINKNO=
                    DMA.R_TCD_ITER_ELINKNO(ELINK=False,
                                           ITER=self.channel_sc1as.size),
                    BITER_ELINKNO=
                    DMA.R_TCD_ITER_ELINKNO(ELINK=False,
                                           ITER=self.channel_sc1as.size),
                    ATTR=DMA.R_TCD_ATTR(SSIZE=DMA.R_TCD_ATTR._32_BIT,
                                        DSIZE=DMA.R_TCD_ATTR._32_BIT),
                    NBYTES_MLNO=4,  # `SDA1` register is 4 bytes (32-bit)
                    SADDR=int(self.allocs.sc1as),
                    SOFF=4,
                    SLAST=-self.channel_sc1as.size * 4,
                    DADDR=int(adc.ADC0_SC1A),
                    DOFF=0,
                    DLASTSGA=0,
                    CSR=DMA.R_TCD_CSR(START=0, DONE=False))

        self.proxy.update_dma_TCD(self.dma_channels.i, sca1_tcd_msg)

    def configure_dma_channel_i_mux(self):
        self.proxy.update_dma_mux_chcfg(self.dma_channels.i,
                                        DMA.MUX_CHCFG(SOURCE=
                                                      dma.DMAMUX_SOURCE_PDB,
                                                      TRIG=False,
                                                      ENBL=True))
        self.proxy.update_dma_registers(
            DMA.Registers(SERQ=int(self.dma_channels.i)))

    def configure_adc(self):
        # Select B input for ADC MUX.
        self.proxy.update_adc_registers(
            self.adc_number,
            ADC.Registers(CFG2=ADC.R_CFG2(MUXSEL=ADC.R_CFG2.B)))

    def configure_timer(self, sample_rate_hz):
        # Set PDB interrupt to occur when IDLY is equal to CNT + 1.
        # PDB0_IDLY = 1
        self.proxy.mem_cpy_host_to_device(pdb.PDB0_IDLY,
                                          np.uint32(1).tostring())

        clock_divide = pdb.get_pdb_divide_params(sample_rate_hz).iloc[0]

        # PDB0_MOD = (uint16_t)(mod-1);
        self.proxy.mem_cpy_host_to_device(pdb.PDB0_MOD,
                                          np.uint32(clock_divide.clock_mod)
                                          .tostring())

        PDB_CONFIG = (pdb.PDB_SC_TRGSEL(15)  # Software trigger
                      | pdb.PDB_SC_PDBEN  # Enable PDB
                      | pdb.PDB_SC_CONT  # Continuous
                      | pdb.PDB_SC_LDMOD(0)
                      | pdb.PDB_SC_PRESCALER(clock_divide.prescaler)
                      | pdb.PDB_SC_MULT(clock_divide.mult_)
                      | pdb.PDB_SC_DMAEN  # Enable DMA
                      | pdb.PDB_SC_LDOK)  # Load all new values
        self.proxy.mem_cpy_host_to_device(pdb.PDB0_SC,
                                          np.uint32(PDB_CONFIG)
                                          .tostring())
        return PDB_CONFIG

    def configure_dma_channel_ii_mux(self):
        ### Set DMA mux source for channel to ADC0 ###
        self.proxy.update_dma_mux_chcfg(
            self.dma_channels.ii,
            DMA.MUX_CHCFG(
                # Route ADC0 as DMA channel source.
                SOURCE=dma.DMAMUX_SOURCE_ADC0,
                TRIG=False,# Disable periodic trigger.
                # Enable the DMAMUX configuration for channel.
                ENBL=True))
        self.proxy.enableDMA(teensy.ADC_0)

    def configure_dma_channel_ii(self):
        tcd_msg = DMA.TCD(
            CITER_ELINKYES=
            DMA.R_TCD_ITER_ELINKYES(ELINK=True,
                                    LINKCH=1,
                                    ITER=self.channel_sc1as.size),
            BITER_ELINKYES=
            DMA.R_TCD_ITER_ELINKYES(ELINK=True,
                                    LINKCH=1,
                                    ITER=self.channel_sc1as.size),
            ATTR=DMA.R_TCD_ATTR(SSIZE=DMA.R_TCD_ATTR._16_BIT,
                                DSIZE=DMA.R_TCD_ATTR._16_BIT),
            NBYTES_MLNO=2,  # sizeof(uint16)
            SADDR=adc.ADC0_RA,
            SOFF=0,
            SLAST=0,
            DADDR=int(self.allocs.scan_result),
            DOFF=2,
            DLASTSGA=-self.N,
            CSR=DMA.R_TCD_CSR(START=0, DONE=False,
                              # Start `scatter` DMA channel
                              # after completion of major loop.
                              MAJORELINK=True,
                              MAJORLINKCH=
                              int(self.dma_channels.scatter)))

        self.proxy.update_dma_TCD(self.dma_channels.ii, tcd_msg)

        # DMA request input signals and this enable request flag
        # must be asserted before a channel’s hardware service
        # request is accepted (21.3.3/394).
        self.proxy.update_dma_registers(
            DMA.Registers(SERQ=int(self.dma_channels.ii)))

    def start_read(self, sample_rate_hz):
        '''
        Trigger start of ADC sampling at the specified sampling rate.
        '''
        self.proxy.attach_dma_interrupt(self.dma_channels.scatter)
        pdb_config = self.configure_timer(sample_rate_hz)
        pdb_config |= pdb.PDB_SC_SWTRIG  # Start the counter.

        # Copy configured PDB register state to device hardware register.
        self.proxy.mem_cpy_host_to_device(pdb.PDB0_SC, np.uint32(pdb_config)
                                          .tostring())

        # **N.B.,** Timer will be stopped by the scatter DMA channel major loop
        # interrupt handler after `sample_count` samples have been collected.

    def __dealloc__(self):
        self.allocs[['scan_result', 'samples']].map(self.proxy.mem_free)
        self.allocs[['sc1as', 'tcds']].map(self.proxy.mem_aligned_free)

    def get_results(self):
        data = self.proxy.mem_cpy_device_to_host(self.allocs.samples,
                                                 self.sample_count * self.N)
        df_adc_results = pd.DataFrame(data.view('uint16')
                                      .reshape(-1, self.sample_count).T,
                                      columns=self.channels)
        return df_adc_results
