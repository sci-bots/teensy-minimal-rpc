#ifndef ___TEENSY__ADC__H___
#define ___TEENSY__ADC__H___

#include <kinetis.h>  // ADC register addresses, etc.
#include <CArrayDefs.h>  // UInt8Array
#include <pb_cpp_api.h>  // nanopb::serialize_to_array
#include <TeensyMinimalRpc/ADC_pb.h>
#include <TeensyMinimalRpc/RegisterMacros.h>


namespace teensy {
namespace adc {
  struct AdcRegister_t {
    volatile uint32_t SC1A;
    volatile uint32_t SC1B;
    volatile uint32_t CFG1;
    volatile uint32_t CFG2;
    volatile uint32_t RA;
    volatile uint32_t RB;
    volatile uint32_t CV1;
    volatile uint32_t CV2;
    volatile uint32_t SC2;
    volatile uint32_t SC3;
    volatile uint32_t OFS;
    volatile uint32_t PG;
    volatile uint32_t MG;
    volatile uint32_t CLPD;
    volatile uint32_t CLPS;
    volatile uint32_t CLP4;
    volatile uint32_t CLP3;
    volatile uint32_t CLP2;
    volatile uint32_t CLP1;
    volatile uint32_t CLP0;
    volatile uint32_t PGA;
    volatile uint32_t CLMD;
    volatile uint32_t CLMS;
    volatile uint32_t CLM4;
    volatile uint32_t CLM3;
    volatile uint32_t CLM2;
    volatile uint32_t CLM1;
    volatile uint32_t CLM0;
  };

  UInt8Array serialize_registers(uint8_t adc_num, UInt8Array buffer);
  int8_t update_registers(uint8_t adc_num, UInt8Array serialized_registers);
}  // namespace adc
}  // namespace teensy

#endif  // #ifndef ___TEENSY__ADC__H___

