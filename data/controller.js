
class Controller {
    constructor(model, view)
    {
      console.log('Creating Controller.')

      //store controller and viewe references
      this.model = model
      this.view = view

      //set model for view
      this.view.setModel(this.model)

      //bind view callbacks
      this.view.bindOnButtonClickCB((name) => this.onOnMode(name))
      this.view.bindAlarmTimeSubmitCB((alarm_time) => this.onAlarmTime(alarm_time))
      this.view.bindAlarmWeekendSubmitCB((alarm_weekend) => this.onAlarmWeekend(alarm_weekend))
      this.view.bindFadeMinutesSubmitCB((fade_minutes) => this.onFadeMinutes(fade_minutes))
      this.view.bindDutyMaxSubmitCB((duty_max) => this.onDutyMax(duty_max))
      this.view.bindDutyMinSubmitCB((duty_min) => this.onDutyMin(duty_min))

      this.fetchParameters()
    }

    intervalUpdate()
    {
        console.log("intervalUpdate")

        // TODO: fetch esp-32 time and display!
        this.fetchParameters()

        setTimeout(() => {
            this.intervalUpdate()
        }, 5000);
    }

    fetchParameters()
    {
        console.log('fetchParameters')

        fetch(this.model.getHostURLHTTP() + "parameters", {
            method: 'GET',
            headers: {'Content-Type': 'text/plain'},
            body: null,
        })
        .then(response => response.text())
        .then(data => {
            console.log('got Parameters: ' + data)
            let params = data.split(" ")
            this.model.setParameters(
                params[0], 
                JSON.parse(params[1]), // true / false
                parseInt(params[2]), 
                parseInt(params[3]), 
                parseFloat(params[4]), 
                parseFloat(params[5]))
        })
    }

    onAlarmTime(alarm_time)
    {
        console.log("Controller:onAlarmTime(" + alarm_time.toString() + ")")

        fetch(this.model.getHostURLHTTP() + "set_alarm_time", {
            method: 'POST',
            headers: {'Content-Type': 'text/plain'},
            body: alarm_time.toString() + '\0',
        })
        .then(response => {
            this.fetchParameters()
        })
    }

    onAlarmWeekend(alarm_weekend)
    {
        console.log("Controller:onAlarmWeekend(" + alarm_weekend.toString() + ")")

        fetch(this.model.getHostURLHTTP() + "set_alarm_weekend", {
            method: 'POST',
            headers: {'Content-Type': 'text/plain'},
            body: alarm_weekend.toString() + '\0',
        })
        .then(response => response.text())
        .then(data => {
            this.fetchParameters()
        })
    }

    onFadeMinutes(fade_minutes)
    {
        console.log("Controller:onFadeMinutes(" + fade_minutes.toString() + ")")

        fetch(this.model.getHostURLHTTP() + "set_fade_minutes", {
            method: 'POST',
            headers: {'Content-Type': 'text/plain'},
            body: fade_minutes.toString() + '\0',
        })
        .then(response => response.text())
        .then(data => {
            this.fetchParameters()
        })
    }

    fetch_post(endpoint, body_data)
    {
        fetch(this.model.getHostURLHTTP() + endpoint, {
            method: 'POST',
            headers: {'Content-Type': 'text/plain'},
            body: body_data,
        })
        .then(response => response.text())
        .then(data => {
            this.fetchParameters()
        })
    }

    onDutyMax(duty_max)
    {
        console.log("Controller:onDutyMax(" + duty_max.toString() + ")")
        this.fetch_post("set_duty_max", duty_max.toString() + '\0')
    }

    onDutyMin(duty_min)
    {
        console.log("Controller:onDutyMin(" + duty_min.toString() + ")")
        this.fetch_post("set_duty_min", duty_min.toString() + '\0')
    }

    onOnMode(name)
    {
        console.log("Controller:onOnMode(" + name + ")")

        let endpoint = "on_mode"

        if(name == "alarm_off") endpoint = "alarm_off"
        if(name == "alarm_on") endpoint = "alarm_on"

        fetch(this.model.getHostURLHTTP() + endpoint, {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: null,
        })
        .then(response => response.text())
        .then(data => {
            console.log("onOnMode got " + data.toString())
            this.fetchParameters()
        })
    }
}

var app = new Controller(new Model(), new View());

// document.addEventListener('DOMContentLoaded', function() {
//         app.intervalUpdate();
//     }, false);
