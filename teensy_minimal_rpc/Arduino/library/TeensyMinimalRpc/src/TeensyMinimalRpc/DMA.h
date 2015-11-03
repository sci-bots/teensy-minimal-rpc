#ifndef ___TEENSY__DMA__H___
#define ___TEENSY__DMA__H___

#include <DMAChannel.h>
#include <CArrayDefs.h>  // UInt8Array
#include <pb_cpp_api.h>  // nanopb::serialize_to_array
#include <TeensyMinimalRpc/DMA_pb.h>
#include <TeensyMinimalRpc/RegisterMacros.h>


namespace teensy {
namespace dma {
  inline void reset_TCD(uint8_t channel_num) {
    const size_t tcd_size = sizeof(DMABaseClass::TCD_t);
    // __NB__ Transfer control descriptor (TCD) range starts at address of
    // `DMA_TCD0_SADDR`.
    volatile DMABaseClass::TCD_t &tcd =
      *(reinterpret_cast<volatile DMABaseClass::TCD_t *>(&DMA_TCD0_SADDR) +
        channel_num);
    memset((void *)&tcd, 0, tcd_size);
  }

  UInt8Array serialize_TCD(uint8_t channel_num, UInt8Array buffer);
  inline int8_t update_TCD(uint8_t channel_num, UInt8Array tcd_data) {
    // Create empty DMA Registers Protocol Buffer message.
    teensy__3_1_dma_TCD tcd_new = teensy__3_1_dma_TCD_init_default;

    bool ok = nanopb::decode_from_array(tcd_data, teensy__3_1_dma_TCD_fields,
                                        tcd_new);
    if (!ok) { return -1; }

    // __NB__ Transfer control descriptor (TCD) range starts at address of
    // `DMA_TCD0_SADDR`.
    volatile DMABaseClass::TCD_t &tcd =
      *(reinterpret_cast<volatile DMABaseClass::TCD_t *>(&DMA_TCD0_SADDR) +
        channel_num);

    // (32 bits) TCD Source Address 21.3.17/415
    if (tcd_new.has_SADDR) { tcd.SADDR = (const volatile void *)tcd_new.SADDR; }
    // (16 bits) TCD Signed Source Address Offset 21.3.18/415
    if (tcd_new.has_SOFF) { tcd_new.has_SOFF = (int16_t)tcd_new.SOFF; }

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
      PB_UPDATE_TEENSY_REG_BIT(tcd_new.CSR, DMA_TCD_CSR, START, CSR)
      tcd.CSR = CSR;
    }
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
    return 0;
  }
}  // namespace dma
}  // namespace teensy

#endif  // #ifndef ___TEENSY__DMA__H___
