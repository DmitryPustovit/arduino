#include "PMS.h"
#include "../Main/BoardDef.h"

/**
 * @brief Initializes the sensor and attempts to read data.
 *
 * @param stream UART stream
 * @return true Sucecss
 * @return false Failure
 */
bool PMSBase::begin(Stream *stream) {
  Serial.printf("initializing PM sensor\n");
  this->stream = stream;

  failed = true;
  failCount = 0;
  lastRead = 0; // To read buffer on handle without wait after 1.5sec

  // empty first
  int bytesCleared = 0;
  while (this->stream->read() != -1) {
    bytesCleared++;
  }
  Serial.printf("cleared %d byte(s)\n", bytesCleared);

  // explicitly put the sensor into active mode, this seems to be be needed for the Cubic PM2009X
  Serial.printf("setting active mode\n");
  uint8_t activeModeCommand[] = { 0x42, 0x4D, 0xE1, 0x00, 0x01, 0x01, 0x71 };
  size_t bytesWritten = this->stream->write(activeModeCommand, sizeof(activeModeCommand));
  Serial.printf("%d byte(s) written\n", bytesWritten);

  // Run and check sensor data for 4sec
  while (1) {
    handle();
    if (failed == false) {
      Serial.printf("PM sensor initialized\n");
      return true; 
    }

    delay(10);
    uint32_t ms = (uint32_t)(millis() - lastRead);
    if (ms >= 4000) {
      break;
    }
  }
  Serial.printf("PM sensor initialization failed\n");
  return false;
}

/**
 * @brief Check and read sensor data then update variable.
 * Check result from method @isFailed before get value
 */
void PMSBase::handle() {
  uint32_t ms;
  if (lastRead == 0) {
    lastRead = millis();
    if (lastRead == 0) {
      lastRead = 1;
    }
  } else {
    ms = (uint32_t)(millis() - lastRead);
    /**
     * The PMS in Active mode sends an update data every 1 second. If we read
     * exactly every 1 sec then we may or may not get an update (depending on
     * timing tolerances). Hence we read every 2.5 seconds and expect 2 ..3
     * updates,
     */
    if (ms < 2500) {
      return;
    }
  }
  bool result = false;
  char buf[32];
  int bufIndex;
  int step = 0;
  int len = 0;
  int bcount = 0;

  while (stream->available()) {
    char value = stream->read();
    switch (step) {
    case 0: {
      if (value == 0x42) {
        step = 1;
        bufIndex = 0;
        buf[bufIndex++] = value;
      }
      break;
    }
    case 1: {
      if (value == 0x4d) {
        step = 2;
        buf[bufIndex++] = value;
        // Serial.println("Got 0x4d");
      } else {
        step = 0;
      }
      break;
    }
    case 2: {
      buf[bufIndex++] = value;
      if (bufIndex >= 4) {
        len = toI16(&buf[2]);
        if (len != 28) {
          // Serial.printf("Got good bad len %d\r\n", len);
          len += 4;
          step = 3;
        } else {
          // Serial.println("Got good len");
          step = 4;
        }
      }
      break;
    }
    case 3: {
      bufIndex++;
      if (bufIndex >= len) {
        step = 0;
        // Serial.println("Bad lengh read all buffer");
      }
      break;
    }
    case 4: {
      buf[bufIndex++] = value;
      if (bufIndex >= 32) {
        result |= validate(buf);
        step = 0;
        // Serial.println("Got data");
      }
      break;
    }
    default:
      break;
    }

    // Reduce core panic: delay 1 ms each 32bytes data
    bcount++;
    if ((bcount % 32) == 0) {
      delay(1);
    }
  }

  if (result) {
    lastRead = millis();
    if (lastRead == 0) {
      lastRead = 1;
    }
    failed = false;
  } else {
    if (ms > 5000) {
      failed = true;
    }
  }
}

