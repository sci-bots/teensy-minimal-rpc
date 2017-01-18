#ifndef ___TEENSY__SIM__H___
#define ___TEENSY__SIM__H___

#include <CArrayDefs.h>  // UInt8Array
#include <pb_cpp_api.h>  // nanopb::serialize_to_array
#include <TeensyMinimalRpc/SIM_pb.h>
#include <TeensyMinimalRpc/RegisterMacros.h>


namespace teensy {
namespace sim {
  inline UInt8Array serialize_SCGC6(UInt8Array buffer) {
    /*
     * # (32-bit) System Clock Gating Control Register 6 (12.2.13/256) #
     *
     * Serialize System Clock Gating Control Register 6 (SCGC6) to Protocol
     * Buffer message format.
     */
    // (32-bit) System Clock Gating Control Register 6 (12.2.13/256)
    // Create empty SCGC6 Register Protocol Buffer message.
    teensy__3_1_sim_R_SCGC6 result = teensy__3_1_sim_R_SCGC6_init_default;

    // Set value of each Protocol Buffer field according to value of
    // corresponding bit in `SIM_SCGC6` register.
    PB_SET_TEENSY_REG_BIT(result, SIM_SCGC6, RTC)  // RTC Access Control
    PB_SET_TEENSY_REG_BIT(result, SIM_SCGC6, ADC0)  // ADC0 Clock Gate Control
    PB_SET_TEENSY_REG_BIT(result, SIM_SCGC6, FTM1)  // FTM1 Clock Gate Control
    PB_SET_TEENSY_REG_BIT(result, SIM_SCGC6, FTM0)  // FTM0 Clock Gate Control
    PB_SET_TEENSY_REG_BIT(result, SIM_SCGC6, PIT)  // PIT Clock Gate Control
    PB_SET_TEENSY_REG_BIT(result, SIM_SCGC6, PDB)  // PDB Clock Gate Control
    PB_SET_TEENSY_REG_BIT(result, SIM_SCGC6, USBDCD)  // USB DCD Clock Gate Control
    PB_SET_TEENSY_REG_BIT(result, SIM_SCGC6, CRC)  // CRC Clock Gate Control
    PB_SET_TEENSY_REG_BIT(result, SIM_SCGC6, I2S)  // I2S Clock Gate Control
    PB_SET_TEENSY_REG_BIT(result, SIM_SCGC6, SPI1)  // SPI1 Clock Gate Control
    PB_SET_TEENSY_REG_BIT(result, SIM_SCGC6, SPI0)  // SPI0 Clock Gate Control
    PB_SET_TEENSY_REG_BIT(result, SIM_SCGC6, FLEXCAN0)  // FlexCAN0 Clock Gate Control
    PB_SET_TEENSY_REG_BIT(result, SIM_SCGC6, DMAMUX)  // DMA Mux Clock Gate Control
    PB_SET_TEENSY_REG_BIT(result, SIM_SCGC6, FTFL)  // Flash Memory Clock Gate Control

    UInt8Array output =
      nanopb::serialize_to_array(result, teensy__3_1_sim_R_SCGC6_fields,
                                 buffer);
    return output;
  }

  inline UInt8Array serialize_SCGC7(UInt8Array buffer) {
    /*
     * # (32-bit) System Clock Gating Control Register 7 (12.2.14/259) #
     *
     * Serialize System Clock Gating Control Register 7 (SCGC7) to Protocol
     * Buffer message format.
     */
    // Create empty SCGC7 Register Protocol Buffer message.
    teensy__3_1_sim_R_SCGC7 result = teensy__3_1_sim_R_SCGC7_init_default;

    // Set value of each Protocol Buffer field according to value of
    // corresponding bit in `SIM_SCGC7` register.
    PB_SET_TEENSY_REG_BIT(result, SIM_SCGC7, DMA)  // DMA Clock Gate Control

    UInt8Array output =
      nanopb::serialize_to_array(result, teensy__3_1_sim_R_SCGC7_fields,
                                 buffer);
    return output;
  }

