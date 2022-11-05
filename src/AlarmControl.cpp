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
  // TODO check if correct
  strcpy(this->alarm_time_, "21:23");

  this->fade_minutes_ = this->preferences_->getUInt("fade_min", 30);
  if (this->fade_minutes_ == 0 || this->fade_minutes_ >= 150)
  {
    this->fade_minutes_ = 30;
    this->preferences_->putUInt("fade_min", this->fade_minutes_);
    Serial.println("Setting default value for fade_min: " + String(this->fade_minutes_));
  }

  this->duty_max_ = this->preferences_->getFloat("duty_max", 99.0f);
  if (this->duty_max_ <= 0.0f || this->duty_max_ > 99.0f)
  {
    this->duty_max_ = 99.0f;
    this->preferences_->putFloat("duty_max", this->duty_max_);
    Serial.println("Setting default value for duty_max: " + String(this->duty_max_));
  }

  this->duty_min_ = this->preferences_->getFloat("duty_min", 99.0f);
  if (this->duty_min_ <= 0.0f || this->duty_min_ > 99.0f)
  {
    this->duty_min_ = 1.0f;
    this->preferences_->putFloat("duty_min", this->duty_min_);
    Serial.println("Setting default value for duty_min: " + String(this->duty_min_));
  }

  this->preferences_->end();
  preferences_lock.unlock();

  Serial.println("Initialized Alarm with:" 
                 " alarm_time_=" + String(this->alarm_time_) + 
                 " fade_minutes_=" + String(this->fade_minutes_) + 
                 " duty_max_" + String(this->duty_max_) + 
                 " duty_min_" + String(this->duty_min_));
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

uint8_t AlarmControl::getMode(void)
{
  return this->led_control_->getMode();
}

void AlarmControl::setOnMode(void)
{
  Serial.println("setOnMode");
  // TODO signal task
  this->led_control_->setOnMode();
}

void AlarmControl::setOffMode(void)
{
  Serial.println("setOffMode");
  // TODO signal task,
  this->led_control_->setOffMode();
}

void AlarmControl::setAlarmMode(void)
{
  Serial.println("setAlarmMode");

}

void AlarmControl::setAlarmTime(String alarm_time)
{
  Serial.printf("setAlarmTime %s\n", alarm_time);
  
}

void AlarmControl::setFadeMinutes(uint32_t fade_minutes)
{
  this->fade_minutes_ = fade_minutes;
  Serial.printf("setFadeMinutes %d\n", fade_minutes);
  // TODO
}

void AlarmControl::setDutyMax(float duty_max)
{
  this->duty_max_ = duty_max;
  Serial.printf("setDutyMax %f\n", duty_max);

  // TODO
}

void AlarmControl::setDutyMin(float duty_min)
{
  this->duty_min_ = duty_min;
  Serial.printf("setDutyMin %f\n", duty_min);

  // TODO
}
