#include "Arduino.h"
#include "EEPROM.h"
#include "SPI.h"
#include "Wire.h"
#include "LinkedList.h"
#include "Memory.h"  // Required replacing memory functions with stubs returning 0.
#include "ArduinoRpc.h"
#include "nanopb.h"
#include "NadaMQ.h"  // Required replacing `#ifndef AVR` with `#if !defined(AVR) && !defined(__arm__)`
#include "CArrayDefs.h"
#include "RPCBuffer.h"
#include "NodeCommandProcessor.h"
#include "BaseNodeRpc.h"  // Check for changes (may have removed some include statements...
#include "TeensyMinimalRpc.h"
#include "ADC.h"
#include <IntervalTimer.h>
#include "Node.h"


teensy_minimal_rpc::Node node_obj;
teensy_minimal_rpc::CommandProcessor<teensy_minimal_rpc::Node> command_processor(node_obj);
IntervalTimer timer0; // timer

// when the measurement finishes, this will be called
// first: see which pin finished and then save the measurement into the correct buffer
void adc0_isr() {
  node_obj.on_adc_done();
  //ADC0_RA; // clear interrupt
}

void serialEvent() { node_obj.serial_handler_.receiver()(Serial.available()); }


void setup() {
  node_obj.begin();
}


void loop() {
  /* Parse all new bytes that are available.  If the parsed bytes result in a
   * completed packet, pass the complete packet to the command-processor to
   * process the request. */
  if (node_obj.serial_handler_.packet_ready()) {
    node_obj.serial_handler_.process_packet(command_processor);
  }
}


void timer0_callback(void) { node_obj.on_tick(); }
