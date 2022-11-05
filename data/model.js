
class Model {
    constructor() 
    {
        this.alarm_time = 0  // TODO
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

    setParameters(alarm_time, fade_minutes, mode, duty_max, duty_min)
    {
        console.log('setParameters: alarm_time=' + alarm_time + 
                    'fade_minutes=' + fade_minutes + 
                    'mode=' + mode + 
                    'duty_max=' + duty_max + 
                    'duty_min=' + duty_min)

        this.alarm_time = alarm_time
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

    setFadeMinutes(fade_minutes)
    {
        this.fade_minutes = fade_minutes
        this.dataChangedCB()
    }

    getAlarmTime()
    {
        return this.alarm_time
    }

    getFadeMinutes()
    {
        return this.fade_minutes
    }

    getModeName()
    {
        if(this.mode == 0) return "ON Mode"
        if(this.mode == 1) return "OFF Mode"
        return "Alarm Mode"
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
  