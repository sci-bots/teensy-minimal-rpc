#include "Node.h"

namespace teensy_minimal_rpc {

void Node::begin() {
  state_.set_buffer(get_buffer());
  state_.validator_.set_node(*this);
  state_.reset();
#if !defined(DISABLE_SERIAL)
  // Start Serial after loading config to set baud rate.
  Serial.begin(115200);
#endif  // #ifndef DISABLE_SERIAL
  adc_ = new ADC();
  Wire.begin();
  Wire.setClock(400000);
}

}  // namespace teensy_minimal_rpc
