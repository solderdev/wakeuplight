#include "AlarmControl.hpp"
#include "Storage.hpp"
#include "TaskConfig.hpp"

AlarmControl::AlarmControl(LEDControl *led_control) :
  led_control_(led_control),
  task_handle_alarm_(nullptr)
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

  this->alarm_weekend_ = this->preferences_->getBool("alarmwe", false);

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

  if (xTaskCreate(&AlarmControl::task_alarm_wrapper, "task_alarm", TaskConfig::Alarm_stacksize, this, TaskConfig::Alarm_priority, &task_handle_alarm_) != pdPASS)
    log_e("Alarm ERROR task init failed");
}

String AlarmControl::getAlarmTime(void)
{
  return String(this->alarm_time_);
}

bool AlarmControl::getAlarmWeekend(void)
{
  return String(this->alarm_weekend_);
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

AlarmControl::AlarmMode_t AlarmControl::getMode(void)
{
  return this->mode_;
}

void AlarmControl::setOnMode(void)
{
  log_d("setOnMode");
  this->mode_ = ALARMMODE_FORCE_ON;
}

void AlarmControl::setAlarmOFF(void)
{
  log_d("setAlarmOFF");
  this->mode_ = ALARMMODE_ALARM_OFF;
}

void AlarmControl::setAlarmON(void)
{
  log_d("setAlarmON");
  this->mode_ = ALARMMODE_ALARM_ON;
}

void AlarmControl::setAlarmTime(String alarm_time)
{
  log_d("setAlarmTime %s", alarm_time);
  if (String(this->alarm_time_) != alarm_time)
  {
    preferences_lock.lock();
    this->preferences_->begin("alarmctrl");
    this->preferences_->putString("alarmtime", alarm_time.c_str());
    this->preferences_->end();
    preferences_lock.unlock();
  }
  strcpy(this->alarm_time_, alarm_time.c_str());
}

void AlarmControl::setAlarmWeekend(bool alarm_weekend)
{
  log_d("setAlarmWeekend %d", alarm_weekend);
  if (this->alarm_weekend_ != alarm_weekend)
  {
    preferences_lock.lock();
    this->preferences_->begin("alarmctrl");
    this->preferences_->putBool("alarmwe", alarm_weekend);
    this->preferences_->end();
    preferences_lock.unlock();
  }
  this->alarm_weekend_ = alarm_weekend;
}

void AlarmControl::setFadeMinutes(uint32_t fade_minutes)
{
  log_d("setFadeMinutes %u", fade_minutes);
  if (this->fade_minutes_ != fade_minutes)
  {
    preferences_lock.lock();
    this->preferences_->begin("alarmctrl");
    this->preferences_->putUInt("fade_min", fade_minutes);
    this->preferences_->end();
    preferences_lock.unlock();
  }
  this->fade_minutes_ = fade_minutes;
}

void AlarmControl::setDutyMax(float duty_max)
{
  log_d("setDutyMax %.2f", duty_max);
  if (this->duty_max_ != duty_max)
  {
    preferences_lock.lock();
    this->preferences_->begin("alarmctrl");
    this->preferences_->putFloat("duty_max", duty_max);
    this->preferences_->end();
    preferences_lock.unlock();
  }
  this->duty_max_ = duty_max;
}

void AlarmControl::setDutyMin(float duty_min)
{
  log_d("setDutyMin %.2f", duty_min);
  if (this->duty_min_ != duty_min)
  {
    preferences_lock.lock();
    this->preferences_->begin("alarmctrl");
    this->preferences_->putFloat("duty_min", duty_min);
    this->preferences_->end();
    preferences_lock.unlock();
  }
  this->duty_min_ = duty_min;
}

void AlarmControl::task_alarm_wrapper(void *arg)
{
  AlarmControl* alarm_ctrl = static_cast<AlarmControl *>(arg);
  alarm_ctrl->task_alarm();
}
void AlarmControl::task_alarm()
{
  struct tm timeinfo;
  float duty_calc;
  
  while (1)
  {
    switch (this->mode_)
    {
    case ALARMMODE_FORCE_ON:
      // we are instructed to stay 100% on
      this->led_control_->setOnMode();
      break;

    case ALARMMODE_ALARM_ON:
      if (!getLocalTime(&timeinfo, 0))
      {
        log_w("Failed to obtain time");
        break;
      }
      //   log_d("%04u-%02u-%02u %02u:%02u:%02u", 
      //         1900 + timeinfo.tm_year, timeinfo.tm_mon, timeinfo.tm_mday, 
      //         timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

      // check if we are outside alarm-fade time
      // TODO
      this->led_control_->setOffMode();

      // if we are inside alarm time: calculate fade duty
      // TODO
      this->led_control_->setDutyCycle(duty_calc);
      break;
    
    case ALARMMODE_ALARM_OFF:
      // alarm is off
      this->led_control_->setOffMode();
      break;
    
    default:
      break;
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