  inline int8_t update_SCGC6(UInt8Array serialized_scgc6) {
    /*
     * # (32-bit) System Clock Gating Control Register 6 (12.2.13/256) #
     *
     * Update System Clock Gating Control Register 6 (SCGC6) according to
     * fields set in the provided Protocol Buffer message.
     */
    // Create empty  Registers Protocol Buffer message.
    teensy__3_1_sim_R_SCGC6 scgc6 = teensy__3_1_sim_R_SCGC6_init_default;

    bool ok = nanopb::decode_from_array(serialized_scgc6,
                                        teensy__3_1_sim_R_SCGC6_fields, scgc6);
    if (!ok) { return -1; }

    uint32_t SCGC6 = SIM_SCGC6;

    // Update bit fields in `SCGC6` value with values that are explicitly set
    // in the `scgc6` Protocol Buffer message.
    PB_UPDATE_TEENSY_REG_BIT(scgc6, SIM_SCGC6, RTC, SCGC6)  // RTC Access Control
    PB_UPDATE_TEENSY_REG_BIT(scgc6, SIM_SCGC6, ADC0, SCGC6)  // ADC0 Clock Gate Control
    PB_UPDATE_TEENSY_REG_BIT(scgc6, SIM_SCGC6, FTM1, SCGC6)  // FTM1 Clock Gate Control
    PB_UPDATE_TEENSY_REG_BIT(scgc6, SIM_SCGC6, FTM0, SCGC6)  // FTM0 Clock Gate Control
    PB_UPDATE_TEENSY_REG_BIT(scgc6, SIM_SCGC6, PIT, SCGC6)  // PIT Clock Gate Control
    PB_UPDATE_TEENSY_REG_BIT(scgc6, SIM_SCGC6, PDB, SCGC6)  // PDB Clock Gate Control
    PB_UPDATE_TEENSY_REG_BIT(scgc6, SIM_SCGC6, USBDCD, SCGC6)  // USB DCD Clock Gate Control
    PB_UPDATE_TEENSY_REG_BIT(scgc6, SIM_SCGC6, CRC, SCGC6)  // CRC Clock Gate Control
    PB_UPDATE_TEENSY_REG_BIT(scgc6, SIM_SCGC6, I2S, SCGC6)  // I2S Clock Gate Control
    PB_UPDATE_TEENSY_REG_BIT(scgc6, SIM_SCGC6, SPI1, SCGC6)  // SPI1 Clock Gate Control
    PB_UPDATE_TEENSY_REG_BIT(scgc6, SIM_SCGC6, SPI0, SCGC6)  // SPI0 Clock Gate Control
    PB_UPDATE_TEENSY_REG_BIT(scgc6, SIM_SCGC6, FLEXCAN0, SCGC6)  // FlexCAN0 Clock Gate Control
    PB_UPDATE_TEENSY_REG_BIT(scgc6, SIM_SCGC6, DMAMUX, SCGC6)  // DMA Mux Clock Gate Control
    PB_UPDATE_TEENSY_REG_BIT(scgc6, SIM_SCGC6, FTFL, SCGC6)  // Flash Memory Clock Gate Control

    SIM_SCGC6 = SCGC6;
    return 0;
  }

  inline int8_t update_SCGC7(UInt8Array serialized_scgc7) {
    // (32-bit) System Clock Gating Control Register 7 (12.2.14/259)
    // Create empty  Registers Protocol Buffer message.
    teensy__3_1_sim_R_SCGC7 scgc7 = teensy__3_1_sim_R_SCGC7_init_default;

    bool ok = nanopb::decode_from_array(serialized_scgc7,
                                        teensy__3_1_sim_R_SCGC7_fields, scgc7);
    if (!ok) { return -1; }

    uint32_t SCGC7 = SIM_SCGC7;

    // Update bit fields in `SCGC7` value with values that are explicitly set
    // in the `scgc7` Protocol Buffer message.
    PB_UPDATE_TEENSY_REG_BIT(scgc7, SIM_SCGC7, DMA, SCGC7);  // DMA Clock Gate Control

    SIM_SCGC7 = SCGC7;
    return 0;
  }
}  // namespace sim
}  // namespace teensy

#endif  // #ifndef ___TEENSY__SIM__H___

