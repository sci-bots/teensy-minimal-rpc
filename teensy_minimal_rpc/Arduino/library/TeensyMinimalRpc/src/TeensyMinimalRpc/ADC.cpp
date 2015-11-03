#include "ADC.h"

namespace teensy {
namespace adc {
  UInt8Array serialize_registers(uint8_t adc_num, UInt8Array buffer) {
    volatile AdcRegister_t &adc =
      *(reinterpret_cast<volatile AdcRegister_t *>(&ADC0_SC1A) + adc_num);
    // Cast buffer as ADC_REGISTERS Protocol Buffer message.
    teensy__3_1_adc_Registers result;

    result.has_SC1A = true;
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.SC1A, ADC_SC1, AIEN, adc.SC1A)
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.SC1A, ADC_SC1, COCO, adc.SC1A)
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.SC1A, ADC_SC1, DIFF, adc.SC1A)
    result.SC1A.has_ADCH = true;
    result.SC1A.ADCH = adc.SC1A & 0x1F;

    result.has_SC1B = true;
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.SC1B, ADC_SC1, AIEN, adc.SC1B)
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.SC1B, ADC_SC1, COCO, adc.SC1B)
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.SC1B, ADC_SC1, DIFF, adc.SC1B)
    result.SC1B.has_ADCH = true;
    result.SC1B.ADCH = adc.SC1B & 0x1F;

    result.has_CFG1 = true;
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.CFG1, ADC_CFG1, ADLPC, adc.CFG1)
    result.CFG1.has_ADICLK = true;
    result.CFG1.ADICLK = (teensy__3_1_adc_R_CFG1_E_ADICLK)(adc.CFG1 & 0x3);
    result.CFG1.has_ADIV = true;
    result.CFG1.ADIV = (teensy__3_1_adc_R_CFG1_E_ADIV)((adc.CFG1 >> 5) & 0x3);
    result.CFG1.has_ADLSMP = true;
    result.CFG1.ADLSMP = (teensy__3_1_adc_R_CFG1_E_ADLSMP)(adc.CFG1 & ADC_CFG1_ADLSMP);
    result.CFG1.has_MODE = true;
    result.CFG1.MODE = (teensy__3_1_adc_R_CFG1_E_MODE)((adc.CFG1 >> 2) & 0x3);

    result.has_CFG2 = true;
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.CFG2, ADC_CFG2, ADACKEN, adc.CFG2)
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.CFG2, ADC_CFG2, ADHSC, adc.CFG2)
    result.CFG2.has_ADLSTS = true;
    result.CFG2.ADLSTS = (teensy__3_1_adc_R_CFG2_E_ADLSTS)(adc.CFG2 & 0x3);
    result.CFG2.has_MUXSEL = true;
    result.CFG2.MUXSEL = (teensy__3_1_adc_R_CFG2_E_MUXSEL)((adc.CFG2 >> 4) & 0x1);

    result.has_RA = true;
    result.RA = adc.RA;
    result.has_RB = true;
    result.RB = adc.RB;

    result.has_CV1 = true;
    result.CV1 = adc.CV1;
    result.has_CV2 = true;
    result.CV2 = adc.CV2;

    result.has_SC2 = true;
    result.has_SC3 = true;

    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.SC2, ADC_SC2, ACFE, adc.SC2)
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.SC2, ADC_SC2, ACFGT, adc.SC2)
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.SC2, ADC_SC2, ACREN, adc.SC2)
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.SC2, ADC_SC2, ADACT, adc.SC2)
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.SC2, ADC_SC2, DMAEN, adc.SC2)
    result.SC2.has_ADTRG = true;
    result.SC2.ADTRG = (teensy__3_1_adc_R_SC2_E_ADTRG)(adc.SC2 & ADC_SC2_ADTRG);
    result.SC2.has_REFSEL = true;
    result.SC2.REFSEL = (teensy__3_1_adc_R_SC2_E_REFSEL)(adc.SC2 & 0x3);

    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.SC3, ADC_SC3, ADCO, adc.SC3)
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.SC3, ADC_SC3, AVGE, adc.SC3)
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.SC3, ADC_SC3, CAL, adc.SC3)
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.SC3, ADC_SC3, CALF, adc.SC3)
    result.SC3.has_AVGS = true;
    result.SC3.AVGS = (teensy__3_1_adc_R_SC3_E_AVGS)(adc.SC3 & 0x3);

    result.has_OFS = true;
    result.OFS = adc.OFS;

    result.has_PGA = true;
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.PGA, ADC_PGA, PGAEN, adc.PGA)
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.PGA, ADC_PGA, PGALPB, adc.PGA)
    result.PGA.has_PGAG = true;
    result.PGA.PGAG = (teensy__3_1_adc_R_PGA_E_PGAG)((adc.PGA >> 16) & 0xF);

    result.has_CLM0 = true;
    result.CLM0 = adc.CLM0;
    result.has_CLM1 = true;
    result.CLM1 = adc.CLM1;
    result.has_CLM2 = true;
    result.CLM2 = adc.CLM2;
    result.has_CLM3 = true;
    result.CLM3 = adc.CLM3;
    result.has_CLM4 = true;
    result.CLM4 = adc.CLM4;
    result.has_CLMD = true;
    result.CLMD = adc.CLMD;
    result.has_CLMS = true;
    result.CLMS = adc.CLMS;
    result.has_CLP0 = true;
    result.CLP0 = adc.CLP0;
    result.has_CLP1 = true;
    result.CLP1 = adc.CLP1;
    result.has_CLP2 = true;
    result.CLP2 = adc.CLP2;
    result.has_CLP3 = true;
    result.CLP3 = adc.CLP3;
    result.has_CLP4 = true;
    result.CLP4 = adc.CLP4;
    result.has_CLPD = true;
    result.CLPD = adc.CLPD;
    result.has_CLPS = true;
    result.CLPS = adc.CLPS;
    result.has_MG = true;
    result.MG = adc.MG;
    result.has_PG = true;
    result.PG = adc.PG;

    UInt8Array output =
      nanopb::serialize_to_array(result, teensy__3_1_adc_Registers_fields,
                                 buffer);
    return output;
  }

  int8_t update_registers(uint8_t adc_num, UInt8Array serialized_registers) {
    // Create empty ADC Registers Protocol Buffer message.
    teensy__3_1_adc_Registers adc_msg = teensy__3_1_adc_Registers_init_default;

    bool ok = nanopb::decode_from_array(serialized_registers,
                                        teensy__3_1_adc_Registers_fields,
                                        adc_msg);

    if (!ok) { return -1; }

    volatile AdcRegister_t &adc =
      *(reinterpret_cast<volatile AdcRegister_t *>(&ADC0_SC1A) + adc_num);
    // Cast buffer as ADC_REGISTERS Protocol Buffer message.

    if (adc_msg.has_SC1A) {
      uint32_t SC1A = adc.SC1A;

      PB_UPDATE_TEENSY_REG_BIT(adc_msg.SC1A, ADC_SC1, AIEN, SC1A)
      PB_UPDATE_TEENSY_REG_BIT(adc_msg.SC1A, ADC_SC1, COCO, SC1A)
      PB_UPDATE_TEENSY_REG_BIT(adc_msg.SC1A, ADC_SC1, DIFF, SC1A)
      PB_UPDATE_TEENSY_REG_BITS(adc_msg.SC1A, 5, 0, ADCH, SC1A)
    }

    if (adc_msg.has_SC1B) {
      uint32_t SC1B = adc.SC1B;

      PB_UPDATE_TEENSY_REG_BIT(adc_msg.SC1B, ADC_SC1, AIEN, SC1B)
      PB_UPDATE_TEENSY_REG_BIT(adc_msg.SC1B, ADC_SC1, COCO, SC1B)
      PB_UPDATE_TEENSY_REG_BIT(adc_msg.SC1B, ADC_SC1, DIFF, SC1B)
      PB_UPDATE_TEENSY_REG_BITS(adc_msg.SC1B, 5, 0, ADCH, SC1B)
      adc.SC1B = SC1B;
    }

    if (adc_msg.has_CFG1) {
      uint32_t CFG1 = adc.CFG1;

      PB_UPDATE_TEENSY_REG_BIT(adc_msg.CFG1, ADC_CFG1, ADLPC, CFG1)
      PB_UPDATE_TEENSY_REG_BITS(adc_msg.CFG1, 2, 5, ADIV, CFG1)
      PB_UPDATE_TEENSY_REG_BIT(adc_msg.CFG1, ADC_CFG1, ADLSMP, CFG1)
      PB_UPDATE_TEENSY_REG_BITS(adc_msg.CFG1, 2, 2, MODE, CFG1)
      PB_UPDATE_TEENSY_REG_BITS(adc_msg.CFG1, 2, 0, ADICLK, CFG1)

      adc.CFG1 = CFG1;
    }

    if (adc_msg.has_CFG2) {
      uint32_t CFG2 = adc.CFG2;

      PB_UPDATE_TEENSY_REG_BIT(adc_msg.CFG2, ADC_CFG2, MUXSEL, CFG2)
      PB_UPDATE_TEENSY_REG_BIT(adc_msg.CFG2, ADC_CFG2, ADACKEN, CFG2)
      PB_UPDATE_TEENSY_REG_BIT(adc_msg.CFG2, ADC_CFG2, ADHSC, CFG2)
      PB_UPDATE_TEENSY_REG_BITS(adc_msg.CFG2, 2, 0, ADLSTS, CFG2)

      adc.CFG2 = CFG2;
    }

    if (adc_msg.has_RA) { adc.RA = adc_msg.RA; }
    if (adc_msg.has_RB) { adc.RB = adc_msg.RB; }
    if (adc_msg.has_CV1) { adc.CV1 = adc_msg.CV1; }
    if (adc_msg.has_CV2) { adc.CV2 = adc_msg.CV2; }

    if (adc_msg.has_SC2) {
      uint32_t SC2 = adc.SC2;

      PB_UPDATE_TEENSY_REG_BIT(adc_msg.SC2, ADC_SC2, ADACT, SC2)
      PB_UPDATE_TEENSY_REG_BIT(adc_msg.SC2, ADC_SC2, ADTRG, SC2)
      PB_UPDATE_TEENSY_REG_BIT(adc_msg.SC2, ADC_SC2, ACFE, SC2)
      PB_UPDATE_TEENSY_REG_BIT(adc_msg.SC2, ADC_SC2, ACFGT, SC2)
      PB_UPDATE_TEENSY_REG_BIT(adc_msg.SC2, ADC_SC2, ACREN, SC2)
      PB_UPDATE_TEENSY_REG_BIT(adc_msg.SC2, ADC_SC2, DMAEN, SC2)
      PB_UPDATE_TEENSY_REG_BITS(adc_msg.SC2, 2, 0, REFSEL, SC2)

      adc.SC2 = SC2;
    }

    if (adc_msg.has_SC3) {
      uint32_t SC3 = adc.SC3;

      PB_UPDATE_TEENSY_REG_BIT(adc_msg.SC3, ADC_SC3, CAL, SC3)
      PB_UPDATE_TEENSY_REG_BIT(adc_msg.SC3, ADC_SC3, CALF, SC3)
      PB_UPDATE_TEENSY_REG_BIT(adc_msg.SC3, ADC_SC3, ADCO, SC3)
      PB_UPDATE_TEENSY_REG_BIT(adc_msg.SC3, ADC_SC3, AVGE, SC3)
      PB_UPDATE_TEENSY_REG_BITS(adc_msg.SC3, 2, 0, AVGS, SC3)

      adc.SC3 = SC3;
    }

    if (adc_msg.has_OFS) { adc.OFS = adc_msg.OFS; }

    adc_msg.has_PGA = true;
    if (adc_msg.has_PGA) {
      uint32_t PGA = adc.PGA;

      PB_UPDATE_TEENSY_REG_BIT(adc_msg.PGA, ADC_PGA, PGAEN, PGA)
      PB_UPDATE_TEENSY_REG_BIT(adc_msg.PGA, ADC_PGA, PGALPB, PGA)
      PB_UPDATE_TEENSY_REG_BITS(adc_msg.PGA, 4, 16, PGAG, PGA)

      adc.PGA = PGA;
    }

    if (adc_msg.has_CLM0) { adc.CLM0 = adc_msg.CLM0; }
    if (adc_msg.has_CLM1) { adc.CLM1 = adc_msg.CLM1; }
    if (adc_msg.has_CLM2) { adc.CLM2 = adc_msg.CLM2; }
    if (adc_msg.has_CLM3) { adc.CLM3 = adc_msg.CLM3; }
    if (adc_msg.has_CLM4) { adc.CLM4 = adc_msg.CLM4; }
    if (adc_msg.has_CLMD) { adc.CLMD = adc_msg.CLMD; }
    if (adc_msg.has_CLMS) { adc.CLMS = adc_msg.CLMS; }
    if (adc_msg.has_CLP0) { adc.CLP0 = adc_msg.CLP0; }
    if (adc_msg.has_CLP1) { adc.CLP1 = adc_msg.CLP1; }
    if (adc_msg.has_CLP2) { adc.CLP2 = adc_msg.CLP2; }
    if (adc_msg.has_CLP3) { adc.CLP3 = adc_msg.CLP3; }
    if (adc_msg.has_CLP4) { adc.CLP4 = adc_msg.CLP4; }
    if (adc_msg.has_CLPD) { adc.CLPD = adc_msg.CLPD; }
    if (adc_msg.has_CLPS) { adc.CLPS = adc_msg.CLPS; }
    if (adc_msg.has_MG) { adc.MG = adc_msg.MG; }
    if (adc_msg.has_PG) { adc.PG = adc_msg.PG; }

    //return 0;
    return adc_msg.CFG2.ADLSTS;
  }
}  // namespace adc
}  // namespace teensy
