//const char CONTROLLER_JS_CODE[] = R"======(

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
      this.view.bindFadeMinutesSubmitCB((fade_minutes) => this.onFadeMinutes(fade_minutes))
      this.view.bindDutyMaxSubmitCB((duty_max) => this.onDutyMax(duty_max))
      this.view.bindDutyMinSubmitCB((duty_min) => this.onDutyMin(duty_min))

      this.fetchParameters()
    }

    fetchParameters()
    {
        fetch(this.model.getHostURLHTTP() + "parameters", {
            method: 'GET',
            headers: {'Content-Type': 'text/plain'},
            body: null,
        })
        .then(response => response.text())
        .then(data => {
            let params = data.split(" ")
            this.model.setParameters(
                parseInt(params[0]), 
                parseFloat(params[1]), 
                parseInt(params[2]), 
                parseFloat(params[3]), 
                parseFloat(params[4]))
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

        if(name == "off_mode") endpoint = "off_mode"
        if(name == "alarm_mode") endpoint = "alarm_mode"

        fetch(this.model.getHostURLHTTP() + endpoint, {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: null,
        })
        .then(response => response.text())
        .then(data => {
            console.log(data.toString())
            this.fetchParameters()
        })
    }
  }

  var app = new Controller(new Model(), new View());

  //)======";
