#ifndef ___TEENSY__DMA__H___
#define ___TEENSY__DMA__H___

#include <DMAChannel.h>
#include <CArrayDefs.h>  // UInt8Array
#include <pb_cpp_api.h>  // nanopb::serialize_to_array
#include <TeensyMinimalRpc/DMA_pb.h>
#include <TeensyMinimalRpc/RegisterMacros.h>


namespace teensy {
namespace dma {
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
  int8_t update_TCD(uint8_t channel_num, teensy__3_1_dma_TCD const &tcd_new);
  int8_t update_TCD(uint8_t channel_num, UInt8Array tcd_data);

  teensy__3_1_dma_Registers registers_to_protobuf();
  UInt8Array serialize_registers(UInt8Array buffer);
  int8_t update_registers(UInt8Array serialized_registers);

  UInt8Array serialize_mux_chcfg(uint32_t channel_num, UInt8Array buffer);
  int8_t update_mux_chcfg(uint32_t channel_num,
                          UInt8Array serialized_mux_chcfg);
  UInt8Array serialize_dchpri(uint32_t channel_num, UInt8Array buffer);
}  // namespace dma
}  // namespace teensy

#endif  // #ifndef ___TEENSY__DMA__H___
