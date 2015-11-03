#ifndef ___TEENSY__DMA__H___
#define ___TEENSY__DMA__H___

#include <DMAChannel.h>
#include <CArrayDefs.h>  // UInt8Array
#include <pb_cpp_api.h>  // nanopb::serialize_to_array
#include <TeensyMinimalRpc/DMA_pb.h>
#include <TeensyMinimalRpc/RegisterMacros.h>


namespace teensy {
namespace dma {
#ifdef bitWrite
#undef bitWrite
#endif
  template <typename T>
  inline void bitWrite (T & value, const uint8_t bit, const uint8_t val) {
    if (val) {
      value |= (T(1) << bit);
    } else {
      value &= ~(T(1) << bit);
    }
  }

#ifdef bitRead
#undef bitRead
#endif
  template <typename T>
  inline bool bitRead (const T & value, const uint8_t bit) {
    return (value & ((T)1 << bit)) != 0;
  }

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
        if (tcd_new.ATTR.has_SMOD) {
            ATTR &= ~(0x1F << 3);
            ATTR |= tcd_new.ATTR.SMOD << 3;
        }
        if (tcd_new.ATTR.has_SSIZE) {
            ATTR &= ~0x3;
            ATTR |= tcd_new.ATTR.SSIZE;
        }
        if (tcd_new.ATTR.has_SMOD || tcd_new.ATTR.has_SSIZE) {
            tcd.ATTR_SRC = ATTR;
        }

        ATTR = 0;
        if (tcd_new.ATTR.has_DMOD) {
            ATTR &= ~(0x1F << 3);
            ATTR |= tcd_new.ATTR.DMOD << 3;
        }
        if (tcd_new.ATTR.has_DSIZE) {
            ATTR &= ~0x3;
            ATTR |= tcd_new.ATTR.DSIZE;
        }
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
        if (tcd_new.NBYTES_MLOFFYES.has_SMLOE) {
          bitWrite(NBYTES, 31, tcd_new.NBYTES_MLOFFYES.SMLOE);
        }
        if (tcd_new.NBYTES_MLOFFYES.has_DMLOE) {
          bitWrite(NBYTES, 30, tcd_new.NBYTES_MLOFFYES.DMLOE);
        }
        if (tcd_new.NBYTES_MLOFFYES.has_MLOFF) {
          NBYTES &= ~(0xFFFF << 10);
          NBYTES |= tcd_new.NBYTES_MLOFFYES.MLOFF << 10;
        }
        if (tcd_new.NBYTES_MLOFFYES.has_NBYTES) {
          NBYTES &= ~0x3FF;
          NBYTES |= tcd_new.NBYTES_MLOFFYES.NBYTES;
        }
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
      if (tcd_new.CITER_ELINKYES.has_ELINK) {
        bitWrite(CITER, 15, tcd_new.CITER_ELINKYES.ELINK);
      }
      if (tcd_new.CITER_ELINKYES.has_LINKCH) {
        CITER &= ~(0xF << 9);  // Clear existing LINKCH
        CITER |= tcd_new.CITER_ELINKYES.LINKCH << 9;  // Set new LINKCH
      }
      if (tcd_new.CITER_ELINKYES.has_ITER) {
        CITER &= ~(0x1FF);  // Clear existing ITER
        CITER |= tcd_new.CITER_ELINKYES.ITER;  // Set new ITER
      }
      tcd.CITER_ELINKYES = CITER;  // Update hardware TCD register.
    } else if (tcd_new.has_CITER_ELINKNO) {
      // (16 bits) 21.3.27/422
      uint16_t CITER = tcd.CITER_ELINKNO;
      if (tcd_new.CITER_ELINKNO.has_ELINK) {
        bitWrite(CITER, 15, tcd_new.CITER_ELINKNO.ELINK);
      }
      if (tcd_new.CITER_ELINKNO.has_ITER) {
        CITER &= ~(0x7FFF);  // Clear existing ITER
        CITER |= tcd_new.CITER_ELINKNO.ITER;  // Set new ITER
      }
      tcd.CITER_ELINKNO = CITER;  // Update hardware TCD register.
    }

    // (32 bits) TCD Last Destination Address Adjustment/Scatter Gather Address 21.3.28/423
    if (tcd_new.has_DLASTSGA) { tcd.DLASTSGA = tcd_new.DLASTSGA; }

    // (16 bits) TCD Control and Status 21.3.29/424
    if (tcd_new.has_CSR) {
      uint16_t CSR = tcd.CSR;
      if (tcd_new.CSR.has_BWC) {
        CSR &= ~(0x3 << 14);  // Clear existing BWC
        CSR |= tcd_new.CSR.BWC << 14;  // Set new BWC
      }
      if (tcd_new.CSR.has_MAJORLINKCH) {
        CSR &= ~(0xF << 8);  // Clear existing MAJORLINKCH
        CSR |= tcd_new.CSR.MAJORLINKCH << 8;  // Set new MAJORLINKCH
      }
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
      if (tcd_new.BITER_ELINKYES.has_ELINK) {
        bitWrite(BITER, 15, tcd_new.BITER_ELINKYES.ELINK);
      }
      if (tcd_new.BITER_ELINKYES.has_LINKCH) {
        BITER &= ~(0xF << 9);  // Clear existing LINKCH
        BITER |= tcd_new.BITER_ELINKYES.LINKCH << 9;  // Set new LINKCH
      }
      if (tcd_new.BITER_ELINKYES.has_ITER) {
        BITER &= ~(0x1FF);  // Clear existing ITER
        BITER |= tcd_new.BITER_ELINKYES.ITER;  // Set new ITER
      }
      tcd.BITER_ELINKYES = BITER;  // Update hardware TCD register.
    } else if (tcd_new.has_BITER_ELINKNO) {
      // (16 bits) TCD Beginning Minor Loop Link, Major Loop Count (Channel Linking Disabled) 21.3.31/427
      uint16_t BITER = tcd.BITER_ELINKNO;
      if (tcd_new.BITER_ELINKNO.has_ELINK) {
        bitWrite(BITER, 15, tcd_new.BITER_ELINKNO.ELINK);
      }
      if (tcd_new.BITER_ELINKNO.has_ITER) {
        BITER &= ~(0x7FFF);  // Clear existing ITER
        BITER |= tcd_new.BITER_ELINKNO.ITER;  // Set new ITER
      }
      tcd.BITER_ELINKNO = BITER;  // Update hardware TCD register.
    }
    return 0;
  }
}  // namespace dma
}  // namespace teensy

#endif  // #ifndef ___TEENSY__DMA__H___
