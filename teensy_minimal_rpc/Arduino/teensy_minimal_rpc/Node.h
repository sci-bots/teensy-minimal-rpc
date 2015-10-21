#ifndef ___NODE__H___
#define ___NODE__H___

#include <stdint.h>
#include <Arduino.h>
#include <NadaMQ.h>
#include <CArrayDefs.h>
#include "RPCBuffer.h"  // Define packet sizes
#include "TeensyMinimalRpc/Properties.h"  // Define package name, URL, etc.
#include <BaseNodeRpc/BaseNode.h>
#include <BaseNodeRpc/BaseNodeSerialHandler.h>
#include <BaseNodeRpc/SerialHandler.h>
#include <ADC.h>
#include <IntervalTimer.h>

extern IntervalTimer timer0; // timer
void timer0_callback(void);


namespace teensy_minimal_rpc {
const size_t FRAME_SIZE = (3 * sizeof(uint8_t)  // Frame boundary
                           - sizeof(uint16_t)  // UUID
                           - sizeof(uint16_t)  // Payload length
                           - sizeof(uint16_t));  // CRC

class Node;


class Node :
  public BaseNode, public BaseNodeSerialHandler {
public:
  typedef PacketParser<FixedPacket> parser_t;

  static const uint16_t BUFFER_SIZE = 128;  // >= longest property string

  uint8_t buffer_[BUFFER_SIZE];
  ADC *adc_;

  Node() : BaseNode() {}

  UInt8Array get_buffer() { return UInt8Array(sizeof(buffer_), buffer_); }
  /* This is a required method to provide a temporary buffer to the
   * `BaseNode...` classes. */

  void begin();
  /****************************************************************************
   * # User-defined methods #
   *
   * Add new methods below.  When Python package is generated using the
   * command, `paver sdist` from the project root directory, the signatures of
   * the methods below will be scanned and code will automatically be generated
   * to support calling the methods from Python over a serial connection.
   *
   * e.g.
   *
   *     bool less_than(float a, float b) { return a < b; }
   *
   * See [`arduino_rpc`][1] and [`base_node_rpc`][2] for more details.
   *
   * [1]: https://github.com/wheeler-microfluidics/arduino_rpc
   * [2]: https://github.com/wheeler-microfluidics/base_node_rpc
   */

  /////////////// METHODS TO SET/GET SETTINGS OF THE ADC ////////////////////

  //! Set the voltage reference you prefer, default is 3.3 V (VCC)
  /*!
  * \param type can be ADC_REF_3V3, ADC_REF_1V2 (not for Teensy LC) or ADC_REF_EXT.
  *
  *  It recalibrates at the end.
  */
  void setReference(uint8_t type, int8_t adc_num) {
    adc_->setReference(type, adc_num);
  }

  void start_timer(uint32_t period) {
    timer0.begin(timer0_callback, period);
  }

  void stop_timer() {
    timer0.end();
  }

  void on_tick() {
    digitalWriteFast(13, !digitalReadFast(13));
  }

  //! Change the resolution of the measurement.
  /*!
  *  \param bits is the number of bits of resolution.
  *  For single-ended measurements: 8, 10, 12 or 16 bits.
  *  For differential measurements: 9, 11, 13 or 16 bits.
  *  If you want something in between (11 bits single-ended for example) select the inmediate higher
  *  and shift the result one to the right.
  *
  *  Whenever you change the resolution, change also the comparison values (if you use them).
  */
  void setResolution(uint8_t bits, int8_t adc_num) {
    adc_->setResolution(bits, adc_num);
  }

  //! Returns the resolution of the ADC_Module.
  uint8_t getResolution(int8_t adc_num) {
    return adc_->getResolution(adc_num);
  }

  //! Returns the maximum value for a measurement: 2^res-1.
  uint32_t getMaxValue(int8_t adc_num) {
    return adc_->getMaxValue(adc_num);
  }


  //! Sets the conversion speed (changes the ADC clock, ADCK)
  /**
  * \param speed can be ADC_VERY_LOW_SPEED, ADC_LOW_SPEED, ADC_MED_SPEED, ADC_HIGH_SPEED_16BITS, ADC_HIGH_SPEED or ADC_VERY_HIGH_SPEED.
  *
  * ADC_VERY_LOW_SPEED is guaranteed to be the lowest possible speed within specs for resolutions less than 16 bits (higher than 1 MHz),
  * it's different from ADC_LOW_SPEED only for 24, 4 or 2 MHz bus frequency.
  * ADC_LOW_SPEED is guaranteed to be the lowest possible speed within specs for all resolutions (higher than 2 MHz).
  * ADC_MED_SPEED is always >= ADC_LOW_SPEED and <= ADC_HIGH_SPEED.
  * ADC_HIGH_SPEED_16BITS is guaranteed to be the highest possible speed within specs for all resolutions (lower or eq than 12 MHz).
  * ADC_HIGH_SPEED is guaranteed to be the highest possible speed within specs for resolutions less than 16 bits (lower or eq than 18 MHz).
  * ADC_VERY_HIGH_SPEED may be out of specs, it's different from ADC_HIGH_SPEED only for 48, 40 or 24 MHz bus frequency.
  *
  * Additionally the conversion speed can also be ADC_ADACK_2_4, ADC_ADACK_4_0, ADC_ADACK_5_2 and ADC_ADACK_6_2,
  * where the numbers are the frequency of the ADC clock (ADCK) in MHz and are independent on the bus speed.
  * This is useful if you are using the Teensy at a very low clock frequency but want faster conversions,
  * but if F_BUS<F_ADCK, you can't use ADC_VERY_HIGH_SPEED for sampling speed.
  *
  */
  void setConversionSpeed(uint8_t speed, int8_t adc_num) {
    adc_->setConversionSpeed(speed, adc_num);
  }


  //! Sets the sampling speed
  /** Increase the sampling speed for low impedance sources, decrease it for higher impedance ones.
  * \param speed can be ADC_VERY_LOW_SPEED, ADC_LOW_SPEED, ADC_MED_SPEED, ADC_HIGH_SPEED or ADC_VERY_HIGH_SPEED.
  *
  * ADC_VERY_LOW_SPEED is the lowest possible sampling speed (+24 ADCK).
  * ADC_LOW_SPEED adds +16 ADCK.
  * ADC_MED_SPEED adds +10 ADCK.
  * ADC_HIGH_SPEED (or ADC_HIGH_SPEED_16BITS) adds +6 ADCK.
  * ADC_VERY_HIGH_SPEED is the highest possible sampling speed (0 ADCK added).
  */
  void setSamplingSpeed(uint8_t speed, int8_t adc_num) {
    adc_->setSamplingSpeed(speed, adc_num);
  }

  uint32_t benchmark_flops(uint32_t N) {
    float a = 1e6;
    float b = 1e7;
    uint32_t start = micros();
    for (int i = 0; i < N; i++) {
      a /= b;
    }
    return (micros() - start);
  }

  uint32_t benchmark_iops(uint32_t N) {
    uint32_t a = 1e6;
    uint32_t b = 1e7;
    uint32_t start = micros();
    for (int i = 0; i < N; i++) {
      a /= b;
    }
    return (micros() - start);
  }

  //! Set the number of averages
  /*!
  * \param num can be 0, 4, 8, 16 or 32.
  */
  void setAveraging(uint8_t num, int8_t adc_num) {
    adc_->setAveraging(num, adc_num);
  }


  //! Enable interrupts
  /** An IRQ_ADC0 Interrupt will be raised when the conversion is completed
  *  (including hardware averages and if the comparison (if any) is true).
  */
  void enableInterrupts(int8_t adc_num) {
    adc_->enableInterrupts(adc_num);
  }

  //! Disable interrupts
  void disableInterrupts(int8_t adc_num) {
    adc_->disableInterrupts(adc_num);
  }


  //! Enable DMA request
  /** An ADC DMA request will be raised when the conversion is completed
  *  (including hardware averages and if the comparison (if any) is true).
  */
  void enableDMA(int8_t adc_num) {
    adc_->enableDMA(adc_num);
  }

  //! Disable ADC DMA request
  void disableDMA(int8_t adc_num) {
    adc_->disableDMA(adc_num);
  }


  //! Enable the compare function to a single value
  /** A conversion will be completed only when the ADC value
  *  is >= compValue (greaterThan=1) or < compValue (greaterThan=0)
  *  Call it after changing the resolution
  *  Use with interrupts or poll conversion completion with isComplete()
  */
  void enableCompare(int16_t compValue, bool greaterThan, int8_t adc_num) {
    adc_->enableCompare(compValue, greaterThan, adc_num);
  }

  //! Enable the compare function to a range
  /** A conversion will be completed only when the ADC value is inside (insideRange=1) or outside (=0)
  *  the range given by (lowerLimit, upperLimit),including (inclusive=1) the limits or not (inclusive=0).
  *  See Table 31-78, p. 617 of the freescale manual.
  *  Call it after changing the resolution
  *  Use with interrupts or poll conversion completion with isComplete()
  */
  void enableCompareRange(int16_t lowerLimit, int16_t upperLimit, bool insideRange, bool inclusive, int8_t adc_num) {
    adc_->enableCompareRange(lowerLimit, upperLimit, insideRange, inclusive, adc_num);
  }

  //! Disable the compare function
  void disableCompare(int8_t adc_num) {
    adc_->disableCompare(adc_num);
  }


  //! Enable and set PGA
  /** Enables the PGA and sets the gain
  *   Use only for signals lower than 1.2 V
  *   \param gain can be 1, 2, 4, 8, 16, 32 or 64
  *
  */
  void enablePGA(uint8_t gain, int8_t adc_num) {
    adc_->enablePGA(gain, adc_num);
  }

  //! Returns the PGA level
  /** PGA level = from 1 to 64
  */
  uint8_t getPGA(int8_t adc_num) {
    return adc_->getPGA(adc_num);
  }

  //! Disable PGA
  void disablePGA(int8_t adc_num) {
    adc_->disablePGA(adc_num);
  }



  ////////////// INFORMATION ABOUT THE STATE OF THE ADC /////////////////

  //! Is the ADC converting at the moment?
  bool isConverting(int8_t adc_num) {
    return adc_->isConverting(adc_num);
  }

  //! Is an ADC conversion ready?
  /**
  *  \return 1 if yes, 0 if not.
  *  When a value is read this function returns 0 until a new value exists
  *  So it only makes sense to call it with continuous or non-blocking methods
  */
  bool isComplete(int8_t adc_num) {
    return adc_->isComplete(adc_num);
  }

  //! Is the ADC in differential mode?
  bool isDifferential(int8_t adc_num) {
    return adc_->isDifferential(adc_num);
  }

  //! Is the ADC in continuous mode?
  bool isContinuous(int8_t adc_num) {
    return adc_->isContinuous(adc_num);
  }



  //////////////// BLOCKING CONVERSION METHODS //////////////////

  //! Returns the analog value of the pin.
  /** It waits until the value is read and then returns the result.
  * If a comparison has been set up and fails, it will return ADC_ERROR_VALUE.
  * This function is interrupt safe, so it will restore the adc to the state it was before being called
  * If more than one ADC exists, it will select the module with less workload, you can force a selection using
  * adc_num. If you select ADC1 in Teensy 3.0 it will return ADC_ERROR_VALUE.
  */
  int analogRead(uint8_t pin, int8_t adc_num) {
    return adc_->analogRead(pin, adc_num);
  }

  //! Reads the differential analog value of two pins (pinP - pinN).
  /** It waits until the value is read and then returns the result.
  * If a comparison has been set up and fails, it will return ADC_ERROR_VALUE.
  * \param pinP must be A10 or A12.
  * \param pinN must be A11 (if pinP=A10) or A13 (if pinP=A12).
  * Other pins will return ADC_ERROR_VALUE.
  *
  * This function is interrupt safe, so it will restore the adc to the state it was before being called
  * If more than one ADC exists, it will select the module with less workload, you can force a selection using
  * adc_num
  */
  int analogReadDifferential(uint8_t pinP, uint8_t pinN, int8_t adc_num) {
    return adc_->analogReadDifferential(pinP, pinN, adc_num);
  }


  /////////////// NON-BLOCKING CONVERSION METHODS //////////////

  //! Starts an analog measurement on the pin and enables interrupts.
  /** It returns inmediately, get value with readSingle().
  *   If the pin is incorrect it returns ADC_ERROR_VALUE
  *   If this function interrupts a measurement, it stores the settings in adc_config
  */
  bool startSingleRead(uint8_t pin, int8_t adc_num) {
    return adc_->startSingleRead(pin, adc_num);
  }

  //! Start a differential conversion between two pins (pinP - pinN) and enables interrupts.
  /** It returns inmediately, get value with readSingle().
  *   \param pinP must be A10 or A12.
  *   \param pinN must be A11 (if pinP=A10) or A13 (if pinP=A12).
  *
  *   Other pins will return ADC_ERROR_DIFF_VALUE.
  *   If this function interrupts a measurement, it stores the settings in adc_config
  */
  bool startSingleDifferential(uint8_t pinP, uint8_t pinN, int8_t adc_num) {
    return adc_->startSingleDifferential(pinP, pinN, adc_num);
  }

  //! Reads the analog value of a single conversion.
  /** Set the conversion with with startSingleRead(pin) or startSingleDifferential(pinP, pinN).
  *   \return the converted value.
  */
  int readSingle(int8_t adc_num) {
    return adc_->readSingle(adc_num);
  }



  ///////////// CONTINUOUS CONVERSION METHODS ////////////

  //! Starts continuous conversion on the pin.
  /** It returns as soon as the ADC is set, use analogReadContinuous() to read the value.
  */
  bool startContinuous(uint8_t pin, int8_t adc_num) {
    return adc_->startContinuous(pin, adc_num);
  }

  //! Starts continuous conversion between the pins (pinP-pinN).
  /** It returns as soon as the ADC is set, use analogReadContinuous() to read the value.
  * \param pinP must be A10 or A12.
  * \param pinN must be A11 (if pinP=A10) or A13 (if pinP=A12).
  * Other pins will return ADC_ERROR_DIFF_VALUE.
  */
  bool startContinuousDifferential(uint8_t pinP, uint8_t pinN, int8_t adc_num) {
    return adc_->startContinuousDifferential(pinP, pinN, adc_num);
  }

  //! Reads the analog value of a continuous conversion.
  /** Set the continuous conversion with with analogStartContinuous(pin) or startContinuousDifferential(pinP, pinN).
  *   \return the last converted value.
  *   If single-ended and 16 bits it's necessary to typecast it to an unsigned type (like uint16_t),
  *   otherwise values larger than 3.3/2 V are interpreted as negative!
  */
  int analogReadContinuous(int8_t adc_num) {
    return adc_->analogReadContinuous(adc_num);
  }

  //! Stops continuous conversion
  void stopContinuous(int8_t adc_num) {
    adc_->stopContinuous(adc_num);
  }



  /////////// SYNCHRONIZED METHODS ///////////////
  ///// IF THE BOARD HAS ONLY ONE ADC, THEY ARE EMPYT METHODS /////

  //! Struct for synchronous measurements
  /** result_adc0 has the result from ADC0 and result_adc1 from ADC1.
  */

  //////////////// SYNCHRONIZED BLOCKING METHODS //////////////////

  //! Returns the analog values of both pins, measured at the same time by the two ADC modules.
  /** It waits until the values are read and then returns the result as a struct Sync_result,
  *   use Sync_result.result_adc0 and Sync_result.result_adc1.
  * If a comparison has been set up and fails, it will return ADC_ERROR_VALUE in both fields of the struct.
  * This function is interrupt safe, so it will restore the adc to the state it was before being called
  */
  Int32Array analogSynchronizedRead(uint8_t pin0, uint8_t pin1) {
    UInt8Array buffer = get_buffer();
    ADC::Sync_result &result = *reinterpret_cast<ADC::Sync_result *>(&buffer.data[0]);
    Int32Array output;
    output.length = sizeof(ADC::Sync_result) / sizeof(int32_t);
    output.data = reinterpret_cast<int32_t *>(&result);

    result = adc_->analogSynchronizedRead(pin0, pin1);
    return output;
  }
#if 0

  Int32Array analogSyncRead(uint8_t pin0, uint8_t pin1) __attribute__((always_inline)) {
    return analogSynchronizedRead(pin0, pin1);
  }

  //! Returns the differential analog values of both sets of pins, measured at the same time by the two ADC modules.
  /** It waits until the values are read and then returns the result as a struct Sync_result,
  *   use Sync_result.result_adc0 and Sync_result.result_adc1.
  * If a comparison has been set up and fails, it will return ADC_ERROR_VALUE in both fields of the struct.
  * This function is interrupt safe, so it will restore the adc to the state it was before being called
  */
  Int32Array analogSynchronizedReadDifferential(uint8_t pin0P, uint8_t pin0N, uint8_t pin1P, uint8_t pin1N) {
    UInt8Array buffer = get_buffer();
    Int32Array output;
    output.length = sizeof(Sync_result) / sizeof(int32_t);
    output.data = reinterpret_cast<int32_t *>(&result);
    Sync_result &result = *reinterpret_cast<Sync_result *>(&buffer.data[0]);

    result = adc_->analogSynchronizedReadDifferential(pin0P, pin0N, pin1P, pin1N);
    return result
  }

  Int32Array analogSyncReadDifferential(uint8_t pin0P, uint8_t pin0N, uint8_t pin1P, uint8_t pin1N) __attribute__((always_inline)) {
      return analogSynchronizedReadDifferential(pin0P, pin0N, pin1P, pin1N);
  }

  /////////////// SYNCHRONIZED NON-BLOCKING METHODS //////////////

  //! Starts an analog measurement at the same time on the two ADC modules
  /** It returns inmediately, get value with readSynchronizedSingle().
  *   If the pin is incorrect it returns false
  *   If this function interrupts a measurement, it stores the settings in adc_config
  */
  bool startSynchronizedSingleRead(uint8_t pin0, uint8_t pin1) {
    return adc_->startSynchronizedSingleRead(pin0, pin1);
  }

  //! Start a differential conversion between two pins (pin0P - pin0N) and (pin1P - pin1N)
  /** It returns inmediately, get value with readSynchronizedSingle().
  *   \param pin0P, pin1P must be A10 or A12.
  *   \param pin0N, pin1N must be A11 (if pinP=A10) or A13 (if pinP=A12).
  *   Other pins will return false.
  *   If this function interrupts a measurement, it stores the settings in adc_config
  */
  bool startSynchronizedSingleDifferential(uint8_t pin0P, uint8_t pin0N, uint8_t pin1P, uint8_t pin1N) {
    return adc_->startSynchronizedSingleDifferential(pin0P, pin0N, pin1P, pin1N);
  }

  //! Reads the analog value of a single conversion.
  /**
  *   \return the converted value.
  */
  Int32Array readSynchronizedSingle() {
    UInt8Array buffer = get_buffer();
    Int32Array output;
    output.length = sizeof(ADC::Sync_result) / sizeof(int32_t);
    output.data = reinterpret_cast<int32_t *>(&result);
    ADC::Sync_result &result = *reinterpret_cast<ADC::Sync_result *>(&buffer.data[0]);

    result = adc_->readSynchronizedSingle();
    return result;
  }


  ///////////// SYNCHRONIZED CONTINUOUS CONVERSION METHODS ////////////

  //! Starts a continuous conversion in both ADCs simultaneously
  /** Use readSynchronizedContinuous to get the values
  *
  */
  bool startSynchronizedContinuous(uint8_t pin0, uint8_t pin1) {
    return adc_->startSynchronizedContinuous(pin0, pin1);
  }

  //! Starts a continuous differential conversion in both ADCs simultaneously
  /** Use readSynchronizedContinuous to get the values
  *
  */
  bool startSynchronizedContinuousDifferential(uint8_t pin0P, uint8_t pin0N, uint8_t pin1P, uint8_t pin1N) {
    return adc_->startSynchronizedContinuousDifferential(pin0P, pin0N, pin1P, pin1N);
  }

  //! Returns the values of both ADCs.
  Int32Array readSynchronizedContinuous() {
    UInt8Array buffer = get_buffer();
    Int32Array output;
    output.length = sizeof(Sync_result) / sizeof(int32_t);
    output.data = reinterpret_cast<int32_t *>(&result);
    Sync_result &result = *reinterpret_cast<Sync_result *>(&buffer.data[0]);

    result = adc_->readSynchronizedContinuous();
    return result;
  }

  //! Stops synchronous continuous conversion
  void stopSynchronizedContinuous() {
    adc_->stopSynchronizedContinuous();
  }
#endif
};

}  // namespace teensy_minimal_rpc


#endif  // #ifndef ___NODE__H___
