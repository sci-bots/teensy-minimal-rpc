#include "PIT.h"

namespace teensy {
namespace pit {

  UInt8Array serialize_timer_config(uint8_t index, UInt8Array buffer) {
    teensy__3_1_pit_TimerConfig result =
      teensy__3_1_pit_TimerConfig_init_default;

    // Get pointer to Timer Load Value Register (37.3.2/904)
    volatile uint32_t *TIMER_REG = timer_index_to_registers_addr(index);

    // Get reference to Timer Load Value Register (37.3.2/904)
    volatile uint32_t &LDVAL = *TIMER_REG++;
    // Get reference to Timer Control Register (37.3.4/905).
    volatile uint32_t &CVAL = *TIMER_REG++;
    // Get reference to Timer Control Register (37.3.4/905).
    volatile uint32_t &TCTRL = *TIMER_REG++;
    // Get reference to Timer Flag Register (37.3.5/906)
    volatile uint32_t &TFLG = *TIMER_REG++;

    result.has_LDVAL = true;
    result.LDVAL = LDVAL;

    result.has_CVAL = true;
    result.CVAL = CVAL;

    result.has_TCTRL = true;
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.TCTRL, PIT_TCTRL, CHN, TCTRL) // Chain Mode (37.3.4/905)
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.TCTRL, PIT_TCTRL, TIE, TCTRL) // Timer Interrupt Enable (37.3.4/905)
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.TCTRL, PIT_TCTRL, TEN, TCTRL) // Timer Enable (37.3.4/905)

    result.has_TFLG = true;

    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.TFLG, PIT_TFLG, TIF, TFLG) // Timer Interrupt Flag (37.3.5/906)

    UInt8Array output =
      nanopb::serialize_to_array(result, teensy__3_1_pit_TimerConfig_fields,
                                 buffer);
    return output;
  }

  int8_t update_timer_config(uint32_t index, UInt8Array serialized_config) {
    teensy__3_1_pit_TimerConfig pit_msg =
      teensy__3_1_pit_TimerConfig_init_default;

    bool ok = nanopb::decode_from_array(serialized_config,
                                        teensy__3_1_pit_TimerConfig_fields,
                                        pit_msg);
    if (!ok) { return -1; }

    volatile uint32_t *TIMER_REG = timer_index_to_registers_addr(index);

    // Get reference to Timer Load Value Register (37.3.2/904)
    volatile uint32_t &LDVAL = *TIMER_REG++;
    // Get reference to Timer Control Register (37.3.4/905).
    volatile uint32_t &CVAL = *TIMER_REG++;
    // Get reference to Timer Control Register (37.3.4/905).
    volatile uint32_t &TCTRL = *TIMER_REG++;
    // Get reference to Timer Flag Register (37.3.5/906)
    volatile uint32_t &TFLG = *TIMER_REG++;

    if (pit_msg.has_LDVAL) { LDVAL = pit_msg.LDVAL; } // Timer Load Value Register (37.3.2/904)
    if (pit_msg.has_CVAL) { CVAL = pit_msg.CVAL; }    // Current Timer Value Register (37.3.3/905)
    if (pit_msg.has_TCTRL)  {
      uint32_t tctrl = TCTRL;
      PB_UPDATE_TEENSY_REG_BIT(pit_msg.TCTRL, PIT_TCTRL, CHN, tctrl) // Chain Mode (37.3.4/905)
      PB_UPDATE_TEENSY_REG_BIT(pit_msg.TCTRL, PIT_TCTRL, TIE, tctrl) // Timer Interrupt Enable (37.3.4/905)
      PB_UPDATE_TEENSY_REG_BIT(pit_msg.TCTRL, PIT_TCTRL, TEN, tctrl) // Timer Enable (37.3.4/905)
      TCTRL = tctrl;
    }
    PB_UPDATE_TEENSY_REG_BIT(pit_msg.TFLG, PIT_TFLG, TIF, TFLG) // Timer Interrupt Flag (37.3.5/906)

    return 0;
  }

  UInt8Array serialize_registers(UInt8Array buffer) {
    teensy__3_1_pit_Registers result =
      teensy__3_1_pit_Registers_init_default;

    result.has_MCR = true;
    PB_SET_TEENSY_REG_BIT(result.MCR, PIT_MCR, MDIS) // Module Disable (37.3.1/903)
    PB_SET_TEENSY_REG_BIT(result.MCR, PIT_MCR, FRZ) // Freeze (37.3.1/903)

    UInt8Array output =
      nanopb::serialize_to_array(result, teensy__3_1_pit_Registers_fields,
                                 buffer);
    return output;
  }

  int8_t update_registers(UInt8Array serialized_registers) {
    teensy__3_1_pit_Registers pit_msg =
      teensy__3_1_pit_Registers_init_default;

    bool ok = nanopb::decode_from_array(serialized_registers,
                                        teensy__3_1_pit_Registers_fields,
                                        pit_msg);
    if (!ok) { return -1; }


    if (pit_msg.has_MCR)  {
      uint32_t pit_mcr = PIT_MCR;
      PB_UPDATE_TEENSY_REG_BIT(pit_msg.MCR, PIT_MCR, MDIS, pit_mcr) // Module Disable (37.3.1/903)
      PB_UPDATE_TEENSY_REG_BIT(pit_msg.MCR, PIT_MCR, FRZ, pit_mcr) // Freeze (37.3.1/903)
      PIT_MCR = pit_mcr;
    }

    return 0;
  }
}  // namespace pit
}  // namespace teensy

