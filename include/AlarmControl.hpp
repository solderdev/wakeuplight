#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "LEDControl.hpp"

class AlarmControl
{
public:
  typedef enum {
    ALARMMODE_FORCE_ON = 0,
    ALARMMODE_ALARM_ON = 1,
    ALARMMODE_ALARM_OFF = 2,
    ALARMMODE_LIMIT
  } AlarmMode_t;

  explicit AlarmControl(LEDControl *led_control);
  AlarmControl(AlarmControl const&) = delete;
  void operator=(AlarmControl const&)  = delete;

  String getAlarmTime(void);
  bool getAlarmWeekend(void);
  uint32_t getFadeMinutes(void);
  float getDutyMax(void);
  float getDutyMin(void);
  AlarmMode_t getMode(void);
  void setOnMode(void);
  void setMode(AlarmControl::AlarmMode_t mode);
  void setAlarmOFF(void);
  void setAlarmON(void);
  void setAlarmTime(String alarm_time);
  void setAlarmWeekend(bool alarm_weekend);
  void setFadeMinutes(uint32_t fade_minutes);
  void setDutyMax(float duty_max);
  void setDutyMin(float duty_min);

private:
  static void task_alarm_wrapper(void *arg);
  void task_alarm();
  LEDControl *led_control_;
  Preferences *preferences_;
  char alarm_time_[10]; // TODO check max size
  char alarm_time_nvm_[10]; // TODO check max size
  bool alarm_weekend_;
  bool alarm_weekend_nvm_;
  uint32_t fade_minutes_;
  uint32_t fade_minutes_nvm_;
  float duty_max_;
  float duty_max_nvm_;
  float duty_min_;
  float duty_min_nvm_;
  AlarmMode_t mode_;
  AlarmMode_t mode_nvm_;

  TaskHandle_t task_handle_alarm_;
};
