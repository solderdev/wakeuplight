
class View 
{
  constructor() 
  {
    document.getElementById('on_mode_button_id').addEventListener(
        "click", event => this.onButtonClick(event, "on_mode"), true);
    
    document.getElementById('alarm_off_button_id').addEventListener(
        "click", event => this.onButtonClick(event, "alarm_off"), true);

    document.getElementById('alarm_on_button_id').addEventListener(
        "click", event => this.onButtonClick(event, "alarm_on"), true);

    document.getElementById('alarm_time_form_id').addEventListener(
        "change", event => this.onSubmitAlarmTime(event), true);
    document.getElementById('alarm_time_form_id').addEventListener(
        "submit", event => this.onSubmitPreventDefault(event), true);
 
    document.getElementById('alarm_weekend_form_id').addEventListener(
        "change", event => this.onSubmitAlarmWeekend(event), true);
    document.getElementById('alarm_weekend_form_id').addEventListener(
        "submit", event => this.onSubmitPreventDefault(event), true);

    document.getElementById('fade_minutes_form_id').addEventListener(
        "change", event => this.onSubmitFadeMinutes(event), true);
    document.getElementById('fade_minutes_form_id').addEventListener(
        "submit", event => this.onSubmitPreventDefault(event), true);

    document.getElementById('duty_max_form_id').addEventListener(
        "change", event => this.onSubmitDutyMax(event), true);
    document.getElementById('duty_max_form_id').addEventListener(
        "submit", event => this.onSubmitPreventDefault(event), true);

    document.getElementById('duty_min_form_id').addEventListener(
        "change", event => this.onSubmitDutyMin(event), true);
    document.getElementById('duty_min_form_id').addEventListener(
        "submit", event => this.onSubmitPreventDefault(event), true);
  }

  setModel(model)
  {
    //store model and set model changed callbacks
    this.model = model
    this.model.bindDataChangedCB(() => this.onModelDataChanged())

    //log host and port
    console.log("HOST: " + window.location.hostname)
    console.log("PORT: " + window.location.port)

    this.model.setHostIP(window.location.hostname)
  }

  bindOnButtonClickCB(callback) { this.onButtonClickCB = callback }
  bindAlarmTimeSubmitCB(callback) { this.alarmTimeSubmitCB = callback }
  bindAlarmWeekendSubmitCB(callback) { this.alarmWeekendSubmitCB = callback }
  bindFadeMinutesSubmitCB(callback) { this.fadeMinutesSubmitCB = callback }
  bindDutyMaxSubmitCB(callback) { this.dutyMaxSubmitCB = callback }
  bindDutyMinSubmitCB(callback) { this.dutyMinSubmitCB = callback }

  onModelDataChanged()
  {
    console.log("View:onModelDataChanged()")

    document.getElementById('alarm_time_input_id').value = this.model.getAlarmTime()
    document.getElementById('alarm_weekend_input_id').checked = this.model.getAlarmWeekend()
    document.getElementById('fade_minutes_input_id').value = this.model.getFadeMinutes()
    document.getElementById('mode_id').innerHTML = this.model.getModeName()
    document.getElementById('duty_max_input_id').value = this.model.getDutyMax()
    document.getElementById('duty_min_input_id').value = this.model.getDutyMin()
  }

  onButtonClick(callback, name) {
    console.log("View:onOnModeButtonClick()")
    this.onButtonClickCB(name)
  }

  onSubmitPreventDefault(event)
  {
    console.log("View:onSubmitPreventDefault")
    event.preventDefault()
  }

  onSubmitAlarmTime(event)
  {
    console.log("View:onSubmitAlarmTime()")
    event.preventDefault()

    let alarm_time = document.getElementById('alarm_time_input_id').value  // TODO parse time

    console.log("Alarm Time: " + alarm_time.toString())

    this.alarmTimeSubmitCB(alarm_time)
  }

  onSubmitAlarmWeekend(event)
  {
    console.log("View:onSubmitAlarmWeekend()")
    event.preventDefault()

    let alarm_weekend = document.getElementById('alarm_weekend_input_id').checked

    console.log("Alarm Weekend: " + alarm_weekend.toString())

    this.alarmWeekendSubmitCB(alarm_weekend)
  }

  onSubmitFadeMinutes(event)
  {
    console.log("View:onSubmitFadeMinutes()")
    event.preventDefault()

    let fade_m = Math.max(1, parseInt(document.getElementById('fade_minutes_input_id').value))
    fade_m = Math.min(120, fade_m)

    console.log("Fade Minutes: " + fade_m.toString())

    this.fadeMinutesSubmitCB(fade_m)
  }

  onSubmitDutyMax(event)
  {
    console.log("View:onSubmitDutyMax()")
    event.preventDefault()

    let duty_max = Math.max(0.1, parseFloat(document.getElementById('duty_max_input_id').value))
    duty_max = Math.min(99.0, duty_max)

    console.log("Max Duty: " + duty_max.toString())

    this.dutyMaxSubmitCB(duty_max)
  }

  onSubmitDutyMin(event)
  {
    console.log("View:onSubmitDutyMin()")
    event.preventDefault()

    let duty_min = Math.max(0.1, parseFloat(document.getElementById('duty_min_input_id').value))
    duty_min = Math.min(99.0, duty_min)

    console.log("Min Duty: " + duty_min.toString())

    this.dutyMinSubmitCB(duty_min)
  }
}
