#ifndef ___TEENSY__REGISTER_MACROS__H___
#define ___TEENSY__REGISTER_MACROS__H___


/*
 * Update bit in `OUTPUT` value with boolean field value that is explicitly set
 * in the `MSG` Protocol Buffer message.
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
 * Update bit-range in `OUTPUT` value with integer field value that is
 * explicitly set in the `MSG` Protocol Buffer message.
 *  - `MSG`: Name of protocol buffer message instance.
 *  - `width`: Bit-width of register field.
 *  - `shift`: Position of least-significant bit of field within register.
 *  - `FIELD`: Name of integer field (e.g., `INTMAJOR`).
 *  - `OUTPUT`: Name of output variable to set bit range in.
 *
 *  The `=&` operation clears current bit range contents in the output variable
 *  and the `|=` operation sets the new values for the bit range.
 */
#define PB_UPDATE_TEENSY_REG_BITS(MSG, width, shift, FIELD, OUTPUT) \
  if ( MSG.has_##FIELD ) { \
    uint32_t mask = 0; \
    for (int mask_i = 0; mask_i < width; mask_i++) { \
      mask |= (1 << mask_i); \
    } \
    mask <<= shift; \
    OUTPUT &= ~mask; \
    OUTPUT |= MSG.FIELD << shift; \
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
