#include <esp32-hal-log.h>

float mapExp(int value, int inMin, int inMax, float outMin, float outMax) {
  float norm = (float)(value - inMin) / (inMax - inMin);
  return outMin * pow(outMax / outMin, norm);
}

float mapLog(float value, float inMin, float inMax, float outMin, float outMax) {
  if (value < inMin) value = inMin;
  if (value > inMax) value = inMax;
  float scale = log(value / inMin) / log(inMax / inMin);
  return outMin + scale * (outMax - outMin);
}

int clamp(int val, int minVal, int maxVal) {
  return max(minVal, min(val, maxVal));
}