/**
 * @brief Check that PMS send is failed or disconnected
 *
 * @return true Failed
 * @return false No problem
 */
bool PMSBase::isFailed(void) { return failed; }

/**
 * @brief Increate number of fail
 * 
 */
void PMSBase::updateFailCount(void) {
  if (failCount < failCountMax) {
    failCount++;
  }
}

void PMSBase::resetFailCount(void) { failCount = 0; }

/**
 * @brief Get number of fail
 * 
 * @return int 
 */
int PMSBase::getFailCount(void) { return failCount; }

int PMSBase::getFailCountMax(void) { return failCountMax; }

/**
 * @brief Read PMS 0.1 ug/m3 with CF = 1 PM estimates
 *
 * @return uint16_t
 */
uint16_t PMSBase::getRaw0_1(void) { return toU16(&package[4]); }

/**
 * @brief Read PMS 2.5 ug/m3 with CF = 1 PM estimates
 *
 * @return uint16_t
 */
uint16_t PMSBase::getRaw2_5(void) { return toU16(&package[6]); }

/**
 * @brief Read PMS 10 ug/m3 with CF = 1 PM estimates
 *
 * @return uint16_t
 */
uint16_t PMSBase::getRaw10(void) { return toU16(&package[8]); }

/**
 * @brief Read PMS 0.1 ug/m3
 *
 * @return uint16_t
 */
uint16_t PMSBase::getPM0_1(void) { return toU16(&package[10]); }

/**
 * @brief Read PMS 2.5 ug/m3
 *
 * @return uint16_t
 */
uint16_t PMSBase::getPM2_5(void) { return toU16(&package[12]); }

/**
 * @brief Read PMS 10 ug/m3
 *
 * @return uint16_t
 */
uint16_t PMSBase::getPM10(void) { return toU16(&package[14]); }

/**
 * @brief Get numnber concentrations over 0.3 um/0.1L
 *
 * @return uint16_t
 */
uint16_t PMSBase::getCount0_3(void) { return toU16(&package[16]); }

/**
 * @brief Get numnber concentrations over 0.5 um/0.1L
 *
 * @return uint16_t
 */
uint16_t PMSBase::getCount0_5(void) { return toU16(&package[18]); }

/**
 * @brief Get numnber concentrations over 1.0 um/0.1L
 *
 * @return uint16_t
 */
uint16_t PMSBase::getCount1_0(void) { return toU16(&package[20]); }

/**
 * @brief Get numnber concentrations over 2.5 um/0.1L
 *
 * @return uint16_t
 */
uint16_t PMSBase::getCount2_5(void) { return toU16(&package[22]); }

/**
 * @brief Get numnber concentrations over 5.0 um/0.1L (only PMS5003)
 *
 * @return uint16_t
 */
uint16_t PMSBase::getCount5_0(void) { return toU16(&package[24]); }

/**
 * @brief Get numnber concentrations over 10.0 um/0.1L (only PMS5003)
 *
 * @return uint16_t
 */
uint16_t PMSBase::getCount10(void) { return toU16(&package[26]); }

/**
 * @brief Get temperature (only PMS5003T)
 *
 * @return uint16_t
 */
int16_t PMSBase::getTemp(void) { return toI16(&package[24]); }

/**
 * @brief Get humidity (only PMS5003T)
 *
 * @return uint16_t
 */
uint16_t PMSBase::getHum(void) { return toU16(&package[26]); }

/**
 * @brief Get firmware version code
 * 
 * @return uint8_t 
 */
uint8_t PMSBase::getFirmwareVersion(void) { return package[28]; }

/**
 * @brief Ge PMS5003 error code
 * 
 * @return uint8_t 
 */
uint8_t PMSBase::getErrorCode(void) { return package[29]; }

/**
 * @brief Convert PMS2.5 to US AQI unit
 *
 * @param pm02
 * @return int
 */
