# -*- coding: utf-8 -*-
from __future__ import division
import datetime as dt
import types
import weakref

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
    Returns
    -------
    pandas.DataFrame
        Table containing one ADC configuration per row.

    Notes
    -----
    The conversion time is calculated according to the "ConversionTime"
    equation (see 31.4.5.5/681) in `K20P64M72SF1RM` manual.

    **TODO** The conversion times calculated in this function seem to differ
    from those calculated by the Kinetis ADC conversion time calculator.  For
    now, we assume that they are close enough for practical use, but there
    might be some edge cases where inappropriate ADC settings may be chosen as
    a result.
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
HW_TCDS_ADDR = 0x40009000
TCD_RECORD_DTYPE = [('SADDR', 'uint32'),
                    ('SOFF', 'uint16'),
                    ('ATTR', 'uint16'),
                    ('NBYTES', 'uint32'),
                    ('SLAST', 'uint32'),
                    ('DADDR', 'uint32'),
                    ('DOFF', 'uint16'),
                    ('CITER', 'uint16'),
                    ('DLASTSGA', 'uint32'),
                    ('CSR', 'uint16'),
                    ('BITER', 'uint16')]

class AdcSampler(object):
    '''
    This class manages configuration of an analog-to-digital converter (ADC)
    and three DMA channels to sample multiple measurements from one or more
    analog input channels.

    Parameters
    ----------
    proxy : teensy_minimal_rpc.proxy.Proxy
    channels : list
        List of labels of analog channels to measure (e.g., ``['A0', 'A3',
        'A1']``).
    sample_count : int
        Number of samples to measure from each channel during each read
        operation.
    dma_channels : list,optional
        List of identifiers of DMA channels to use (default=``[0, 1, 2]``).
    adc_number : int
        Identifier of ADC to use (default=:data:`teensy.ADC_0`)
    '''
    def __init__(self, proxy, channels, sample_count,
                 dma_channels=None, adc_number=teensy.ADC_0):
        # Use weak reference to prevent zombie `proxy` staying alive even after
        # deleting the original `proxy` reference.
        self.proxy = weakref.ref(proxy)
        if isinstance(channels, types.StringTypes):
            # Single channel was specified.  Wrap channel in list.
            channels = [channels]
        self.channels = channels
        # The number of samples to record for each ADC channel.
        self.sample_count = sample_count
        if dma_channels is None:
            dma_channels = pd.Series([0, 1, 2],
                                     index=['scatter', 'adc_channel_configs',
                                            'adc_conversion'])
        self.dma_channels = dma_channels
        self.adc_number = adc_number

        # Map Teensy analog channel labels to channels in
        # `ADC_SC1x` format.
        self.channel_sc1as = np.array(adc.SC1A_PINS[channels].tolist(),
                                      dtype='uint32')

        # Enable PDB clock (DMA and ADC clocks should already be enabled).
        self.proxy().update_sim_SCGC6(SIM.R_SCGC6(PDB=True))

        self.allocate_device_arrays()
        self.reset()

        self.configure_adc()
        self.configure_dma()

    def configure_dma(self):
        self.configure_dma_channel_adc_conversion_mux()
        self.assert_no_dma_error()

        self.configure_dma_channel_scatter()
        self.assert_no_dma_error()

        self.configure_dma_channel_adc_channel_configs()
        self.assert_no_dma_error()

        self.configure_dma_channel_adc_conversion()
        self.assert_no_dma_error()

        self.configure_dma_channel_adc_channel_configs_mux()
        self.assert_no_dma_error()

    def assert_no_dma_error(self):
        df_dma_registers = self.proxy().DMA_registers()
        df_errors = df_dma_registers.loc[(df_dma_registers.full_name == 'ERR')
                                         & (df_dma_registers.value > 0)]
        if df_errors.shape[0] > 0:
            raise IOError('One or more DMA errors occurred.\n%s' % df_errors)

    def allocate_device_arrays(self):
        '''
        Dynamically allocate (using :func:`malloc`) arrays on Teensy device.

        Notes
        -----

            The :meth:`__del__` frees the memory allocated by this method.

        +-------------+------------------------------+-----------------------+
        | Name        | Description                  | Size (bytes)          |
        +=============+==============================+=======================+
        | scan_result | Measurements of single scan  | len(:attr:`channels`) |
        |             | through input channels       | * sizeof(uint16)      |
        +-------------+------------------------------+-----------------------+
        | tcds        | Transfer Control Descriptors | :attr:`sample_count`  |
        |             |                              | * sizeof(uint32)      |
        +-------------+------------------------------+-----------------------+

        For each **analog input channel**:

        +---------+----------------------------------+----------------------+
        | Name    | Description                      | Size (bytes)         |
        +=========+==================================+======================+
        | sc1as   | ``SC1A`` register configuration  | 1                    |
        |         | specifying input channel address |                      |
        +---------+----------------------------------+----------------------+
        | samples | Contiguous measurements for      | :attr:`sample_count` |
        |         | input channel                    | * sizeof(uint16)     |
        +---------+----------------------------------+----------------------+
        '''
        # Calculate total number of bytes for single scan of ADC channels.
        self.N = np.dtype('uint16').itemsize * self.channel_sc1as.size

        # Use series to store all allocations.  This makes it easy
        # to, e.g., free allocated device memory on clean up.
        self.allocs = pd.Series()

        # Allocate device memory for results from single ADC scan.
        self.allocs['scan_result'] = self.proxy().mem_alloc(self.N)

        # Allocate and copy channel SC1A configurations to device memory.
        self.allocs['sc1as'] = (self.proxy()
                                .mem_aligned_alloc_and_set(4,
                                                           self.channel_sc1as
                                                           .view('uint8')))

        # Allocate device memory for sample buffer for each
        # ADC channel.
        self.allocs['samples'] = self.proxy().mem_alloc(self.sample_count *
                                                        self.N)

        # Allocate device memory for DMA TCD configurations. __N.B.,__ Transfer
        # control descriptors are 32 bytes each and MUST be aligned to
        # 0-modulo-32 address.
        self.allocs['tcds'] = self.proxy().mem_aligned_alloc(32,
                                                             self.sample_count
                                                             * 32)
        # Store list of device TCD configuration addresses.
        self.tcd_addrs = [self.allocs.tcds + 32 * i
                          for i in xrange(self.sample_count)]
        # Store list of device TCD register addresses.
        # __N.B.,__ There are 16 DMA channels on the device.
        # __TODO__ Query `proxy` to determine number of DMA channels.
        self.hw_tcd_addrs = [dma.HW_TCDS_ADDR + 32 * i for i in xrange(16)]

    def reset(self):
        '''
        Fill result arrays with zeros.

        See also
        --------
        :meth:`allocate_device_arrays`
        '''
        self.proxy().mem_fill_uint8(self.allocs.scan_result, 0, self.N)
        self.proxy().mem_fill_uint8(self.allocs.samples, 0, self.sample_count *
                                    self.N)

    def configure_dma_channel_adc_channel_configs(self):
        '''
        Configure DMA channel ``adc_channel_configs`` to copy SC1A
        configurations from :attr:`channel_sc1as`, one at a time, to the
        :data:`ADC0_SC1A` register (i.e., ADC Status and Control Register 1).

        See also
        --------
        :meth:`configure_dma_channel_adc_conversion`

        :meth:`configure_dma_channel_scatter`

        Section **ADC Status and Control Registers 1 (ADCx_SC1n) (31.3.1/653)**
        in `K20P64M72SF1RM`_ manual.

        .. _K20P64M72SF1RM: https://www.pjrc.com/teensy/K20P64M72SF1RM.pdf
        '''
        sca1_tcd_msg = \
            DMA.TCD(CITER_ELINKNO=
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

        self.proxy().update_dma_TCD(self.dma_channels.adc_channel_configs,
                                    sca1_tcd_msg)

    def configure_dma_channel_adc_channel_configs_mux(self):
        '''
        Configure DMA channel ``adc_channel_configs`` to trigger based on
        programmable delay block timer.

        See also
        --------
        Section **DMA MUX request sources (3.3.8.1/77)** in `K20P64M72SF1RM`_
        manual.

        Section **DMA channels with periodic triggering capability
        (20.4.1/367)** in `K20P64M72SF1RM`_ manual.

        Section **Channel Configuration register (``DMAMUX_CHCFGn``)
        (20.3.1/366)** in `K20P64M72SF1RM`_ manual.

        .. _K20P64M72SF1RM: https://www.pjrc.com/teensy/K20P64M72SF1RM.pdf
        '''
        # Configure DMA channel `i` enable to use MUX triggering from
        # programmable delay block.
        self.proxy().update_dma_mux_chcfg(self.dma_channels.adc_channel_configs,
                                          DMA.MUX_CHCFG(SOURCE=
                                                        dma.DMAMUX_SOURCE_PDB,
                                                        TRIG=False, ENBL=True))

        # Set enable request for DMA channel `i`.
        #
        # [1]: https://www.pjrc.com/teensy/K20P64M72SF1RM.pdf
        self.proxy().update_dma_registers(
            DMA.Registers(SERQ=int(self.dma_channels.adc_channel_configs)))

    def configure_dma_channel_adc_conversion_mux(self):
        '''
        Set mux source for DMA channel ``adc_conversion`` to ADC0 and enable
        DMA for ADC0.

        See also
        --------
        Section **Channel Configuration register (``DMAMUX_CHCFGn``)
        (20.3.1/366)** in `K20P64M72SF1RM`_ manual.

        Section **Status and Control Register 2 (ADCx_SC2) (31.3.6/661)** in
        `K20P64M72SF1RM`_ manual.

        .. _K20P64M72SF1RM: https://www.pjrc.com/teensy/K20P64M72SF1RM.pdf
        '''
        self.proxy().update_dma_mux_chcfg(
            self.dma_channels.adc_conversion,
            DMA.MUX_CHCFG(
                # Route ADC0 as DMA channel source.
                SOURCE=dma.DMAMUX_SOURCE_ADC0,
                TRIG=False,# Disable periodic trigger.
                # Enable the DMAMUX configuration for channel.
                ENBL=True))
        # Update ADC0_SC2 to enable DMA and assert the ADC DMA request during
        # an ADC conversion complete event noted when any of the `SC1n[COCO]`
        # (i.e., conversion complete) flags is asserted.
        self.proxy().enableDMA(teensy.ADC_0)

    def configure_dma_channel_adc_conversion(self):
        '''
        Configure DMA channel ``adc_conversion`` to:

         - Copy result after completion of each ADC conversion to subsequent
           locations in :attr:`allocs.scan_result` array.
         - Trigger DMA channel ``adc_channel_configs`` after each ADC
           conversion to copy next ADC configuration to SC1A register (i.e.,
           ADC Status and Control Register 1).
         - Trigger DMA channel ``scatter`` to start after completion of major
           loop.

        Notes
        -----
            After starting PDB timer with :meth:`start_read`, DMA channel
            ``adc_conversion`` will continue to scan analog input channels
            until the PDB timer is stopped.  The handler for the completed
            scatter DMA interrupt currently stops the PDB timer.

        See also
        --------
        :meth:`configure_dma_channel_adc_channel_configs`

        :meth:`configure_dma_channel_scatter`

        Section **ADC Status and Control Registers 1 (ADCx_SC1n) (31.3.1/653)**
        in `K20P64M72SF1RM`_ manual.

        .. _K20P64M72SF1RM: https://www.pjrc.com/teensy/K20P64M72SF1RM.pdf
        '''
        # **NOTE**: "When the CITER field is initially loaded by software, it
        # must be set to the same value as that contained in the BITER field."
        # CITER is current major iteration count, and BITER is the
        # starting/beginning major iteration count.
        #
        # See `CITER` field section **TCD Current Minor Loop Link, Major Loop
        # Count (Channel Linking Disabled) (`DMA_TCDn_CITER_ELINKNO`)
        # (21.3.27/423)
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

        self.proxy().update_dma_TCD(self.dma_channels.adc_conversion, tcd_msg)

        # DMA request input signals and this enable request flag
        # must be asserted before a channel’s hardware service
        # request is accepted (21.3.3/394).
        self.proxy().update_dma_registers(
            DMA.Registers(SERQ=int(self.dma_channels.adc_conversion)))

    def configure_dma_channel_scatter(self):
        '''
        Configure a Transfer Control Descriptor structure for *each scan* of
        the analog input channels, copy TCD structures to device, and attach
        DMA interrupt handler to scatter DMA channel.

        Notes
        -----
        To measure :attr:`sample_count` measurements from each analog input
        channel in :attr:`channels`, :attr:`sample_count` consecutive scans
        through the analog channels are performed.

        A Transfer Control Descriptor structure is configured for *each scan*
        to the scatters the results from the scan to the next index position in
        the samples array of each analog input channel.

        See also
        --------
        :meth:`allocate_device_arrays`

        :meth:`configure_dma_channel_adc_channel_configs`

        :meth:`configure_dma_channel_adc_conversion`
        '''
        # Create Transfer Control Descriptor configuration for first chunk, encoded
        # as a Protocol Buffer message.
        tcd0_msg = DMA.TCD(CITER_ELINKNO=DMA.R_TCD_ITER_ELINKNO(ITER=1),
                           BITER_ELINKNO=DMA.R_TCD_ITER_ELINKNO(ITER=1),
                           ATTR=DMA.R_TCD_ATTR(SSIZE=DMA.R_TCD_ATTR._16_BIT,
                                               DSIZE=DMA.R_TCD_ATTR._16_BIT),
                           # N=analog input channels * sizeof(uint16_t)
                           NBYTES_MLNO=self.N,
                           SADDR=int(self.allocs.scan_result),
                           SOFF=2,
                           SLAST=-self.N,
                           DADDR=int(self.allocs.samples),
                           DOFF=2 * self.sample_count,
                           DLASTSGA=int(self.tcd_addrs[1]),
                           CSR=DMA.R_TCD_CSR(START=0, DONE=False, ESG=True))

        # Convert Protocol Buffer encoded TCD to bytes structure (see
        # `TCD_RECORD_DTYPE`).
        tcd0 = self.proxy().tcd_msg_to_struct(tcd0_msg)

        # Create binary TCD struct for each TCD protobuf message and copy to
        # device memory.
        for i in xrange(self.sample_count):
            tcd_i = tcd0.copy()
            # Copy from `scan_result` array.
            tcd_i['SADDR'] = self.allocs.scan_result
            # Perform strided copy to next available `samples` location for
            # each analog input channel.
            tcd_i['DADDR'] = self.allocs.samples + 2 * i
            # After copying is finished, load Transfer Control Descriptor for
            # next sample scan.
            tcd_i['DLASTSGA'] = self.tcd_addrs[(i + 1)
                                               % len(self.tcd_addrs)]
            tcd_i['CSR'] |= (1 << 4)
            if i == (self.sample_count - 1):
                # Last sample, so trigger major loop interrupt
                tcd_i['CSR'] |= (1 << 1)  # Set `INTMAJOR` (21.3.29/426)
            # Copy TCD for sample number `i` to device.
            self.proxy().mem_cpy_host_to_device(self.tcd_addrs[i],
                                                tcd_i.tostring())

        # Load initial TCD in scatter chain to DMA channel chosen to handle
        # scattering.
        self.proxy().mem_cpy_host_to_device(self.hw_tcd_addrs
                                            [self.dma_channels.scatter],
                                            tcd0.tostring())
        # Attach interrupt handler to scatter DMA channel.
        self.proxy().attach_dma_interrupt(self.dma_channels.scatter)

    def configure_adc(self):
        '''
        Select ``b`` input for ADC MUX.

        Notes
        -----
            It seems like this has to do with chip pinout configuration, where
            on the Teensy 3.2, the ``b`` ADC inputs are used?

        See also
        --------
        Section **ADC Configuration Register 2 (``ADCx_CFG2``) (31.3.3/658)**
        in `K20P64M72SF1RM`_ manual.

        Section **ADC0 Channel Assignment for 64-Pin Package (3.7.1.3.1.1/98)**
        in `K20P64M72SF1RM`_ manual.

        Section **K20 Signal Multiplexing and Pin Assignments (10.3.1/207)** in
        `K20P64M72SF1RM`_ manual.

        .. _K20P64M72SF1RM: https://www.pjrc.com/teensy/K20P64M72SF1RM.pdf
        '''
        self.proxy().update_adc_registers(
            self.adc_number,
            ADC.Registers(CFG2=ADC.R_CFG2(MUXSEL=ADC.R_CFG2.B)))

    def configure_timer(self, sample_rate_hz):
        '''
        Configure programmable delay block according to specified sampling
        rate.

        Notes
        -----

            Enable software trigger, continuous mode, and **generate a DMA
            request** instead of an interrupt **when the programmed delay has
            passed**.

        Parameters
        ----------
        sample_rate_hz : int
            Sample rate in Hz.

        Returns
        -------
        int
            Programmable delay block configuration

        See also
        --------
        Chapter 35 **Programmable Delay Block (PDB) (35.1/747)** in
        `K20P64M72SF1RM`_ manual.

        .. _K20P64M72SF1RM: https://www.pjrc.com/teensy/K20P64M72SF1RM.pdf
        '''
        # Set PDB interrupt to occur when IDLY is equal to CNT + 1.
        # PDB0_IDLY = 1
        self.proxy().mem_cpy_host_to_device(pdb.PDB0_IDLY,
                                            np.uint32(1).tostring())

        clock_divide = pdb.get_pdb_divide_params(sample_rate_hz).iloc[0]

        # PDB0_MOD = (uint16_t)(mod-1);
        self.proxy().mem_cpy_host_to_device(pdb.PDB0_MOD,
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
        self.proxy().mem_cpy_host_to_device(pdb.PDB0_SC, np.uint32(PDB_CONFIG)
                                            .tostring())
        return PDB_CONFIG

    def start_read(self, sample_rate_hz, stream_id=0):
        '''
        Trigger start of ADC sampling at the specified sampling rate.

         1. Start PDB timer according to specified sample rate (in Hz).
         2. Each completion of the PDB timer triggers DMA channel
            ``adc_channel_configs`` request.
         3. Each request to DMA channel ``adc_channel_configs`` copies the
            configuration for an analog input channel to SC1A register (i.e.,
            ADC Status and Control Register 1), which triggers analog
            conversion.
         4. Completion of analog conversion triggers DMA channel
            ``adc_conversion`` to copy result from ``ADC0_RA`` to the
            respective position in the :data:``scan_result`` array.
         5. After each scan through **all channels** (i.e., after each DMA
            channel ``adc_conversion`` major loop completion), trigger for DMA
            channel ``scatter``.
         6. Each request to the ``scatter`` DMA channel scatters results from
            one scan pass to append onto a separate array for each analog input
            channel.
         7. Steps 2-6 repeat :attr:`sample_count` times.

        Parameters
        ----------
        sample_rate_hz : int
            Sample rate in Hz.
        stream_id : int, optional
            Stream identifier.

            May be passed to :method:`get_results_async` to filter related DMA
            data.

        See also
        --------

        :method:`get_results`
        :method:`get_results_async`

        Section **ADC Status and Control Registers 1 (ADCx_SC1n) (31.3.1/653)**
        in `K20P64M72SF1RM`_ manual.

        .. _K20P64M72SF1RM: https://www.pjrc.com/teensy/K20P64M72SF1RM.pdf
        '''
        self.proxy().attach_dma_interrupt(self.dma_channels.scatter)
        pdb_config = self.configure_timer(sample_rate_hz)
        pdb_config |= pdb.PDB_SC_SWTRIG  # Start the counter.

        # Copy configured PDB register state to device hardware register.
        self.proxy().start_dma_adc(np.uint32(pdb_config), self.allocs.samples,
                                   self.sample_count * self.N, stream_id)
        self.sample_rate_hz = sample_rate_hz

    def get_results(self):
        '''
        Returns
        -------
        pandas.DataFrame
            Table containing :attr:`sample_count` ADC readings for each analog
            input channel.

        Notes
        -----
            **Does not guarantee result is ready!**

            **TODO** Provide mechanism to poll status of previously started
            read.
        '''
        data = self.proxy().mem_cpy_device_to_host(self.allocs.samples,
                                                   self.sample_count * self.N)
        df_adc_results = pd.DataFrame(data.view('uint16')
                                      .reshape(-1, self.sample_count).T,
                                      columns=self.channels)
        return df_adc_results

    def get_results_async(self, stream_id=None):
        '''
        Returns
        -------
        pandas.DataFrame
            Table containing :attr:`sample_count` ADC readings for each analog
            input channel, indexed by:

             - ADC DMA stream identifier (i.e., ``stream_id``).
             - Measurement timestamp.

        Notes
        -----
            **Does not guarantee result is ready!**
        '''
        stream_queue = self.proxy()._packet_watcher.queues.stream
        frames = []
        packet_count = stream_queue.qsize()
        for i in xrange(packet_count):
            datetime_i, packet_i = stream_queue.get_nowait()
            if stream_id is not None and stream_id != packet_i.iuid:
                # Packet does not match specified stream ID.
                stream_queue.put_nowait((datetime_i, packet_i))
                continue
            datetimes_i = [datetime_i + dt.timedelta(seconds=t_j)
                           for t_j in np.arange(self.sample_count) *
                           1. / self.sample_rate_hz]
            df_adc_results_i = pd.DataFrame(np.fromstring(packet_i.data(),
                                                          dtype='uint16')
                                            .reshape(-1, self.sample_count).T,
                                            columns=self.channels,
                                            index=datetimes_i)
            df_adc_results_i.insert(0, 'stream_id', packet_i.iuid)
            frames.append(df_adc_results_i)
        if not frames:
            return pd.DataFrame(None, columns=self.channels)
        return (pd.concat(frames).set_index('stream_id', append=True)
                .reorder_levels(['stream_id', 0]))

    def __del__(self):
        self.allocs[['scan_result', 'samples']].map(self.proxy().mem_free)
        self.allocs[['sc1as', 'tcds']].map(self.proxy().mem_aligned_free)


class AdcDmaMixin(object):
    '''
    This mixin class implements DMA-enabled analog-to-digital converter (ADC)
    methods.
    '''
    def init_dma(self):
        '''
        Initialize eDMA engine.  This includes:

         - Enabling clock gating for DMA and DMA mux.
         - Resetting all DMA channel transfer control descriptors.

        See the following sections in [K20P64M72SF1RM][1] for more
        information:

         - (12.2.13/256) System Clock Gating Control Register 6 (SIM_SCGC6)
         - (12.2.14/259) System Clock Gating Control Register 7 (SIM_SCGC7)
         - (21.3.17/415) TCD registers

        [1]: https://www.pjrc.com/teensy/K20P64M72SF1RM.pdf
        '''
        from teensy_minimal_rpc.SIM import R_SCGC6, R_SCGC7

        # Enable DMA-related clocks in clock-gating configuration
        # registers.
        # SIM_SCGC6 |= SIM_SCGC6_DMAMUX;
        self.update_sim_SCGC6(R_SCGC6(DMAMUX=True))
        # SIM_SCGC7 |= SIM_SCGC7_DMA;
        self.update_sim_SCGC7(R_SCGC7(DMA=True))

        # Reset all DMA transfer control descriptor registers (i.e., set to
        # 0).
        for i in xrange(self.dma_channel_count()):
            self.reset_dma_TCD(i)

    def DMA_TCD(self, dma_channel):
        '''
        Parameters
        ----------
        dma_channel : int
            DMA channel for which to read the Transfer Control Descriptor.

        Returns
        -------
        pandas.DataFrame
            Table of Transfer Control Descriptor fields with corresponding
            values.

            In addition, several reference columns are included.  For
            example, the ``page`` column indicates the page in the
            [reference manual][1] where the respective register field is
            described.

        [1]: http://www.freescale.com/products/arm-processors/kinetis-cortex-m/adc-calculator:ADC_CALCULATOR
        '''
        from arduino_rpc.protobuf import resolve_field_values
        import arduino_helpers.hardware.teensy.dma as dma
        from teensy_minimal_rpc.DMA import TCD

        tcd = TCD.FromString(self.read_dma_TCD(dma_channel).tostring())
        df_tcd = resolve_field_values(tcd)
        return (df_tcd[['full_name', 'value']].dropna()
                .join(dma.TCD_DESCRIPTIONS, on='full_name')
                [['full_name', 'value', 'short_description', 'page']]
                .sort_values(['page','full_name']))

    def DMA_registers(self):
        '''
        Returns
        -------
        pandas.DataFrame
            Table of DMA configuration fields with corresponding values.

            In addition, several reference columns are included.  For
            example, the ``page`` column indicates the page in the
            [reference manual][1] where the respective register field is
            described.

        [1]: http://www.freescale.com/products/arm-processors/kinetis-cortex-m/adc-calculator:ADC_CALCULATOR
        '''
        from arduino_rpc.protobuf import resolve_field_values
        import arduino_helpers.hardware.teensy.dma as dma
        from teensy_minimal_rpc.DMA import Registers

        dma_registers = (Registers
                            .FromString(self.read_dma_registers().tostring()))
        df_dma = resolve_field_values(dma_registers)
        return (df_dma.dropna(subset=['value'])
                .join(dma.REGISTERS_DESCRIPTIONS, on='full_name')
                [['full_name', 'value', 'short_description', 'page']])

    def tcd_msg_to_struct(self, tcd_msg):
        '''
        Convert Transfer Control Descriptor from Protocol Buffer message
        encoding to raw structure, i.e., 32 bytes starting from `SADDR`
        field.

        See 21.3.17/415 in the [manual][1] for more details.

        Parameters
        ----------
        tcd_msg : teensy_minimal_rpc.DMA.TCD
            Transfer Control Descriptor Protocol Buffer message.

        Returns
        -------
        numpy.ndarray(dtype=TCD_RECORD_DTYPE)
            Raw Transfer Control Descriptor structure (i.e., 32 bytes
            starting from ``SADDR`` field) as a :class:`numpy.ndarray` of
            type :data:`TCD_RECORD_DTYPE`.

        See also
        --------
        :meth:`tcd_struct_to_msg`

        [1]: https://www.pjrc.com/teensy/K20P64M72SF1RM.pdf
        '''
        # Copy TCD to device so that we can extract the raw bytes from
        # device memory (raw TCD bytes are read into `tcd0`.
        #
        # **TODO**:
        #  - Modify `TeensyMinimalRpc/DMA.h::serialize_TCD` and
        #  `TeensyMinimalRpc/DMA.h::update_TCD` functions to work offline.
        #      * Operate on variable by reference, on-device use actual register.
        #  - Add `arduino_helpers.hardware.teensy` function to convert
        #    between TCD protobuf message and binary TCD struct.
        self.update_dma_TCD(0, tcd_msg)
        return (self.mem_cpy_device_to_host(HW_TCDS_ADDR, 32)
                .view(TCD_RECORD_DTYPE)[0])

    def tcd_struct_to_msg(self, tcd_struct):
        '''
        Convert Transfer Control Descriptor from raw structure (i.e., 32
        bytes starting from ``SADDR`` field) to Protocol Buffer message
        encoding.

        See 21.3.17/415 in the [manual][1] for more details.

        Parameters
        ----------
        tcd_struct : numpy.ndarray
            Raw Transfer Control Descriptor structure (i.e., 32 bytes
            starting from ``SADDR`` field)

        Returns
        -------
        teensy_minimal_rpc.DMA.TCD
            Transfer Control Descriptor Protocol Buffer message.

        See also
        --------
        :meth:`tcd_msg_to_struct`

        [1]: https://www.pjrc.com/teensy/K20P64M72SF1RM.pdf
        '''
        from teensy_minimal_rpc.DMA import TCD

        # Copy TCD structure to device so that we can extract the
        # serialized protocol buffer representation from the device.
        #
        # **TODO**:
        #  - Modify `TeensyMinimalRpc/DMA.h::serialize_TCD` and
        #    `TeensyMinimalRpc/DMA.h::update_TCD` functions to operate on
        #    arbitrary source address (instead of hard-coded address of TCD
        #    register).
        #      * Operate on variable by reference, on-device use actual register.
        #  - Add `arduino_helpers.hardware.teensy` function to convert between TCD
        #    protobuf message and binary TCD struct.
        self.mem_cpy_host_to_device(HW_TCDS_ADDR, tcd_struct.tostring())
        return TCD.FromString(self.read_dma_TCD(0).tostring())

    def analog_reads_config(self, adc_channels, sample_count,
                            resolution=None, average_count=1,
                            sampling_rate_hz=None, differential=False,
                            gain_power=0, adc_num=teensy.ADC_0):
        '''
        Configure ADC sampler to read multiple samples from a single ADC
        channel, using the minimum conversion rate for specified sampling
        parameters.

        The reasoning behind selecting the minimum conversion rate is that
        we expect it to lead to the lowest noise possible while still
        matching the specified requirements.
        **TODO** Should this be handled differently?

        This function uses the following mutator methods:

         - :meth:`setReference` (C++)
             * Set the reference voltage based on whether or not
               differential is selected.

               On the Teensy 3.2 architecture, the 1.2V reference voltage
               *must* be used when operating in differential (as opposed to
               singled-ended) mode.

         - :meth:`update_adc_registers` (C++)
             * Apply ADC ``CFG*`` register settings.

               ADC ``CFG*`` register settings are determined by:

                 - :data:`average_count`
                 - :data:`differential`
                 - :data:`gain_power` (PGA only)
                 - :data:`resolution`
                 - :data:`sampling_rate_hz`

         - :meth:`setAveraging` (C++)
             * Apply ADC hardware averaging setting using Teensy ADC
               library API.

               We use the Teensy API here because it handles calibration,
               etc. automatically.

         - :meth:`setResolution` (C++)
             * Apply ADC resolution setting using Teensy ADC library API.

               We use the Teensy API here because it handles calibration,
               etc. automatically.

        Parameters
        ----------
        adc_channels : string or list
            ADC channel to measure (e.g., ``'A0'``, ``'PGA0'``, etc.).

            Multiple channels may be specified to perform interleaved reads.
        sample_count : int
            Number of samples to measure.
        resolution : int,optional
            Bit resolution to sample at.  Must be one of: 8, 10, 12, 16.
        average_count : int,optional
            Hardware average count.
        sampling_rate_hz : int,optional
            Sampling rate.  If not specified, sampling rate will be based
            on minimum conversion rate based on remaining ADC settings.
        differential : bool,optional
            If ``True``, use differential mode.  Otherwise, use single-ended
            mode.
            .. note::
                **Differential mode** is automatically enabled for ``PGA*``
                measurements (e.g., :data:`adc_channel=='PGA0'`).
        gain_power : int,optional
            When measuring a ``'PGA*'`` channel (also implies differential
            mode), apply a gain of ``2^gain_power`` using the hardware
            programmable amplifier gain.
        adc_num : int,optional
            The ADC to use for the measurement (default is
            ``teensy.ADC_0``).

        Returns
        -------
        sampling_rate_hz : int
            Number of samples per second.
        adc_settings : pandas.Series
            ADC settings used.
        adc_sampler : teensy_minimal_rpc.adc_sampler.AdcSampler
            An ADC sampler object.

            Calls to the
            :meth:`teensy_minimal_rpc.adc_sampler.AdcSampler.start_read`
            method asynchronously initiate reading of the configured
            channels/sample count, at a specified sampling rate.

            The
            :meth:`teensy_minimal_rpc.adc_sampler.AdcSampler.get_results_async`
            method may be called to fetch results from a previously
            initiated read operation.
        '''
        from .adc_sampler import AdcSampler, DEFAULT_ADC_CONFIGS
        import teensy_minimal_rpc.ADC as ADC

        # Select ADC settings to achieve minimum conversion rate for
        # specified resolution, mode (i.e., single-ended or differential),
        # and number of samples to average per conversion (i.e., average
        # count).
        bit_width = resolution

        if isinstance(adc_channels, types.StringTypes):
            # Single channel was specified.  Wrap channel in list.
            adc_channels = [adc_channels]

        # Enable programmable gain if `'PGA'` is part of any selected channel
        # name.
        enabled_programmable_gain = any('PGA' in channel_i
                                        for channel_i in adc_channels)
        if enabled_programmable_gain:
            differential = True

        if differential:
            if (resolution is not None) and ((resolution < 16) and not
                                             (resolution & 0x01)):
                # An even number of bits was specified for resolution in
                # differential mode. However, bit-widths are actually
                # increased by one bit, where the additional bit indicates
                # the sign of the result.
                bit_width += 1

        elif gain_power > 0:
            raise ValueError('Programmable gain amplification is only '
                             'valid in differential mode.')
        mode = 'differential' if differential else 'single-ended'

        # Build up a query mask based on specified options.
        query = ((DEFAULT_ADC_CONFIGS.AverageNum == average_count)
                & (DEFAULT_ADC_CONFIGS.Mode == mode))
        if resolution is not None:
            query &= (DEFAULT_ADC_CONFIGS['Bit-width'] == bit_width)
        if sampling_rate_hz is not None:
            query &= (DEFAULT_ADC_CONFIGS.conversion_rate >=
                      sampling_rate_hz)

        # Use prepared query mask to select matching settings from the
        # table of valid ADC configurations.
        matching_settings = DEFAULT_ADC_CONFIGS.loc[query]

        # Find and select the ADC configuration with the minimum conversion
        # rate.
        # **TODO** The reasoning behind selecting the minimum conversion
        # rate is that we expect it to lead to the lowest noise possible
        # while still matching the specified requirements.
        min_match_index = matching_settings['conversion_rate'].argmin()
        adc_settings = matching_settings.loc[min_match_index].copy()

        if resolution is None:
            resolution = int(adc_settings['Bit-width'])

        # Set the reference voltage based on whether or not differential is
        # selected.
        if differential:
            # On the Teensy 3.2 architecture, the 1.2V reference voltage
            # *must* be used when operating in differential (as opposed to
            # singled-ended) mode.
            self.setReference(teensy.ADC_REF_1V2, adc_num)
            reference_V = 1.2
        else:
            self.setReference(teensy.ADC_REF_3V3, adc_num)
            reference_V = 3.3

        adc_settings['reference_V'] = reference_V
        adc_settings['resolution'] = resolution
        adc_settings['differential'] = differential

        # Verify that a valid gain value has been specified.
        assert(gain_power >= 0 and gain_power < 8)
        adc_settings['gain_power'] = int(gain_power)

        # Construct a Protocol Buffer message according to the selected ADC
        # configuration settings.
        adc_registers = \
            ADC.Registers(CFG2=
                          ADC.R_CFG2(MUXSEL=ADC.R_CFG2.B,
                                     ADACKEN=
                                     int(adc_settings['CFG2[ADACKEN]']),
                                     ADLSTS=
                                     int(adc_settings['CFG2[ADLSTS]']),
                                     ADHSC=
                                     int(adc_settings['CFG2[ADHSC]'])),
                          CFG1=
                          ADC.R_CFG1(ADLSMP=
                                     int(adc_settings['CFG1[ADLSMP]']),
                                     ADICLK=
                                     int(adc_settings['CFG1[ADICLK]']),
                                     ADIV=
                                     int(adc_settings['CFG1[ADIV]'])))
        if enabled_programmable_gain:
            adc_registers.PGA.PGAG = adc_settings.gain_power
            adc_registers.PGA.PGAEN = True

        # Apply ADC `CFG*` register settings.
        self.update_adc_registers(adc_num, adc_registers)

        # Apply non-`CFG*` ADC settings using Teensy ADC library API.
        # We use the Teensy API here because it handles calibration, etc.
        # automatically.
        self.setAveraging(average_count, adc_num)
        self.setResolution(resolution, adc_num)

        if sampling_rate_hz is None:
            # By default, use a sampling rate that is 90% of the maximum
            # conversion rate for the selected ADC settings.
            #
            # XXX We arrived at this after a handful of empirical
            # tests. We think this is necessary due to the slight deviation
            # of the computed conversion rates (see TODO in
            # `adc_sampler.get_adc_configs`) from those computed by the
            # [Freescale ADC calculator][1].
            #
            # [1]: http://www.freescale.com/products/arm-processors/kinetis-cortex-m/adc-calculator:ADC_CALCULATOR
            sampling_rate_hz = (int(.9 * adc_settings.conversion_rate) &
                                0xFFFFFFFE)

        # Create `AdcSampler` for:
        #  - The specified sample count.
        #  - The specified channel.
        #      * **N.B.,** The `AdcSampler` supports scanning multiple
        #        channels, but in this function, we're only reading from a
        #        single channel.

        # **Creation of `AdcSampler` object initializes DMA-related
        # registers**.
        adc_sampler = AdcSampler(self, adc_channels, sample_count)
        return sampling_rate_hz, adc_settings, adc_sampler

    def analog_reads(self, adc_channels, sample_count, resolution=None,
                     average_count=1, sampling_rate_hz=None,
                     differential=False, gain_power=0,
                     adc_num=teensy.ADC_0, timeout_s=None):
        '''
        Read multiple samples from a single ADC channel, using the minimum
        conversion rate for specified sampling parameters.

        Parameters
        ----------
        adc_channels : string or list
            ADC channel to measure (e.g., ``'A0'``, ``'PGA0'``, etc.).

            Multiple channels may be specified to perform interleaved reads.
        sample_count : int
            Number of samples to measure.
        resolution : int,optional
            Bit resolution to sample at.  Must be one of: 8, 10, 12, 16.
        average_count : int,optional
            Hardware average count.
        sampling_rate_hz : int,optional
            Sampling rate.  If not specified, sampling rate will be based
            on minimum conversion rate based on remaining ADC settings.
        differential : bool,optional
            If ``True``, use differential mode.  Otherwise, use single-ended
            mode.
            .. note::
                **Differential mode** is automatically enabled for ``PGA*``
                measurements (e.g., :data:`adc_channel=='PGA0'`).
        gain_power : int,optional
            When measuring a ``'PGA*'`` channel (also implies differential
            mode), apply a gain of ``2^gain_power`` using the hardware
            programmable amplifier gain.
        adc_num : int,optional
            The ADC to use for the measurement (default is
            ``teensy.ADC_0``).

        Returns
        -------
        sampling_rate_hz : int
            Number of samples per second.
        adc_settings : pandas.Series
            ADC settings used.
        df_volts : pandas.DataFrame
            Voltage readings (based on reference voltage and gain).
        df_adc_results : pandas.DataFrame
            Raw ADC values (range depends on resolution, i.e.,
            ``adc_settings['Bit-width']``).

        See also
        --------
        :meth:`analog_reads_config`
        '''
        sample_rate_hz, adc_settings, adc_sampler = \
            self.analog_reads_config(adc_channels, sample_count,
                                     resolution=resolution,
                                     average_count=average_count,
                                     sampling_rate_hz=sampling_rate_hz,
                                     differential=differential,
                                     gain_power=gain_power,
                                     adc_num=adc_num)
        adc_sampler.start_read(sample_rate_hz=sample_rate_hz)
        start_time = dt.datetime.now()
        while self._packet_watcher.queues.stream.qsize() < 1:
            if (timeout_s is not None and (timeout_s <
                                           (dt.datetime.now() -
                                            start_time).total_seconds())):
                raise IOError('Timed out waiting for streamed result.')
        df_adc_results = adc_sampler.get_results_async()
        return (sample_rate_hz, adc_settings) + \
            format_adc_results(df_adc_results, adc_settings)


def format_adc_results(df_adc_results, adc_settings):
    '''
    Convert raw ADC readings to actual voltages.

    Parameters
    ----------
    adc_settings : pandas.Series
        ADC settings used.
    df_adc_results : pandas.DataFrame
        Raw ADC values (range depends on resolution, i.e.,
        ``adc_settings['Bit-width']``).

    Returns
    -------
    df_volts : pandas.DataFrame
        Voltage readings (based on reference voltage and gain).
    df_adc_results : pandas.DataFrame
        Raw ADC values (from input argument).
    '''
    dtype = 'int16' if adc_settings.differential else 'uint16'
    df_adc_results = df_adc_results.astype(dtype)
    scale = adc_settings.reference_V / (1 << (adc_settings.resolution +
                                                adc_settings.gain_power))
    df_volts = scale * df_adc_results
    return df_volts, df_adc_results
