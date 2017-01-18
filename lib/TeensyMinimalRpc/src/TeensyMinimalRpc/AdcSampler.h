#ifndef ___TEENSY__ADC_SAMPLER__H___
#define ___TEENSY__ADC_SAMPLER__H___

#include <aligned_alloc.h>

#if 0
namespace teensy {
namespace adc {
  template <class Node>
  class AdcSampler {
    Node &node_obj_;
    int sample_count_; // The number of samples to record for each ADC channel.
    int adc_number_;
    int dma_scatter_;
    int dma_i_;
    int dma_ii_;
    UInt16Array scan_result_;
    UInt32Array channel_sc1as_;
    UInt16Array samples_;
    UInt8Array tcds_;

    AdcSampler(Node &node_obj, UInt8Array channel_sc1as, int sample_count,
               UInt8Array dma_channels, int adc_number)
      : node_obj_(node_obj), sample_count_(sample_count),
        adc_number_(adc_number) {
      if (dmf_channels.length == 3) {
        dma_scatter_ = dmf_channels.data[0];
        dma_i_ = dmf_channels.data[1];
        dma_ii_ = dmf_channels.data[2];
      } else {
        // By default, use DMA channels 0, 1, and 2.
        dma_scatter_ = 0;
        dma_i_ = 1;
        dma_ii_ = 2;
      }

      // Map Teensy analog channel labels to channels in `ADC_SC1x` format.
      //channel_sc1as = np.array(adc.SC1A_PINS[channels].tolist(),
                                    //dtype='uint32')

      // Allocate device memory for results and DMA/ADC configurations.
      allocate_device_arrays(channel_sc1as.length);
    }

    void init_registers() {
      // Enable PDB clock (DMA and ADC clocks should already be enabled).
      SIM_SCGC6 |= SIM_SCGC6_PDB; // PDB Clock Gate Control

      reset();

      //self.configure_adc()
      //self.configure_timer(1)

      //self.configure_dma_channel_ii_mux()
      //self.assert_no_dma_error()

      //self.configure_dma_channel_scatter()
      //self.assert_no_dma_error()

      //self.configure_dma_channel_i()
      //self.assert_no_dma_error()

      //self.configure_dma_channel_ii()
      //self.assert_no_dma_error()

      //self.configure_dma_channel_i_mux()
      //self.assert_no_dma_error()
    }

