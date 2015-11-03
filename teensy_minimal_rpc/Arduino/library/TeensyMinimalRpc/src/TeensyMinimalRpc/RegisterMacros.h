#ifndef ___TEENSY__REGISTER_MACROS__H___
#define ___TEENSY__REGISTER_MACROS__H___


/*
 * Update bits in `OUTPUT` value with boolean field values that are
 * explicitly set in the `MSG` Protocol Buffer message.
 *  - `MSG`: Name of protocol buffer message instance.
 *  - `REG`: Name of Teensy register (e.g., `DMA_TCD_CSR`).
 *  - `FIELD`: Name of boolean field (e.g., `INTMAJOR`).
 *  - `OUTPUT`: Name of output variable to set bit in.
 */
#define PB_UPDATE_TEENSY_REG_BIT(MSG, REG, FIELD, OUTPUT) \
  if ( MSG.has_##FIELD ) { \
   if ( MSG.FIELD ) { \
     OUTPUT |= REG##_##FIELD; \
   } else { \
     OUTPUT &= ~REG##_##FIELD; \
   } \
  }
/*
 * Set value of each Protocol Buffer field according to value of corresponding
 * bit in `REG` register.
 */
#define PB_SET_TEENSY_REG_BIT(MSG, REG, FIELD) \
  MSG.has_##FIELD = true; \
  MSG.FIELD = REG & REG##_##FIELD;
/*
 * Set value of each Protocol Buffer field according to value of corresponding
 * bit (identified by bit mask for `REG`) in `VAL` variable.
 *
 * This macro allows, e.g., a register to be pre-loaded into a variable, `VAL`,
 * to perform bit look-ups.
 */
#define PB_SET_TEENSY_REG_BIT_FROM_VAL(MSG, REG, FIELD, VAL) \
  MSG.has_##FIELD = true; \
  MSG.FIELD = VAL & REG##_##FIELD;


#endif  // #ifndef ___TEENSY__REGISTER_MACROS__H___
