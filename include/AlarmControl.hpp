#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "LEDControl.hpp"

class AlarmControl
{
public:
  AlarmControl(LEDControl *led_control);
  AlarmControl(AlarmControl const&) = delete;
  void operator=(AlarmControl const&)  = delete;

  String getAlarmTime(void);
  uint32_t getFadeMinutes(void);
  float getDutyMax(void);
  float getDutyMin(void);
  uint8_t getMode(void);
  void setOnMode(void);
  void setOffMode(void);
  void setAlarmMode(void);
  void setAlarmTime(String alarm_time);
  void setFadeMinutes(uint32_t fade_minutes);
  void setDutyMax(float duty_max);
  void setDutyMin(float duty_min);

private:
  LEDControl *led_control_;
  Preferences *preferences_;
  char alarm_time_[10]; // TODO check max size
  uint32_t fade_minutes_;
  float duty_max_;
  float duty_min_;
};
