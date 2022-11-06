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
  // flash storage for variables
  this->preferences_ = new Preferences();
  // load saved values
  preferences_lock.lock();
  this->preferences_->begin("ledctrl");

  this->frequency_hz_ = this->preferences_->getUInt("frequency", 0);
  if (this->frequency_hz_ <= 100 || this->frequency_hz_ >= 10000)
  {
    this->frequency_hz_ = 1000;
    this->preferences_->putUInt("frequency", this->frequency_hz_);
  }

  this->duty_percent_ = this->preferences_->getFloat("duty", 0.0f);
  if (this->duty_percent_ <= 1.0f || this->duty_percent_ > 99.0f)
  {
    this->duty_percent_ = 50.0f;
    this->preferences_->putFloat("duty", this->duty_percent_);
  }

  this->preferences_->end();
  preferences_lock.unlock();
  
  log_d("init mcpwm driver");
  this->updateTiming(this->frequency_hz_, this->duty_percent_);
}

void LEDControl::updateTiming(uint32_t frequency_hz, float duty_percent)
{
  preferences_lock.lock();
  this->preferences_->begin("ledctrl");

  if (frequency_hz > 100 && frequency_hz < 10000 &&
      duty_percent > 1.0 && duty_percent <= 99.0)
  {
    if (this->frequency_hz_ != frequency_hz)
    {
      this->preferences_->putUInt("frequency", frequency_hz);
    }
    if (this->duty_percent_ != duty_percent)
    {
      this->preferences_->putFloat("duty", duty_percent);
    }

    this->frequency_hz_ = frequency_hz;
    this->duty_percent_ = duty_percent;
  }
  else
  {
    log_e("ERR Invalid frequency: %u  duty-cycle: %f", frequency_hz, duty_percent);
    return;
  }

  this->preferences_->end();
  preferences_lock.unlock();

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
