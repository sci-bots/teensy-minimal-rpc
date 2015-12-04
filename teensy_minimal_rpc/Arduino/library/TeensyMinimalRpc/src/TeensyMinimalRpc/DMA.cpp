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
    PB_SET_TEENSY_REG_BITS_FROM_VAL(result.ATTR, 5, 3, SMOD, tcd.ATTR_SRC, uint32_t)
    PB_SET_TEENSY_REG_BITS_FROM_VAL(result.ATTR, 3, 0, SSIZE, tcd.ATTR_SRC, teensy__3_1_dma_R_TCD_ATTR_E_SIZE)
    PB_SET_TEENSY_REG_BITS_FROM_VAL(result.ATTR, 5, 3, DMOD, tcd.ATTR_DST, uint32_t)
    PB_SET_TEENSY_REG_BITS_FROM_VAL(result.ATTR, 3, 0, DSIZE, tcd.ATTR_DST, teensy__3_1_dma_R_TCD_ATTR_E_SIZE)
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
        PB_SET_TEENSY_REG_BITS_FROM_VAL(result.NBYTES_MLOFFNO, 30, 0, NBYTES, tcd.NBYTES_MLOFFNO, uint32_t)
      } else {
        // (32 bits) TCD Signed Minor Loop Offset (Minor Loop and Offset Enabled) 21.3.22/418
        result.has_NBYTES_MLOFFYES = true;
        result.NBYTES_MLOFFYES.has_SMLOE = true;
        result.NBYTES_MLOFFYES.SMLOE = SMLOE;
        result.NBYTES_MLOFFYES.has_DMLOE = true;
        result.NBYTES_MLOFFYES.DMLOE = DMLOE;
        PB_SET_TEENSY_REG_BITS_FROM_VAL(result.NBYTES_MLOFFYES, 20, 10, MLOFF, tcd.NBYTES_MLOFFYES, uint32_t)
        PB_SET_TEENSY_REG_BITS_FROM_VAL(result.NBYTES_MLOFFYES, 9, 0, NBYTES, tcd.NBYTES_MLOFFYES, uint32_t)
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
      PB_SET_TEENSY_REG_BITS_FROM_VAL(result.CITER_ELINKYES, 4, 9, LINKCH, tcd.CITER_ELINKYES, uint32_t)
      PB_SET_TEENSY_REG_BITS_FROM_VAL(result.CITER_ELINKYES, 9, 0, ITER, tcd.CITER_ELINKYES, uint32_t)
    } else {
      // (16 bits) 21.3.27/422
      result.has_CITER_ELINKNO = true;
      result.CITER_ELINKNO.has_ELINK = true;
      result.CITER_ELINKNO.ELINK = false;
      PB_SET_TEENSY_REG_BITS_FROM_VAL(result.CITER_ELINKNO, 15, 0, ITER, tcd.CITER_ELINKNO, uint32_t)
    }
    // (32 bits) TCD Last Destination Address Adjustment/Scatter Gather Address 21.3.28/423
    result.has_DLASTSGA = true;
    result.DLASTSGA = tcd.DLASTSGA;
    // (16 bits) TCD Control and Status 21.3.29/424
    result.has_CSR = true;
    PB_SET_TEENSY_REG_BITS_FROM_VAL(result.CSR, 2, 14, BWC, tcd.CSR, teensy__3_1_dma_R_TCD_CSR_E_BWC)
    PB_SET_TEENSY_REG_BITS_FROM_VAL(result.CSR, 4, 8, MAJORLINKCH, tcd.CSR, uint32_t)
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.CSR, DMA_TCD_CSR, DONE, tcd.CSR)
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.CSR, DMA_TCD_CSR, ACTIVE, tcd.CSR)
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.CSR, DMA_TCD_CSR, MAJORELINK, tcd.CSR)
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.CSR, DMA_TCD_CSR, ESG, tcd.CSR)
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.CSR, DMA_TCD_CSR, DREQ, tcd.CSR)
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.CSR, DMA_TCD_CSR, INTHALF, tcd.CSR)
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.CSR, DMA_TCD_CSR, INTMAJOR, tcd.CSR)
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.CSR, DMA_TCD_CSR, START, tcd.CSR)

    const bool biter_elink = (tcd.BITER_ELINKYES >> 15) & 0x01;
    if (biter_elink) {
      // (16 bits) TCD Beginning Minor Loop Link, Major Loop Count (Channel Linking Enabled) 21.3.30/426
      result.has_BITER_ELINKYES = true;
      result.BITER_ELINKYES.has_ELINK = true;
      result.BITER_ELINKYES.ELINK = true;
      PB_SET_TEENSY_REG_BITS_FROM_VAL(result.BITER_ELINKYES, 4, 9, LINKCH, tcd.BITER_ELINKYES, uint32_t)
      PB_SET_TEENSY_REG_BITS_FROM_VAL(result.BITER_ELINKYES, 9, 0, ITER, tcd.BITER_ELINKYES, uint32_t)
    } else {
      // (16 bits) TCD Beginning Minor Loop Link, Major Loop Count (Channel Linking Disabled) 21.3.31/427
      result.has_BITER_ELINKNO = true;
      result.BITER_ELINKNO.has_ELINK = true;
      result.BITER_ELINKNO.ELINK = false;
      PB_SET_TEENSY_REG_BITS_FROM_VAL(result.BITER_ELINKNO, 15, 0, ITER, tcd.BITER_ELINKNO, uint32_t)
    }

    UInt8Array output =
      nanopb::serialize_to_array(result, teensy__3_1_dma_TCD_fields,
                                 buffer);
    return output;
  }

  int8_t update_TCD(uint8_t channel_num, teensy__3_1_dma_TCD const &tcd_new) {
    // __NB__ Transfer control descriptor (TCD) range starts at address of
    // `DMA_TCD0_SADDR`.
    volatile DMABaseClass::TCD_t &tcd =
      *(reinterpret_cast<volatile DMABaseClass::TCD_t *>(&DMA_TCD0_SADDR) +
        channel_num);

    // (32 bits) TCD Source Address 21.3.17/415
    if (tcd_new.has_SADDR) { tcd.SADDR = (const volatile void *)tcd_new.SADDR; }
    // (16 bits) TCD Signed Source Address Offset 21.3.18/415
    if (tcd_new.has_SOFF) { tcd.SOFF = (int16_t)tcd_new.SOFF; }

    // (16 bits) TCD Transfer Attributes 21.3.19/416
    if (tcd_new.has_ATTR) {
      uint8_t ATTR = tcd.ATTR_SRC;

      PB_UPDATE_TEENSY_REG_BITS(tcd_new.ATTR, 5, 3, SMOD, ATTR)
      PB_UPDATE_TEENSY_REG_BITS(tcd_new.ATTR, 3, 0, SSIZE, ATTR)
      if (tcd_new.ATTR.has_SMOD || tcd_new.ATTR.has_SSIZE) {
          tcd.ATTR_SRC = ATTR;
      }

      ATTR = 0;
      PB_UPDATE_TEENSY_REG_BITS(tcd_new.ATTR, 5, 3, DMOD, ATTR)
      PB_UPDATE_TEENSY_REG_BITS(tcd_new.ATTR, 3, 0, DSIZE, ATTR)
      if (tcd_new.ATTR.has_DMOD || tcd_new.ATTR.has_DSIZE) {
          tcd.ATTR_DST = ATTR;
      }
    }

    // (32 bits) TCD Minor Byte Count (Minor Loop Disabled) 21.3.20/417
    if (tcd_new.has_NBYTES_MLNO) { tcd.NBYTES_MLNO = tcd_new.NBYTES_MLNO; }
    else if (tcd_new.has_NBYTES_MLOFFNO &&
             tcd_new.NBYTES_MLOFFNO.has_NBYTES) {
      // (32 bits) TCD Signed Minor Loop Offset (Minor Loop Enabled and Offset Disabled) 21.3.21/417
      tcd.NBYTES_MLOFFNO = tcd_new.NBYTES_MLOFFNO.NBYTES;
    } else if (tcd_new.has_NBYTES_MLOFFYES) {
      if (tcd_new.NBYTES_MLOFFYES.has_SMLOE) {
        // (32 bits) TCD Signed Minor Loop Offset (Minor Loop and Offset Enabled) 21.3.22/418
        uint32_t NBYTES = tcd.NBYTES_MLOFFYES;

        PB_UPDATE_TEENSY_REG_BIT(tcd_new.NBYTES_MLOFFYES, DMA_TCD_NBYTES, SMLOE, NBYTES)
        PB_UPDATE_TEENSY_REG_BIT(tcd_new.NBYTES_MLOFFYES, DMA_TCD_NBYTES, DMLOE, NBYTES)
        PB_UPDATE_TEENSY_REG_BITS(tcd_new.NBYTES_MLOFFYES, 20, 10, MLOFF, NBYTES)
        PB_UPDATE_TEENSY_REG_BITS(tcd_new.NBYTES_MLOFFYES, 10, 0, NBYTES, NBYTES)

        tcd.NBYTES_MLOFFYES = NBYTES;
      }
    }
    // (32 bits) TCD Last Source Address Adjustment 21.3.23/420
    if (tcd_new.has_SLAST) {
      tcd.SLAST = tcd_new.SLAST;
    }
    // (32 bits) TCD Destination Address 21.3.24/420
    if (tcd_new.has_DADDR) {
      tcd.DADDR = (volatile void *)tcd_new.DADDR;
    }
    // (16 bits) TCD Signed Destination Address Offset 21.3.25/421
    if (tcd_new.has_DOFF) {
      tcd.DOFF = tcd_new.DOFF;
    }

    if (tcd_new.has_CITER_ELINKYES) {
      // (16 bits) TCD Current Minor Loop Link, Major Loop Count (Channel Linking Enabled) 21.3.26/421
      uint16_t CITER = tcd.CITER_ELINKYES;

      PB_UPDATE_TEENSY_REG_BIT(tcd_new.CITER_ELINKYES, DMA_TCD_CITER, ELINK, CITER)
      PB_UPDATE_TEENSY_REG_BITS(tcd_new.CITER_ELINKYES, 4, 9, LINKCH, CITER)
      PB_UPDATE_TEENSY_REG_BITS(tcd_new.CITER_ELINKYES, 9, 0, ITER, CITER)

      tcd.CITER_ELINKYES = CITER;  // Update hardware TCD register.
    } else if (tcd_new.has_CITER_ELINKNO) {
      // (16 bits) 21.3.27/422
      uint16_t CITER = tcd.CITER_ELINKNO;

      PB_UPDATE_TEENSY_REG_BIT(tcd_new.CITER_ELINKNO, DMA_TCD_CITER, ELINK, CITER)
      PB_UPDATE_TEENSY_REG_BITS(tcd_new.CITER_ELINKNO, 15, 0, ITER, CITER)

      tcd.CITER_ELINKNO = CITER;  // Update hardware TCD register.
    }

    // (32 bits) TCD Last Destination Address Adjustment/Scatter Gather Address 21.3.28/423
    if (tcd_new.has_DLASTSGA) { tcd.DLASTSGA = tcd_new.DLASTSGA; }

    if (tcd_new.has_BITER_ELINKYES) {
      // (16 bits) TCD Beginning Minor Loop Link, Major Loop Count (Channel Linking Enabled) 21.3.30/426
      uint16_t BITER = tcd.BITER_ELINKYES;

      PB_UPDATE_TEENSY_REG_BIT(tcd_new.BITER_ELINKYES, DMA_TCD_BITER, ELINK, BITER)
      PB_UPDATE_TEENSY_REG_BITS(tcd_new.BITER_ELINKYES, 4, 9, LINKCH, BITER)
      PB_UPDATE_TEENSY_REG_BITS(tcd_new.BITER_ELINKYES, 9, 0, ITER, BITER)

      tcd.BITER_ELINKYES = BITER;  // Update hardware TCD register.
    } else if (tcd_new.has_BITER_ELINKNO) {
      // (16 bits) TCD Beginning Minor Loop Link, Major Loop Count (Channel Linking Disabled) 21.3.31/427
      uint16_t BITER = tcd.BITER_ELINKNO;

      PB_UPDATE_TEENSY_REG_BIT(tcd_new.BITER_ELINKNO, DMA_TCD_BITER, ELINK, BITER)
      PB_UPDATE_TEENSY_REG_BITS(tcd_new.BITER_ELINKNO, 15, 0, ITER, BITER)

      tcd.BITER_ELINKNO = BITER;  // Update hardware TCD register.
    }

    // (16 bits) TCD Control and Status 21.3.29/424
    if (tcd_new.has_CSR) {
      uint16_t CSR = tcd.CSR;

      PB_UPDATE_TEENSY_REG_BITS(tcd_new.CSR, 2, 14, BWC, CSR)
      PB_UPDATE_TEENSY_REG_BITS(tcd_new.CSR, 4, 8, MAJORLINKCH, CSR)
      PB_UPDATE_TEENSY_REG_BIT(tcd_new.CSR, DMA_TCD_CSR, DONE, CSR)
      PB_UPDATE_TEENSY_REG_BIT(tcd_new.CSR, DMA_TCD_CSR, ACTIVE, CSR)
      PB_UPDATE_TEENSY_REG_BIT(tcd_new.CSR, DMA_TCD_CSR, MAJORELINK, CSR)
      PB_UPDATE_TEENSY_REG_BIT(tcd_new.CSR, DMA_TCD_CSR, ESG, CSR)
      PB_UPDATE_TEENSY_REG_BIT(tcd_new.CSR, DMA_TCD_CSR, DREQ, CSR)
      PB_UPDATE_TEENSY_REG_BIT(tcd_new.CSR, DMA_TCD_CSR, INTHALF, CSR)
      PB_UPDATE_TEENSY_REG_BIT(tcd_new.CSR, DMA_TCD_CSR, INTMAJOR, CSR)
      tcd.CSR = CSR;

      // **N.B.** Start bit *must* be set after *all other* fields are set,
      // since it will immediately trigger the transfer request.
      PB_UPDATE_TEENSY_REG_BIT(tcd_new.CSR, DMA_TCD_CSR, START, tcd.CSR)
      tcd.CSR = CSR;
    }
    return 0;
  }

  int8_t update_TCD(uint8_t channel_num, UInt8Array tcd_data) {
    // Create empty DMA Registers Protocol Buffer message.
    teensy__3_1_dma_TCD tcd_new = teensy__3_1_dma_TCD_init_default;

    bool ok = nanopb::decode_from_array(tcd_data, teensy__3_1_dma_TCD_fields,
                                        tcd_new);
    if (!ok) { return -1; }
    return update_TCD(channel_num, tcd_new);
  }

  teensy__3_1_dma_Registers registers_to_protobuf() {
    teensy__3_1_dma_Registers result = teensy__3_1_dma_Registers_init_default;

    result.has_CR = true;
    PB_SET_TEENSY_REG_BIT(result.CR, DMA_CR, CX) // Cancel Transfer
    PB_SET_TEENSY_REG_BIT(result.CR, DMA_CR, ECX) // Error Cancel Transfer
    PB_SET_TEENSY_REG_BIT(result.CR, DMA_CR, EMLM) // Enable Minor Loop Mapping
    PB_SET_TEENSY_REG_BIT(result.CR, DMA_CR, CLM) // Continuous Link Mode
    PB_SET_TEENSY_REG_BIT(result.CR, DMA_CR, HALT) // Halt DMA Operations
    PB_SET_TEENSY_REG_BIT(result.CR, DMA_CR, HOE) // Halt On Error
    PB_SET_TEENSY_REG_BIT(result.CR, DMA_CR, ERCA) // Enable Round Robin Channel Arbitration
    PB_SET_TEENSY_REG_BIT(result.CR, DMA_CR, EDBG) // Enable Debug

    result.has_ES = true;
    PB_SET_TEENSY_BIT_FROM_VAL(result.ES, 31, VLD, DMA_ES) // Logical OR of all ERR status bits
    PB_SET_TEENSY_BIT_FROM_VAL(result.ES, 16, ECX, DMA_ES) // Transfer Cancelled
    PB_SET_TEENSY_BIT_FROM_VAL(result.ES, 14, CPE, DMA_ES) // Channel Priority Error
    PB_SET_TEENSY_REG_BITS_FROM_VAL(result.ES, 4, 8, ERRCHN, DMA_ES, uint32_t) // Error Channel Number or Cancelled Channel Number
    PB_SET_TEENSY_BIT_FROM_VAL(result.ES, 7, SAE, DMA_ES) // Source Address Error
    PB_SET_TEENSY_BIT_FROM_VAL(result.ES, 6, SOE, DMA_ES) // Source Offset Error
    PB_SET_TEENSY_BIT_FROM_VAL(result.ES, 5, DAE, DMA_ES) // Destination Address Error
    PB_SET_TEENSY_BIT_FROM_VAL(result.ES, 4, DOE, DMA_ES) // Destination Offset Error
    PB_SET_TEENSY_BIT_FROM_VAL(result.ES, 3, NCE, DMA_ES) // NBYTES/CITER Configuration Error
    PB_SET_TEENSY_BIT_FROM_VAL(result.ES, 2, SGE, DMA_ES) // Scatter/Gather Configuration Error
    PB_SET_TEENSY_BIT_FROM_VAL(result.ES, 1, SBE, DMA_ES) // Source Bus Error
    PB_SET_TEENSY_BIT_FROM_VAL(result.ES, 0, DBE, DMA_ES) // Destination Bus Error

    result.has_ERQ = true; // (32 bits) Enable Request Register 21.3.3/394
    result.ERQ = (uint32_t)DMA_ERQ;
    result.has_EEI = true; // (32 bits) Enable Error Interrupt Register 21.3.4/397
    result.EEI = (uint32_t)DMA_EEI;
    result.has_INT = true; // (32 bits) Interrupt Request Register 21.3.13/406
    result.INT = (uint32_t)DMA_INT;
    result.has_ERR = true; // (32 bits) Error Register 21.3.14/409
    result.ERR = (uint32_t)DMA_ERR;
    result.has_HRS = true; // (32 bits) Hardware Request Status Register 21.3.15/411
    result.HRS = (uint32_t)DMA_HRS;

    return result;
  }

  UInt8Array serialize_registers(UInt8Array buffer) {
    teensy__3_1_dma_Registers result = registers_to_protobuf();
    UInt8Array output =
      nanopb::serialize_to_array(result, teensy__3_1_dma_Registers_fields,
                                 buffer);
    return output;
  }

  int8_t update_registers(UInt8Array serialized_registers) {
    // Create empty DMA Registers Protocol Buffer message.
    teensy__3_1_dma_Registers dma_msg = teensy__3_1_dma_Registers_init_default;

    bool ok = nanopb::decode_from_array(serialized_registers,
                                        teensy__3_1_dma_Registers_fields,
                                        dma_msg);
    if (!ok) { return -1; }

    if (dma_msg.has_CR) {
      uint32_t CR = DMA_CR;

      PB_UPDATE_TEENSY_REG_BIT(dma_msg.CR, DMA_CR, CX, CR) // Cancel Transfer
      PB_UPDATE_TEENSY_REG_BIT(dma_msg.CR, DMA_CR, ECX, CR) // Error Cancel Transfer
      PB_UPDATE_TEENSY_REG_BIT(dma_msg.CR, DMA_CR, EMLM, CR) // Enable Minor Loop Mapping
      PB_UPDATE_TEENSY_REG_BIT(dma_msg.CR, DMA_CR, CLM, CR) // Continuous Link Mode
      PB_UPDATE_TEENSY_REG_BIT(dma_msg.CR, DMA_CR, HALT, CR) // Halt DMA Operations
      PB_UPDATE_TEENSY_REG_BIT(dma_msg.CR, DMA_CR, HOE, CR) // Halt On Error
      PB_UPDATE_TEENSY_REG_BIT(dma_msg.CR, DMA_CR, ERCA, CR) // Enable Round Robin Channel Arbitration
      PB_UPDATE_TEENSY_REG_BIT(dma_msg.CR, DMA_CR, EDBG, CR) // Enable Debug

      DMA_CR = CR;
    }

    if (dma_msg.has_ES) {
      uint32_t ES = DMA_ES;

      PB_UPDATE_TEENSY_BIT(dma_msg.ES, 31, VLD, ES) // Logical OR of all ERR status bits
      PB_UPDATE_TEENSY_BIT(dma_msg.ES, 16, ECX, ES) // Transfer Cancelled
      PB_UPDATE_TEENSY_BIT(dma_msg.ES, 14, CPE, ES) // Channel Priority Error
      PB_UPDATE_TEENSY_REG_BITS(dma_msg.ES, 4, 8, ERRCHN, ES) // Error Channel Number or Cancelled Channel Number
      PB_UPDATE_TEENSY_BIT(dma_msg.ES, 7, SAE, ES) // Source Address Error
      PB_UPDATE_TEENSY_BIT(dma_msg.ES, 6, SOE, ES) // Source Offset Error
      PB_UPDATE_TEENSY_BIT(dma_msg.ES, 5, DAE, ES) // Destination Address Error
      PB_UPDATE_TEENSY_BIT(dma_msg.ES, 4, DOE, ES) // Destination Offset Error
      PB_UPDATE_TEENSY_BIT(dma_msg.ES, 3, NCE, ES) // NBYTES/CITER Configuration Error
      PB_UPDATE_TEENSY_BIT(dma_msg.ES, 2, SGE, ES) // Scatter/Gather Configuration Error
      PB_UPDATE_TEENSY_BIT(dma_msg.ES, 1, SBE, ES) // Source Bus Error
      PB_UPDATE_TEENSY_BIT(dma_msg.ES, 0, DBE, ES) // Destination Bus Error

      DMA_CR = ES;
    }

    // (8 bits) Clear Enable Error Interrupt Register 21.3.5/399
    if (dma_msg.has_CEEI) { DMA_CEEI = (uint8_t)dma_msg.CEEI; }
    // (8 bits) Set Enable Error Interrupt Register 21.3.6/400
    if (dma_msg.has_SEEI) { DMA_SEEI = (uint8_t)dma_msg.SEEI; }
    // (8 bits) Clear Enable Request Register 21.3.7/401
    if (dma_msg.has_CERQ) { DMA_CERQ = (uint8_t)dma_msg.CERQ; }
    // (8 bits) Set Enable Request Register 21.3.8/402
    if (dma_msg.has_SERQ) { DMA_SERQ = (uint8_t)dma_msg.SERQ; }
    // (8 bits) Clear DONE Status Bit Register 21.3.9/403
    if (dma_msg.has_CDNE) { DMA_CDNE = (uint8_t)dma_msg.CDNE; }
    // (8 bits) Set START Bit Register 21.3.10/404
    if (dma_msg.has_SSRT) { DMA_SSRT = (uint8_t)dma_msg.SSRT; }
    // (8 bits) Clear Error Register 21.3.11/405
    if (dma_msg.has_CERR) { DMA_CERR = (uint8_t)dma_msg.CERR; }
    // (8 bits) Clear Interrupt Request Register 21.3.12/406
    if (dma_msg.has_CINT) { DMA_CINT = (uint8_t)dma_msg.CINT; }

    // (32 bits) Enable Request Register 21.3.3/394
    if (dma_msg.has_ERQ) { DMA_ERQ = dma_msg.ERQ; }
    // (32 bits) Enable Error Interrupt Register 21.3.4/397
    if (dma_msg.has_EEI) { DMA_EEI = dma_msg.EEI; }
    // (32 bits) Interrupt Request Register 21.3.13/406
    if (dma_msg.has_INT) { DMA_INT = dma_msg.INT; }
    // (32 bits) Error Register 21.3.14/409
    if (dma_msg.has_ERR) { DMA_ERR = dma_msg.ERR; }
    // (32 bits) Hardware Request Status Register 21.3.15/411
    if (dma_msg.has_HRS) { DMA_HRS = dma_msg.HRS; }

    return 0;
  }

  inline volatile uint8_t *channel_to_dchpri_addr(uint8_t channel_num) {
    return &DMA_DCHPRI3 + ((~channel_num) & 0x3) + (channel_num & 0xFC);
  }

  UInt8Array serialize_dchpri(uint32_t channel_num, UInt8Array buffer) {
    // (8 bits) Channel n Priority Register 21.3.16/414
    teensy__3_1_dma_DCHPRI result = teensy__3_1_dma_DCHPRI_init_default;

    volatile uint8_t &DCHPRI = *channel_to_dchpri_addr(channel_num);

    PB_SET_TEENSY_REG_BIT_FROM_VAL(result, DMA_DCHPRI, ECP, DCHPRI)  // Enable PreEmption
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result, DMA_DCHPRI, DPA, DCHPRI)  // Disable PreEmpt Ability
    PB_SET_TEENSY_REG_BITS_FROM_VAL(result, 4, 0, CHPRI, DCHPRI, uint32_t)  // Channel Arbitration Priority

    UInt8Array output =
      nanopb::serialize_to_array(result, teensy__3_1_dma_DCHPRI_fields,
                                 buffer);
    return output;
  }

  int8_t update_dchpri(uint32_t channel_num, UInt8Array serialized_dchpri) {
    // (8 bits) Channel n Priority Register 21.3.16/414
    teensy__3_1_dma_DCHPRI dchpri_msg = teensy__3_1_dma_DCHPRI_init_default;

    bool ok = nanopb::decode_from_array(serialized_dchpri,
                                        teensy__3_1_dma_DCHPRI_fields,
                                        dchpri_msg);
    if (!ok) { return -1; }

    volatile uint8_t &DCHPRI = *channel_to_dchpri_addr(channel_num);
    uint8_t dchpri = DCHPRI;

    PB_UPDATE_TEENSY_REG_BIT(dchpri_msg, DMA_DCHPRI, ECP, dchpri)  // Enable PreEmption
    PB_UPDATE_TEENSY_REG_BIT(dchpri_msg, DMA_DCHPRI, DPA, dchpri)  // Disable PreEmpt Ability
    PB_UPDATE_TEENSY_REG_BITS(dchpri_msg, 4, 0, CHPRI, dchpri)  // Channel Arbitration Priority

    DCHPRI = dchpri;

    return 0;
  }

  UInt8Array serialize_mux_chcfg(uint32_t channel_num, UInt8Array buffer) {
    // (8 bits) Channel Configuration register (DMAMUX_CHCFGn) (20.3.1/366)
    teensy__3_1_dma_MUX_CHCFG result = teensy__3_1_dma_MUX_CHCFG_init_default;

    volatile uint8_t &MUX_CHCFG = *((volatile uint8_t *)&(DMAMUX0_CHCFG0) +
                                    channel_num);

    PB_SET_TEENSY_BIT_FROM_VAL(result, 7, ENBL, MUX_CHCFG)  // DMA Channel Enable (20.3.1/366)
    PB_SET_TEENSY_BIT_FROM_VAL(result, 6, TRIG, MUX_CHCFG)  // DMA Channel Trigger Enable (20.3.1/366)
    PB_SET_TEENSY_REG_BITS_FROM_VAL(result, 6, 0, SOURCE, MUX_CHCFG, uint32_t)  // DMA Channel Source (Slot) (20.3.1/366)

    UInt8Array output =
      nanopb::serialize_to_array(result, teensy__3_1_dma_MUX_CHCFG_fields,
                                 buffer);
    return output;
  }

  int8_t update_mux_chcfg(uint32_t channel_num,
                          UInt8Array serialized_mux_chcfg) {
    // (8 bits) Channel Configuration register (DMAMUX_CHCFGn) (20.3.1/366)
    teensy__3_1_dma_MUX_CHCFG mux_chcfg_msg =
      teensy__3_1_dma_MUX_CHCFG_init_default;

    bool ok = nanopb::decode_from_array(serialized_mux_chcfg,
                                        teensy__3_1_dma_MUX_CHCFG_fields,
                                        mux_chcfg_msg);
    if (!ok) { return -1; }

    volatile uint8_t &MUX_CHCFG = *((volatile uint8_t *)&(DMAMUX0_CHCFG0) +
                                    channel_num);
    uint8_t mux_chcfg = MUX_CHCFG;

    MUX_CHCFG = 0;  // Disable channel during update, as required (20.3.1/366).

    PB_UPDATE_TEENSY_BIT(mux_chcfg_msg, 7, ENBL, mux_chcfg)  // DMA Channel Enable (20.3.1/366)
    PB_UPDATE_TEENSY_BIT(mux_chcfg_msg, 6, TRIG, mux_chcfg)  // DMA Channel Trigger Enable (20.3.1/366)
    PB_UPDATE_TEENSY_REG_BITS(mux_chcfg_msg, 6, 0, SOURCE, mux_chcfg)  // DMA Channel Source (Slot) (20.3.1/366)

    MUX_CHCFG = mux_chcfg;

    return 0;
  }
}  // namespace dma
}  // namespace teensy

