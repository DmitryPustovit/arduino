#ifndef _UTILS_H_
#define _UTILS_H_

#include <Arduino.h>

class utils
{
private:
  /* data */
public:
  utils(/* args */);
  ~utils();

  static bool isValidTemperature(float value);
  static bool isValidHumidity(float value);
  static bool isValidCO2(int16_t value);
  static bool isValidPMS(int value);
  static bool isValidPMS03Count(int value);
  static bool isValidNOx(int value);
  static bool isValidVOC(int value);
  static float getInvalidTemperature(void);
  static float getInvalidHumidity(void);
  static int getInvalidCO2(void);
  static int getInvalidPMS(void);
  static int getInvalidNOx(void);
  static int getInvalidVOC(void);
};


#endif /** _UTILS_H_ */