    //def assert_no_dma_error(self):
        //df_dma_registers = self.proxy.DMA_registers()
        //assert(df_dma_registers.loc[df_dma_registers
                                    //.full_name == 'ERR',
                                    //'value'].sum() == 0)
    void allocate_device_arrays(uint16_t channel_count) {
      // Allocate device memory for results from single ADC scan.
      scan_result_.length = channel_count;
      scan_result_.data = (uint16_t *)calloc(scan_result_.length,
                                             sizeof(uint16_t));

      // Allocate memory for channel SC1A configurations.
      // **N.B.,** Memory must be aligned, since contents of `channel_sc1as_`
      // are used in DMA transfers. **TODO** verify this is true...
      channel_sc1as_.length = channel_count;
      channel_sc1as_.data = (uint32_t *)aligned_malloc(sizeof(uint32_t),
                                                       channel_sc1as_.length *
                                                       sizeof(uint32_t);

      // Allocate device memory for sample buffer for each ADC channel.
      samples_.length = sample_count_ * channel_count;
      samples_.data = (uint16_t *)calloc(samples_.length, sizeof(uint16_t));

      // Allocate device memory for DMA TCD configurations.
      // __N.B.,__ Transfer control descriptors are 32 bytes each and MUST be
      // aligned to 0-modulo-32 address.
      tcds_.length = sample_count_ * 32;
      tcds_.data = (uint8_t *)aligned_malloc(32, tcds_.length);

      // Store list of device TCD configuration addresses.
      //self.tcd_addrs = [self.allocs.tcds + 32 * i
                        //for i in xrange(self.sample_count)]
      // Store list of device TCD register addresses.
      // __N.B.,__ There are 16 DMA channels on the device.
      // __TODO__ Query `proxy` to determine number of DMA channels.
      //self.hw_tcd_addrs = [dma.HW_TCDS_ADDR + 32 * i for i in xrange(16)]
    }

    void dealloc() {
      free(scan_result_.data);
      free(samples_.data);
      aligned_free(channel_sc1as_.data);
      aligned_free(tcds_.data);
    }

    void reset() {
      // Fill result arrays with zeros
      mem_fill((uint16_t *)scan_result_.data, 0, scan_result_.length);
      mem_fill((uint16_t *)samples_.data, 0, samples_.length);
    }

    //def configure_dma_channel_scatter(self):
        //# Create Transfer Control Descriptor configuration for first chunk, encoded
        //# as a Protocol Buffer message.
        //tcd0_msg = DMA.TCD(CITER_ELINKNO=DMA.R_TCD_ITER_ELINKNO(ITER=1),
                           //BITER_ELINKNO=DMA.R_TCD_ITER_ELINKNO(ITER=1),
                           //ATTR=DMA.R_TCD_ATTR(SSIZE=DMA.R_TCD_ATTR._16_BIT,
                                               //DSIZE=DMA.R_TCD_ATTR._16_BIT),
                           //NBYTES_MLNO=self.N,
                           //SADDR=int(self.allocs.scan_result),
                           //SOFF=2,
                           //SLAST=-self.N,
                           //DADDR=int(self.allocs.samples),
                           //DOFF=2 * self.sample_count,
                           //DLASTSGA=int(self.tcd_addrs[1]),
                           //CSR=DMA.R_TCD_CSR(START=0, DONE=False, ESG=True))

        //# Convert Protocol Buffer encoded TCD to bytes structure.
        //tcd0 = self.proxy.tcd_msg_to_struct(tcd0_msg)

        //# Create binary TCD struct for each TCD protobuf message and copy to device
        //# memory.
        //for i in xrange(self.sample_count):
            //tcd_i = tcd0.copy()
            //tcd_i['SADDR'] = self.allocs.scan_result
            //tcd_i['DADDR'] = self.allocs.samples + 2 * i
            //tcd_i['DLASTSGA'] = self.tcd_addrs[(i + 1)
                                               //% len(self.tcd_addrs)]
            //tcd_i['CSR'] |= (1 << 4)
            //if i == (self.sample_count - 1):  # Last sample, so trigger major loop interrupt
                //tcd_i['CSR'] |= (1 << 1)  # Set `INTMAJOR` (21.3.29/426)
            //self.proxy.mem_cpy_host_to_device(self.tcd_addrs[i],
                                              //tcd_i.tostring())

        //# Load initial TCD in scatter chain to DMA channel chosen to handle scattering.
        //self.proxy.mem_cpy_host_to_device(self.hw_tcd_addrs
                                          //[self.dma_channels.scatter],
                                          //tcd0.tostring())
        //self.proxy.attach_dma_interrupt(self.dma_channels.scatter)

    //def configure_dma_channel_i(self):
        //sca1_tcd_msg =            DMA.TCD(CITER_ELINKNO=
                    //DMA.R_TCD_ITER_ELINKNO(ELINK=False,
                                           //ITER=self.channel_sc1as.size),
                    //BITER_ELINKNO=
                    //DMA.R_TCD_ITER_ELINKNO(ELINK=False,
                                           //ITER=self.channel_sc1as.size),
                    //ATTR=DMA.R_TCD_ATTR(SSIZE=DMA.R_TCD_ATTR._32_BIT,
                                        //DSIZE=DMA.R_TCD_ATTR._32_BIT),
                    //NBYTES_MLNO=4,  # `SDA1` register is 4 bytes (32-bit)
                    //SADDR=int(self.allocs.sc1as),
                    //SOFF=4,
                    //SLAST=-self.channel_sc1as.size * 4,
                    //DADDR=int(adc.ADC0_SC1A),
                    //DOFF=0,
                    //DLASTSGA=0,
                    //CSR=DMA.R_TCD_CSR(START=0, DONE=False))

        //self.proxy.update_dma_TCD(self.dma_channels.i, sca1_tcd_msg)

    //def configure_dma_channel_i_mux(self):
        //self.proxy.update_dma_mux_chcfg(self.dma_channels.i,
                                        //DMA.MUX_CHCFG(SOURCE=
                                                      //dma.DMAMUX_SOURCE_PDB,
                                                      //TRIG=False,
                                                      //ENBL=True))
        //self.proxy.update_dma_registers(
            //DMA.Registers(SERQ=int(self.dma_channels.i)))

    void configure_adc() {
      // **TODO**
      // Select B input for ADC MUX.
      //self.proxy.update_adc_registers(
          //self.adc_number,
          //ADC.Registers(CFG2=ADC.R_CFG2(MUXSEL=ADC.R_CFG2.B)))
    }

    //void configure_timer(uint32_t sample_rate_hz) {
    void configure_timer(teensy__3_1_pdb_ClockDivideSettings
                         const &clock_divide) {
      // Set PDB interrupt to occur when IDLY is equal to CNT + 1.
      PDB0_IDLY = 1

      //clock_divide = pdb.get_pdb_divide_params(sample_rate_hz).iloc[0]

      PDB0_MOD = clock_divide.mod;

      uint32_t PDB_CONFIG = (PDB_SC_TRGSEL(15)  // Software trigger
                             | PDB_SC_PDBEN  // Enable PDB
                             | PDB_SC_CONT  // Continuous
                             | PDB_SC_LDMOD(0)
                             | PDB_SC_PRESCALER(clock_divide.prescaler)
                             | PDB_SC_MULT(clock_divide.mult)
                             | PDB_SC_DMAEN  // Enable DMA
                             | PDB_SC_LDOK);  // Load all new values
      PDB0_SC = PDB_CONFIG;
    }

    void configure_dma_channel_ii_mux() {
      // ### Set DMA mux source for channel to ADC0 ###

      self.proxy.update_dma_mux_chcfg(
          self.dma_channels.ii,
          DMA.MUX_CHCFG(
              # Route ADC0 as DMA channel source.
              SOURCE=dma.DMAMUX_SOURCE_ADC0,
              TRIG=False,# Disable periodic trigger.
              # Enable the DMAMUX configuration for channel.
              ENBL=True))
      self.proxy.enableDMA(teensy.ADC_0)
    }

    //def configure_dma_channel_ii(self):
        //tcd_msg = DMA.TCD(
            //CITER_ELINKYES=
            //DMA.R_TCD_ITER_ELINKYES(ELINK=True,
                                    //LINKCH=1,
                                    //ITER=self.channel_sc1as.size),
            //BITER_ELINKYES=
            //DMA.R_TCD_ITER_ELINKYES(ELINK=True,
                                    //LINKCH=1,
                                    //ITER=self.channel_sc1as.size),
            //ATTR=DMA.R_TCD_ATTR(SSIZE=DMA.R_TCD_ATTR._16_BIT,
                                //DSIZE=DMA.R_TCD_ATTR._16_BIT),
            //NBYTES_MLNO=2,  # sizeof(uint16)
            //SADDR=adc.ADC0_RA,
            //SOFF=0,
            //SLAST=0,
            //DADDR=int(self.allocs.scan_result),
            //DOFF=2,
            //DLASTSGA=-self.N,
            //CSR=DMA.R_TCD_CSR(START=0, DONE=False,
                              //# Start `scatter` DMA channel
                              //# after completion of major loop.
                              //MAJORELINK=True,
                              //MAJORLINKCH=
                              //int(self.dma_channels.scatter)))

        //self.proxy.update_dma_TCD(self.dma_channels.ii, tcd_msg)

        //# DMA request input signals and this enable request flag
        //# must be asserted before a channel’s hardware service
        //# request is accepted (21.3.3/394).
        //self.proxy.update_dma_registers(
            //DMA.Registers(SERQ=int(self.dma_channels.ii)))

    //def start_read(self, sample_rate_hz):
        //'''
        //Trigger start of ADC sampling at the specified sampling rate.
        //'''
        //self.proxy.attach_dma_interrupt(self.dma_channels.scatter)
        //pdb_config = self.configure_timer(sample_rate_hz)
        //pdb_config |= pdb.PDB_SC_SWTRIG  # Start the counter.

        //# Copy configured PDB register state to device hardware register.
        //self.proxy.mem_cpy_host_to_device(pdb.PDB0_SC, np.uint32(pdb_config)
                                          //.tostring())

        //# **N.B.,** Timer will be stopped by the scatter DMA channel major loop
        //# interrupt handler after `sample_count` samples have been collected.

    //def __dealloc__(self):
        //self.allocs[['scan_result', 'samples']].map(self.proxy.mem_free)
        //self.allocs[['sc1as', 'tcds']].map(self.proxy.mem_aligned_free)

    //def get_results(self):
        //data = self.proxy.mem_cpy_device_to_host(self.allocs.samples,
                                                 //self.sample_count * self.N)
        //df_adc_results = pd.DataFrame(data.view('uint16')
                                      //.reshape(-1, self.sample_count).T,
                                      //columns=self.channels)
        //return df_adc_results

  }
}  // namespace adc
}  // namespace teensy
#endif

#endif  // #ifndef ___TEENSY__ADC_SAMPLER__H___
