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
    result.SC1A.ADCH = adc.SC1A & 0x1F;

    result.has_SC1B = true;
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.SC1B, ADC_SC1, AIEN, adc.SC1B)
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.SC1B, ADC_SC1, COCO, adc.SC1B)
    PB_SET_TEENSY_REG_BIT_FROM_VAL(result.SC1B, ADC_SC1, DIFF, adc.SC1B)
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
    result.CFG2.MUXSEL = (teensy__3_1_adc_R_CFG2_E_MUXSEL)((adc.CFG2 >> 5) & 0x1);

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
}  // namespace adc
}  // namespace teensy
