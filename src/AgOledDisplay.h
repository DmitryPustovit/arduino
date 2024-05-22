#ifndef _AG_OLED_DISPLAY_H_
#define _AG_OLED_DISPLAY_H_

#include "AgConfigure.h"
#include "AgValue.h"
#include "AirGradient.h"
#include "Main/PrintLog.h"
#include <Arduino.h>

class OledDisplay : public PrintLog {
private:
  Configuration &config;
  AirGradient *ag;
  bool isBegin = false;
  void *u8g2 = NULL;
  Measurements &value;
  bool isDisplayOff = false;

  void showTempHum(bool hasStatus);
  void setCentralText(int y, String text);
  void setCentralText(int y, const char *text);
  void showIcon(int x, int y, const void* icon);

public:
  enum DashboardStatus
  {
    DashBoardStatusNone,          /** No Issue */
    DashBoardStatusWiFiIssue,     /** WiFi Connection issue */
    DashBoardStatusServerIssue,   /** Cloud connection issue */
    DashBoardStatusAddToDashboard,/** Show status "Add To Dashboard"*/
    DashBoardStatusDeviceId       /** Show status: device ID */
  };

  OledDisplay(Configuration &config, Measurements &value,
                Stream &log);
  ~OledDisplay();

  void setAirGradient(AirGradient *ag);
  bool begin(void); 
  void end(void);
  void setText(String &line1, String &line2, String &line3);
  void setText(const char *line1, const char *line2, const char *line3);
  void setText(String &line1, String &line2, String &line3, String &line4);
  void setText(const char *line1, const char *line2, const char *line3,
               const char *line4);
  void showDashboard(void);
  void showDashboard(OledDisplay::DashboardStatus status);
  void setBrightness(int percent);
  void showNewFirmwareVersion(String version);
  void showNewFirmwareUpdating(String percent);
  void showNewFirmwareSuccess(String count);
  void showNewFirmwareFailed(void);
};

#endif /** _AG_OLED_DISPLAY_H_ */
