#include "Node.h"

namespace teensy_minimal_rpc {

void Node::begin() {
  // Start Serial after loading config to set baud rate.
  Serial.begin(115200);
  adc_ = new ADC();
  Wire.begin();
  Wire.setClock(400000);
}

}  // namespace teensy_minimal_rpc
