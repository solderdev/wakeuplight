#pragma once

#include <Arduino.h>
#include "driver/mcpwm.h"

class LEDControl
{
public:
  LEDControl(uint8_t ctrl_pin_1, mcpwm_unit_t mcpwm_unit_id);
  LEDControl(LEDControl const&) = delete;
  void operator=(LEDControl const&)  = delete;

  void updateTiming(uint32_t frequency_hz, float duty_percent);
  uint32_t getFrequencyHz(void);
  float getDutyPercent(void);
  uint8_t getMode(void);
  void setOnMode(void);
  void setOffMode(void);
  void setPwmMode(void);
  void setFrequency(uint32_t frequency);
  void setDutyCycle(float duty_cycle);

private:
  uint8_t ctrl_pin_1;
  mcpwm_unit_t mcpwm_unit_;
  uint32_t frequency_hz_;
  float duty_percent_;
  uint8_t mode = 2; //on == 0, off == 1, pwm == 2
};
