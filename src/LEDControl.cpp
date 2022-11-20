#include "LEDControl.hpp"
#include "Storage.hpp"

// https://forum.arduino.cc/t/esp32-mcpwm-trigger/907542/9
// https://github.com/espressif/esp-idf/blob/master/examples/peripherals/mcpwm/mcpwm_bdc_speed_control/main/mcpwm_bdc_control_example_main.c
// alternative (not exact): https://github.com/khoih-prog/ESP32_PWM
// alternative to switch off with one-shot: https://demo-dijiudu.readthedocs.io/en/latest/api-reference/system/esp_timer.html


LEDControl::LEDControl(uint8_t ctrl_pin_1, mcpwm_unit_t mcpwm_unit_id) :
  mcpwm_unit_(mcpwm_unit_id), 
  ctrl_pin_1(ctrl_pin_1)
{
  this->frequency_hz_ = 20000;
  this->duty_percent_ = 50.0f;
  
  log_d("init mcpwm driver");
  this->updateTiming(this->frequency_hz_, this->duty_percent_);
}

void LEDControl::updateTiming(uint32_t frequency_hz, float duty_percent)
{
  if (frequency_hz > 100 && frequency_hz < 30000 &&
      duty_percent > 1.0 && duty_percent <= 99.0)
  {
    this->frequency_hz_ = frequency_hz;
    this->duty_percent_ = duty_percent;
  }
  else
  {
    log_e("ERR Invalid frequency: %u  duty-cycle: %f", frequency_hz, duty_percent);
    return;
  }

  log_i("Setting frequency: %u", this->frequency_hz_);
  log_i("Setting duty-cycle: %f", this->duty_percent_);

  mcpwm_gpio_init(this->mcpwm_unit_, MCPWM0A, this->ctrl_pin_1);

  // set PWM-mode
  this->mode = 2;

  mcpwm_config_t pwm_config = {
      .frequency = this->frequency_hz_,
      .cmpr_a = 0,
      .cmpr_b = 0,
      .duty_mode = MCPWM_DUTY_MODE_0,
      .counter_mode = MCPWM_DOWN_COUNTER,
  };
  mcpwm_init(this->mcpwm_unit_, MCPWM_TIMER_0, &pwm_config);
  
  mcpwm_set_duty(this->mcpwm_unit_, MCPWM_TIMER_0, MCPWM_GEN_A, 100.0f - this->duty_percent_);

  mcpwm_set_duty_type(this->mcpwm_unit_, MCPWM_TIMER_0, MCPWM_GEN_A, MCPWM_DUTY_MODE_0);

  // activate timer output
  mcpwm_set_timer_sync_output(this->mcpwm_unit_, MCPWM_TIMER_0, MCPWM_SWSYNC_SOURCE_TEZ);
}

uint32_t LEDControl::getFrequencyHz(void)
{
  return this->frequency_hz_;
}

float LEDControl::getDutyPercent(void)
{
  return this->duty_percent_;
}

uint8_t LEDControl::getMode(void)
{
  return this->mode;
}

void LEDControl::setOnMode(void)
{
  this->mode = 0;
  mcpwm_set_signal_low(this->mcpwm_unit_, MCPWM_TIMER_0, MCPWM_GEN_A);
}

void LEDControl::setOffMode(void)
{
  this->mode = 1;
  mcpwm_set_signal_high(this->mcpwm_unit_, MCPWM_TIMER_0, MCPWM_GEN_A);
}

void LEDControl::setPwmMode(void)
{
  this->updateTiming(this->frequency_hz_, this->duty_percent_);
}

void LEDControl::setFrequency(uint32_t frequency)
{
  this->updateTiming(frequency, this->duty_percent_);
}

void LEDControl::setDutyCycle(float duty_cycle)
{
  this->updateTiming(this->frequency_hz_, duty_cycle);
}
