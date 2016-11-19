#ifndef ___ROOT_MEAN_SQUARE__HPP___
#define ___ROOT_MEAN_SQUARE__HPP___

#include <cmath>

namespace teensy_minimal_rpc {

template <typename T, typename S>
float compute_mean(const T *data, S size) {
  float sum = 0;

  for (S i = 0; i < size; i++) { sum += data[i]; }
  return sum / size;
}


template <typename T, typename S>
float compute_sub_rms(const T *data, S size, float bias) {
  float sum_squared = 0;

  for (S i = 0; i < size; i++) {
    const float data_i = data[i] - bias;
    sum_squared += data_i * data_i;
  }

  return sqrt(sum_squared / float(size));
}


template <typename T, typename S>
float compute_mean_sub_rms(const T *data, S size) {
    float mean = compute_mean(data, size);
    return compute_sub_rms(data, size, mean);
}


template <typename T, typename S>
float compute_rms(const T *data, S size) {
  float sum_squared = 0;

  for (S i = 0; i < size; i++) {
    const float data_i = data[i];
    sum_squared += data_i * data_i;
  }

  return sqrt(sum_squared / float(size));
}

}  // namespace teensy_minimal_rpc
#endif  // #ifndef ___ROOT_MEAN_SQUARE__HPP___