int PMSBase::pm25ToAQI(int pm02) {
  if (pm02 <= 12.0)
    return ((50 - 0) / (12.0 - .0) * (pm02 - .0) + 0);
  else if (pm02 <= 35.4)
    return ((100 - 50) / (35.4 - 12.0) * (pm02 - 12.0) + 50);
  else if (pm02 <= 55.4)
    return ((150 - 100) / (55.4 - 35.4) * (pm02 - 35.4) + 100);
  else if (pm02 <= 150.4)
    return ((200 - 150) / (150.4 - 55.4) * (pm02 - 55.4) + 150);
  else if (pm02 <= 250.4)
    return ((300 - 200) / (250.4 - 150.4) * (pm02 - 150.4) + 200);
  else if (pm02 <= 350.4)
    return ((400 - 300) / (350.4 - 250.4) * (pm02 - 250.4) + 300);
  else if (pm02 <= 500.4)
    return ((500 - 400) / (500.4 - 350.4) * (pm02 - 350.4) + 400);
  else
    return 500;
}

/**
 * @brief Correction PM2.5
 * 
 * Formula: https://www.airgradient.com/documentation/correction-algorithms/
 * 
 * @param pm25 Raw PM2.5 value
 * @param humidity Humidity value (%)
 * @return int 
 */
int PMSBase::compensate(int pm25, float humidity) {
  float value;
  float fpm25 = pm25;
  if (humidity < 0) {
    humidity = 0;
  }
  if (humidity > 100) {
    humidity = 100.0f;
  }

  if(pm25 < 30) { /** pm2.5 < 30 */
    value = (fpm25 * 0.524f) - (humidity * 0.0862f) + 5.75f;
  } else if(pm25 < 50) { /** 30 <= pm2.5 < 50 */
    value = (0.786f * (fpm25 * 0.05f - 1.5f) + 0.524f * (1.0f - (fpm25 * 0.05f - 1.5f))) * fpm25 - (0.0862f * humidity) + 5.75f;
  } else if(pm25 < 210) { /** 50 <= pm2.5 < 210 */
    value = (0.786f * fpm25) - (0.0862f * humidity) + 5.75f;
  } else if(pm25 < 260) { /** 210 <= pm2.5 < 260 */
    value = (0.69f * (fpm25 * 0.02f - 4.2f) + 0.786f * (1.0f - (fpm25 * 0.02f - 4.2f))) * fpm25 - (0.0862f * humidity * (1.0f - (fpm25 * 0.02f - 4.2f))) + (2.966f * (fpm25 * 0.02f - 4.2f)) + (5.75f * (1.0f - (fpm25 * 0.02f - 4.2f))) + (8.84f * (1.e-4) * fpm25 * fpm25 * (fpm25 * 0.02f - 4.2f));
  } else { /** 260 <= pm2.5 */
    value = 2.966f + (0.69f * fpm25) + (8.84f * (1.e-4) * fpm25 * fpm25);
  }

  if(value < 0) {
    value = 0;
  }

  return (int)value;
}

/**
 * @brief   Convert two byte value to uint16_t value
 *
 * @param buf bytes array (must be >= 2)
 * @return int16_t
 */
int16_t PMSBase::toI16(char *buf) {
  int16_t value = buf[0];
  value = (value << 8) | buf[1];
  return value;
}

uint16_t PMSBase::toU16(char *buf) {
  uint16_t value = buf[0];
  value = (value << 8) | buf[1];
  return value;
}

/**
 * @brief Validate package data
 *
 * @param buf Package buffer
 * @return true Success
 * @return false Failed
 */
bool PMSBase::validate(char *buf) {
  uint16_t sum = 0;
  for (int i = 0; i < 30; i++) {
    sum += buf[i];
  }
  if (sum == toU16(&buf[30])) {
    for (int i = 0; i < 32; i++) {
      package[i] = buf[i];
    }
    return true;
  }
  return false;
}
