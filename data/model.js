
class Model {
    constructor() 
    {
        this.alarm_time = "11:11"
        this.alarm_weekend = false
        this.fade_minutes = 20
        this.mode = 2
        this.duty_max = 99.0
        this.duty_min = 1.0
        this.host_ip = ""
    }

    bindDataChangedCB(callback)
    {
        this.dataChangedCB = callback
    }

    setHostIP(host_ip)
    {
        this.host_ip = host_ip
    }

    getHostURLHTTP()
    {
        return "http://" + this.host_ip + "/"
    }

    setParameters(alarm_time, alarm_weekend, fade_minutes, mode, duty_max, duty_min)
    {
        console.log('setParameters: alarm_time=' + alarm_time + 
                    ' alarm_weekend=' + alarm_weekend + 
                    ' fade_minutes=' + fade_minutes + 
                    ' mode=' + mode + 
                    ' duty_max=' + duty_max + 
                    ' duty_min=' + duty_min)

        this.alarm_time = alarm_time
        this.alarm_weekend = alarm_weekend
        this.fade_minutes = fade_minutes
        this.mode = mode
        this.duty_max = duty_max
        this.duty_min = duty_min
        this.dataChangedCB()
    }

    setAlarmTime(alarm_time)
    {
        this.alarm_time = alarm_time
        this.dataChangedCB()
    }

    setAlarmWeekend(alarm_weekend)
    {
        this.alarm_weekend = alarm_weekend
        this.dataChangedCB()
    }

    setFadeMinutes(fade_minutes)
    {
        this.fade_minutes = fade_minutes
        this.dataChangedCB()
    }

    getAlarmTime()
    {
        return this.alarm_time
    }

    getAlarmWeekend()
    {
        return this.alarm_weekend
    }

    getFadeMinutes()
    {
        return this.fade_minutes
    }

    getModeName()
    {
        // see AlarmMode_t
        if(this.mode == 0) return "Forced ON"
        if(this.mode == 1) return "Alarm ON"
        if(this.mode == 2) return "Alarm OFF "
        return "undefined"
    }

    getDutyMax()
    {
        return this.duty_max
    }

    getDutyMin()
    {
        return this.duty_min
    }
}
  