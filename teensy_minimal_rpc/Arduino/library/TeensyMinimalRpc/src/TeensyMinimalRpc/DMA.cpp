#include "DMA.h"

namespace teensy {
namespace dma {
  UInt8Array serialize_TCD(uint8_t channel_num, UInt8Array buffer) {
    // __NB__ Transfer control descriptor (TCD) range starts at address of
    // `DMA_TCD0_SADDR`.
    volatile DMABaseClass::TCD_t &tcd =
      *(reinterpret_cast<volatile DMABaseClass::TCD_t *>(&DMA_TCD0_SADDR) +
        channel_num);
    // Create empty DMA Registers Protocol Buffer message.
    teensy__3_1_dma_TCD result = teensy__3_1_dma_TCD_init_default;
    // (32 bits) TCD Source Address 21.3.17/415
    result.has_SADDR = true;
    result.SADDR = (uint32_t)tcd.SADDR;
    // (16 bits) TCD Signed Source Address Offset 21.3.18/415
    result.has_SOFF = true;
    result.SOFF = (uint32_t)tcd.SOFF;
    // (16 bits) TCD Transfer Attributes 21.3.19/416
    result.has_ATTR = true;
    result.ATTR.has_SMOD = true;
    result.ATTR.SMOD = (uint32_t)((tcd.ATTR_SRC >> 3) & 0x1F);
    result.ATTR.has_SSIZE = true;
    result.ATTR.SSIZE = (teensy__3_1_dma_R_TCD_ATTR_E_SIZE)(tcd.ATTR_SRC & 0x7);
    result.ATTR.has_DMOD = true;
    result.ATTR.DMOD = (uint32_t)((tcd.ATTR_DST >> 3) & 0x1F);
    result.ATTR.has_DSIZE = true;
    result.ATTR.DSIZE = (teensy__3_1_dma_R_TCD_ATTR_E_SIZE)(tcd.ATTR_DST & 0x7);
    if (!(DMA_CR & DMA_CR_EMLM)) {  // Enable Minor Loop Mapping
      // (32 bits) TCD Minor Byte Count (Minor Loop Disabled) 21.3.20/417
      result.has_NBYTES_MLNO = true;
      result.NBYTES_MLNO = tcd.NBYTES_MLNO;
    } else {
      const bool SMLOE = (tcd.NBYTES_MLOFFNO >> 31) & 0x1;
      const bool DMLOE = (tcd.NBYTES_MLOFFNO >> 30) & 0x1;

      if (!SMLOE && !DMLOE) {
        // (32 bits) TCD Signed Minor Loop Offset (Minor Loop Enabled and Offset Disabled) 21.3.21/417
        result.has_NBYTES_MLOFFNO = true;
        result.NBYTES_MLOFFNO.has_SMLOE = true;
        result.NBYTES_MLOFFNO.SMLOE = false;
        result.NBYTES_MLOFFNO.has_DMLOE = true;
        result.NBYTES_MLOFFNO.DMLOE = false;
        result.NBYTES_MLOFFNO.has_NBYTES = true;
        result.NBYTES_MLOFFNO.NBYTES = (uint32_t)(tcd.NBYTES_MLOFFNO & 0x3FFFFFFF);
      } else {
        // (32 bits) TCD Signed Minor Loop Offset (Minor Loop and Offset Enabled) 21.3.22/418
        result.has_NBYTES_MLOFFYES = true;
        result.NBYTES_MLOFFYES.has_SMLOE = true;
        result.NBYTES_MLOFFYES.SMLOE = SMLOE;
        result.NBYTES_MLOFFYES.has_DMLOE = true;
        result.NBYTES_MLOFFYES.DMLOE = DMLOE;
        result.NBYTES_MLOFFYES.has_MLOFF = true;
        result.NBYTES_MLOFFYES.MLOFF = (uint32_t)((tcd.NBYTES_MLOFFYES >> 10) & 0xFFFFF);
        result.NBYTES_MLOFFYES.has_NBYTES = true;
        result.NBYTES_MLOFFYES.NBYTES = (uint32_t)(tcd.NBYTES_MLOFFYES & 0x1FF);
      }
    }
    // (32 bits) TCD Last Source Address Adjustment 21.3.23/420
    result.has_SLAST = true;
    result.SLAST = tcd.SLAST;
    // (32 bits) TCD Destination Address 21.3.24/420
    result.has_DADDR = true;
    result.DADDR = (uint32_t)tcd.DADDR;
    // (16 bits) TCD Signed Destination Address Offset 21.3.25/421
    result.has_DOFF = true;
    result.DOFF = tcd.DOFF;
    const bool citer_elink = (tcd.CITER_ELINKYES >> 15) & 0x01;
    if (citer_elink) {
      // (16 bits) TCD Current Minor Loop Link, Major Loop Count (Channel Linking Enabled) 21.3.26/421
      result.has_CITER_ELINKYES = true;
      result.CITER_ELINKYES.has_ELINK = true;
      result.CITER_ELINKYES.ELINK = true;
      result.CITER_ELINKYES.has_LINKCH = true;
      result.CITER_ELINKYES.LINKCH = (uint32_t)((tcd.CITER_ELINKYES >> 9) & 0xF);
      result.CITER_ELINKYES.has_ITER = true;
      result.CITER_ELINKYES.ITER = (uint32_t)(tcd.CITER_ELINKYES & 0x1FF);
    } else {
      // (16 bits) 21.3.27/422
      result.has_CITER_ELINKNO = true;
      result.CITER_ELINKNO.has_ELINK = true;
      result.CITER_ELINKNO.ELINK = false;
      result.CITER_ELINKNO.has_ITER = true;
      result.CITER_ELINKNO.ITER = (uint32_t)(tcd.CITER_ELINKNO & 0x7FFF);
    }
    // (32 bits) TCD Last Destination Address Adjustment/Scatter Gather Address 21.3.28/423
    result.has_DLASTSGA = true;
    result.DLASTSGA = tcd.DLASTSGA;
    // (16 bits) TCD Control and Status 21.3.29/424
    result.has_CSR = true;
    result.CSR.has_BWC = true;
    result.CSR.BWC = (teensy__3_1_dma_R_TCD_CSR_E_BWC)((tcd.CSR >> 14) & 0x3);
    result.CSR.has_MAJORLINKCH = true;
    result.CSR.MAJORLINKCH = (uint32_t)((tcd.CSR >> 8) & 0xF);
    result.CSR.has_DONE = true;
    result.CSR.DONE = (tcd.CSR >> 7) & 0x1;
    result.CSR.has_ACTIVE = true;
    result.CSR.ACTIVE = (tcd.CSR >> 6) & 0x1;
    result.CSR.has_MAJORELINK = true;
    result.CSR.MAJORELINK = (tcd.CSR >> 5) & 0x1;
    result.CSR.has_ESG = true;
    result.CSR.ESG = (tcd.CSR >> 4) & 0x1;
    result.CSR.has_DREQ = true;
    result.CSR.DREQ = (tcd.CSR >> 3) & 0x1;
    result.CSR.has_INTHALF = true;
    result.CSR.INTHALF = (tcd.CSR >> 2) & 0x1;
    result.CSR.has_INTMAJOR = true;
    result.CSR.INTMAJOR = (tcd.CSR >> 1) & 0x1;
    result.CSR.has_START = true;
    result.CSR.START = tcd.CSR & 0x1;
    const bool biter_elink = (tcd.BITER_ELINKYES >> 15) & 0x01;
    if (biter_elink) {
      // (16 bits) TCD Beginning Minor Loop Link, Major Loop Count (Channel Linking Enabled) 21.3.30/426
      result.has_BITER_ELINKYES = true;
      result.BITER_ELINKYES.has_ELINK = true;
      result.BITER_ELINKYES.ELINK = true;
      result.BITER_ELINKYES.has_LINKCH = true;
      result.BITER_ELINKYES.LINKCH = (uint32_t)((tcd.BITER_ELINKYES >> 9) & 0xF);
      result.BITER_ELINKYES.has_ITER = true;
      result.BITER_ELINKYES.ITER = (uint32_t)(tcd.BITER_ELINKYES & 0x1FF);
    } else {
      // (16 bits) TCD Beginning Minor Loop Link, Major Loop Count (Channel Linking Disabled) 21.3.31/427
      result.has_BITER_ELINKNO = true;
      result.BITER_ELINKNO.has_ELINK = true;
      result.BITER_ELINKNO.ELINK = false;
      result.BITER_ELINKNO.has_ITER = true;
      result.BITER_ELINKNO.ITER = (uint32_t)(tcd.BITER_ELINKNO & 0x7FFF);
    }

    UInt8Array output =
      nanopb::serialize_to_array(result, teensy__3_1_dma_TCD_fields,
                                 buffer);
    return output;
  }

}  // namespace dma
}  // namespace teensy
