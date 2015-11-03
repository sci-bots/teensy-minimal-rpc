#include "ADC.h"

namespace teensy {
namespace adc {
  UInt8Array serialize_registers(uint8_t adc_num, UInt8Array buffer) {
    volatile AdcRegister_t &adc =
      *(reinterpret_cast<volatile AdcRegister_t *>(&ADC0_SC1A) + adc_num);
    // Cast buffer as ADC_REGISTERS Protocol Buffer message.
    teensy__3_1_adc_Registers result;

    result.has_SC1A = true;
    result.SC1A.has_AIEN = true;
    result.SC1A.AIEN = adc.SC1A & ADC_SC1_AIEN;
    result.SC1A.has_COCO = true;
    result.SC1A.COCO = adc.SC1A & ADC_SC1_COCO;
    result.SC1A.has_DIFF = true;
    result.SC1A.DIFF = adc.SC1A & ADC_SC1_DIFF;
    result.SC1A.has_ADCH = true;
    result.SC1A.ADCH = adc.SC1A & 0x1F;

    result.has_SC1B = true;
    result.SC1B.has_AIEN = true;
    result.SC1B.AIEN = adc.SC1B & ADC_SC1_AIEN;
    result.SC1B.has_COCO = true;
    result.SC1B.COCO = adc.SC1B & ADC_SC1_COCO;
    result.SC1B.has_DIFF = true;
    result.SC1B.DIFF = adc.SC1B & ADC_SC1_DIFF;
    result.SC1B.has_ADCH = true;
    result.SC1B.ADCH = adc.SC1B & 0x1F;

    result.has_CFG1 = true;
    result.CFG1.has_ADLPC = true;
    result.CFG1.ADLPC = adc.CFG1 & ADC_CFG1_ADLPC;
    result.CFG1.has_ADICLK = true;
    result.CFG1.ADICLK = (teensy__3_1_adc_R_CFG1_E_ADICLK)(adc.CFG1 & 0x3);
    result.CFG1.has_ADIV = true;
    result.CFG1.ADIV = (teensy__3_1_adc_R_CFG1_E_ADIV)((adc.CFG1 >> 5) & 0x3);
    result.CFG1.has_ADLSMP = true;
    result.CFG1.ADLSMP = (teensy__3_1_adc_R_CFG1_E_ADLSMP)(adc.CFG1 & ADC_CFG1_ADLSMP);
    result.CFG1.has_MODE = true;
    result.CFG1.MODE = (teensy__3_1_adc_R_CFG1_E_MODE)((adc.CFG1 >> 2) & 0x3);

    result.has_CFG2 = true;
    result.CFG2.has_ADACKEN = true;
    result.CFG2.ADACKEN = adc.CFG2 & ADC_CFG2_ADACKEN;
    result.CFG2.has_ADHSC = true;
    result.CFG2.ADHSC = adc.CFG2 & ADC_CFG2_ADHSC;
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

    result.SC2.has_ACFE = true;
    result.SC2.ACFE = adc.SC2 & ADC_SC2_ACFE;
    result.SC2.has_ACFGT = true;
    result.SC2.ACFGT = adc.SC2 & ADC_SC2_ACFGT;
    result.SC2.has_ACREN = true;
    result.SC2.ACREN = adc.SC2 & ADC_SC2_ACREN;
    result.SC2.has_ADACT = true;
    result.SC2.ADACT = adc.SC2 & ADC_SC2_ADACT;
    result.SC2.has_DMAEN = true;
    result.SC2.DMAEN = adc.SC2 & ADC_SC2_DMAEN;
    result.SC2.has_ADTRG = true;
    result.SC2.ADTRG = (teensy__3_1_adc_R_SC2_E_ADTRG)(adc.SC2 & ADC_SC2_ADTRG);
    result.SC2.has_REFSEL = true;
    result.SC2.REFSEL = (teensy__3_1_adc_R_SC2_E_REFSEL)(adc.SC2 & 0x3);

    result.SC3.has_ADCO = true;
    result.SC3.ADCO = adc.SC3 & ADC_SC3_ADCO;
    result.SC3.has_AVGE = true;
    result.SC3.AVGE = adc.SC3 & ADC_SC3_AVGE;
    result.SC3.has_CAL = true;
    result.SC3.CAL = adc.SC3 & ADC_SC3_CAL;
    result.SC3.has_CALF = true;
    result.SC3.CALF = adc.SC3 & ADC_SC3_CALF;
    result.SC3.has_AVGS = true;
    result.SC3.AVGS = (teensy__3_1_adc_R_SC3_E_AVGS)(adc.SC3 & 0x3);

    result.has_OFS = true;
    result.OFS = adc.OFS;

    result.has_PGA = true;
    result.PGA.has_PGAEN = true;
    result.PGA.PGAEN = adc.PGA & ADC_PGA_PGAEN;
    result.PGA.has_PGALPb = true;
    result.PGA.PGALPb = adc.PGA & ADC_PGA_PGALPB;
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
