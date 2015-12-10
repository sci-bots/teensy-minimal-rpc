#ifndef ___TEENSY__PIT__H___
#define ___TEENSY__PIT__H___

#include <kinetis.h>
#include <CArrayDefs.h>  // UInt8Array
#include <pb_cpp_api.h>  // nanopb::serialize_to_array
#include <TeensyMinimalRpc/PIT_pb.h>
#include <TeensyMinimalRpc/RegisterMacros.h>


namespace teensy {
namespace pit {
  inline volatile uint32_t *timer_index_to_registers_addr(uint8_t index) {
    // Per-timer registers start at address of `PIT_LDVAL0`, with four 32-bit
    // registers per timer.
    return &PIT_LDVAL0 + 4 * index;
  }

  UInt8Array serialize_registers(UInt8Array buffer);
  int8_t update_registers(UInt8Array serialized_registers);

  UInt8Array serialize_timer_config(uint8_t index, UInt8Array buffer);
  int8_t update_timer_config(uint32_t index, UInt8Array serialized_config);

}  // namespace pit
}  // namespace teensy

#endif  // #ifndef ___TEENSY__PIT__H___

