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
  struct tm timeinfo_alarm;
  float duty_calc;
  int minutes_diff;
  
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

      // if we are on a weekend and option is off --> disable
      if ((timeinfo.tm_wday == 6 || timeinfo.tm_wday == 0) && this->alarm_weekend_ == false)
      {
        this->led_control_->setOffMode();
        break;
      }

      // copy "now" for calculations
      memcpy(&timeinfo_alarm, &timeinfo, sizeof(timeinfo));
      timeinfo_alarm.tm_hour = String(this->alarm_time_).substring(0, 2).toInt();
      timeinfo_alarm.tm_min = String(this->alarm_time_).substring(3, 5).toInt();
      timeinfo_alarm.tm_sec = 0;

      // calculate distance to alarm time
      minutes_diff = difftime(mktime(&timeinfo), mktime(&timeinfo_alarm)) / 60;

      // uint8_t start_h = String(this->alarm_time_).substring(0, 2).toInt();
      // uint8_t start_m = String(this->alarm_time_).substring(3, 5).toInt();
      // uint8_t end_h = (uint8_t)(((uint32_t)(start_h) * 60 + start_m + this->fade_minutes_) / 60) % 24;
      // uint8_t end_m = (start_m + this->fade_minutes_) % 60;

      if (minutes_diff >= 0 && minutes_diff < this->fade_minutes_)
      {
        // if we are inside alarm time: calculate current fade duty cycle
        duty_calc = minutes_diff * 100 / this->fade_minutes_;
        if (duty_calc >= 100)
          this->led_control_->setOnMode();
        else if (duty_calc > 0.0)
          this->led_control_->setDutyCycle(duty_calc);
        else
          this->led_control_->setOffMode();
        break;
      }
      else
      {
        // we are outside of fading time (< 0 is too early)
        this->led_control_->setOffMode();
        break;
      }
    
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
