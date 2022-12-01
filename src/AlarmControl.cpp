#include "AlarmControl.hpp"
#include "Storage.hpp"

AlarmControl::AlarmControl(LEDControl *led_control) :
  led_control_(led_control)
{
  this->preferences_ = new Preferences();
  // load saved values
  preferences_lock.lock();
  this->preferences_->begin("alarmctrl");

  this->preferences_->getString("alarmtime", this->alarm_time_, sizeof(this->alarm_time_));  // TODO - check format
  this->alarm_time_[5] = '\0';  // safety
  if (strchr(this->alarm_time_, ':') != this->alarm_time_ + 2)
  {
    strcpy(this->alarm_time_, "06:45");
    this->preferences_->putString("alarmtime", this->alarm_time_);
    log_d("Setting default value for alarmtime: %s", this->alarm_time_);
  }

  this->fade_minutes_ = this->preferences_->getUInt("fade_min", 0);
  if (this->fade_minutes_ == 0 || this->fade_minutes_ >= 150)
  {
    this->fade_minutes_ = 30;
    this->preferences_->putUInt("fade_min", this->fade_minutes_);
    log_d("Setting default value for fade_min: %u", this->fade_minutes_);
  }

  this->duty_max_ = this->preferences_->getFloat("duty_max", 0.0f);
  if (this->duty_max_ <= 0.0f || this->duty_max_ > 99.0f)
  {
    this->duty_max_ = 99.0f;
    this->preferences_->putFloat("duty_max", this->duty_max_);
    log_d("Setting default value for duty_max: %.2f", this->duty_max_);
  }

  this->duty_min_ = this->preferences_->getFloat("duty_min", 0.0f);
  if (this->duty_min_ <= 0.0f || this->duty_min_ > 99.0f)
  {
    this->duty_min_ = 1.0f;
    this->preferences_->putFloat("duty_min", this->duty_min_);
    log_d("Setting default value for duty_min: %.2f", this->duty_min_);
  }

  this->preferences_->end();
  preferences_lock.unlock();

  log_i("Initialized Alarm with: alarm_time_=%s fade_minutes_=%u duty_max_=%.2f duty_min_=%.2f", 
        this->alarm_time_, this->fade_minutes_, this->duty_max_, this->duty_min_);
}

String AlarmControl::getAlarmTime(void)
{
  return String(this->alarm_time_);
}

uint32_t AlarmControl::getFadeMinutes(void)
{
  return this->fade_minutes_;
}

float AlarmControl::getDutyMax(void)
{
  return this->duty_max_;
}

float AlarmControl::getDutyMin(void)
{
  return this->duty_min_;
}

LEDControl::LEDMode_t AlarmControl::getMode(void)
{
  return this->led_control_->getMode();
}

void AlarmControl::setOnMode(void)
{
  log_d("setOnMode");
  // TODO signal task
  this->led_control_->setOnMode();
}

void AlarmControl::setOffMode(void)
{
  log_d("setOffMode");
  // TODO signal task,
  this->led_control_->setOffMode();
}

void AlarmControl::setAlarmMode(void)
{
  log_d("setAlarmMode");

}

void AlarmControl::setAlarmTime(String alarm_time)
{
  log_d("setAlarmTime %s", alarm_time);
  
}

void AlarmControl::setFadeMinutes(uint32_t fade_minutes)
{
  this->fade_minutes_ = fade_minutes;
  log_d("setFadeMinutes %u", fade_minutes);
  // TODO
}

void AlarmControl::setDutyMax(float duty_max)
{
  this->duty_max_ = duty_max;
  log_d("setDutyMax %.2f", duty_max);

  // TODO
}

void AlarmControl::setDutyMin(float duty_min)
{
  this->duty_min_ = duty_min;
  log_d("setDutyMin %.2f", duty_min);

  // TODO
}
