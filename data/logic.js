class Logic {
    constructor() {
        console.log('Creating Logic')

        this.alarm_time = "11:11"
        this.alarm_weekend = false
        this.fade_minutes = 20
        this.mode = 2
        this.duty_max = 99.0
        this.duty_min = 1.0
        // noinspection HttpUrlsUsage
        this.host_uri = "http://" + window.location.hostname + ":" + window.location.port + "/"

        console.log("URI: " + this.host_uri)

        document.getElementById('lights_on_button_id').addEventListener(
            "click", event => this.onButtonClick(event, "lights_on"), true)

        document.getElementById('alarm_off_button_id').addEventListener(
            "click", event => this.onButtonClick(event, "alarm_off"), true)

        document.getElementById('alarm_on_button_id').addEventListener(
            "click", event => this.onButtonClick(event, "alarm_on"), true)

        document.getElementById('alarm_time_form_id').addEventListener(
            "focusout", event => this.onSubmitAlarmTime(event), true)
        document.getElementById('alarm_time_form_id').addEventListener(
            "submit", event => this.onSubmitPreventDefault(event), true)

        document.getElementById('alarm_weekend_form_id').addEventListener(
            "change", event => this.onSubmitAlarmWeekend(event), true)
        document.getElementById('alarm_weekend_form_id').addEventListener(
            "submit", event => this.onSubmitPreventDefault(event), true)

        document.getElementById('fade_minutes_form_id').addEventListener(
            "change", event => this.onSubmitFadeMinutes(event), true)
        document.getElementById('fade_minutes_form_id').addEventListener(
            "submit", event => this.onSubmitPreventDefault(event), true)

        document.getElementById('duty_max_form_id').addEventListener(
            "change", event => this.onSubmitDutyMax(event), true)
        document.getElementById('duty_max_form_id').addEventListener(
            "submit", event => this.onSubmitPreventDefault(event), true)

        document.getElementById('duty_min_form_id').addEventListener(
            "change", event => this.onSubmitDutyMin(event), true)
        document.getElementById('duty_min_form_id').addEventListener(
            "submit", event => this.onSubmitPreventDefault(event), true)

        this.fetchParameters()
    }

    modeToString(mode) {
        // see AlarmMode_t
        if (mode === 0) return "Lights ON"
        if (mode === 1) return "Alarm ON"
        if (mode === 2) return "Alarm OFF"
        return "undefined"
    }

    fetchParameters() {
        console.log('fetchParameters()')

        fetch(this.host_uri + "parameters", {
            method: 'GET',
            headers: {'Content-Type': 'text/plain'},
            body: null,
        })
            .then(response => response.text())
            .then(data => {
                console.log('got Parameters: ' + data)
                let params = data.split(" ")

                this.alarm_time = params[0]
                this.alarm_weekend = JSON.parse(params[1]) // true / false
                this.fade_minutes = parseInt(params[2])
                this.mode = parseInt(params[3])
                this.duty_max = parseFloat(params[4])
                this.duty_min = parseFloat(params[5])

                console.log('update: alarm_time=' + this.alarm_time +
                    ' alarm_weekend=' + this.alarm_weekend +
                    ' fade_minutes=' + this.fade_minutes +
                    ' mode=' + this.mode +
                    ' duty_max=' + this.duty_max +
                    ' duty_min=' + this.duty_min)

                document.getElementById('alarm_time_input_id').value = this.alarm_time
                document.getElementById('alarm_weekend_input_id').checked = this.alarm_weekend
                document.getElementById('fade_minutes_input_id').value = this.fade_minutes
                document.getElementById('mode_id').innerHTML = this.modeToString(this.mode)
                document.getElementById('duty_max_input_id').value = this.duty_max
                document.getElementById('duty_min_input_id').value = this.duty_min
            })
    }

    onSubmitPreventDefault(event) {
        console.log("onSubmitPreventDefault()")
        event.preventDefault()
    }

    post_and_fetch(endpoint, body_data) {
        fetch(this.host_uri + endpoint, {
            method: 'POST',
            headers: {'Content-Type': 'text/plain'},
            body: body_data,
        })
            .then(response => {
                this.fetchParameters()
            })
    }

    onButtonClick(callback, name) {
        console.log("onButtonClick()")

        let endpoint = "undefined"
        if (name === "alarm_off") endpoint = "alarm_off"
        else if (name === "alarm_on") endpoint = "alarm_on"
        else if (name === "lights_on") endpoint = "lights_on"
        else {
            console.log("ERROR: undefined button callback name: " + name)
            return
        }
        this.post_and_fetch(endpoint, null)
    }

    onSubmitAlarmTime(event) {
        console.log("onSubmitAlarmTime()")
        event.preventDefault()
        let alarm_time = document.getElementById('alarm_time_input_id').value
        console.log("Alarm Time: " + alarm_time.toString())
        this.post_and_fetch("set_alarm_time", alarm_time.toString() + '\0')
    }

    onSubmitAlarmWeekend(event) {
        console.log("onSubmitAlarmWeekend()")
        event.preventDefault()
        let alarm_weekend = document.getElementById('alarm_weekend_input_id').checked
        console.log("Alarm Weekend: " + alarm_weekend.toString())
        this.post_and_fetch("set_alarm_weekend", alarm_weekend.toString() + '\0')
    }

    onSubmitFadeMinutes(event) {
        console.log("onSubmitFadeMinutes()")
        event.preventDefault()
        let fade_m = Math.max(1, parseInt(document.getElementById('fade_minutes_input_id').value))
        fade_m = Math.min(120, fade_m)
        console.log("Fade Minutes: " + fade_m.toString())
        this.post_and_fetch("set_fade_minutes", fade_m.toString() + '\0')
    }

    onSubmitDutyMax(event) {
        console.log("onSubmitDutyMax()")
        event.preventDefault()
        let duty_max = Math.max(0.001, parseFloat(document.getElementById('duty_max_input_id').value))
        duty_max = Math.min(99.999, duty_max)
        console.log("Max Duty: " + duty_max.toString())
        this.post_and_fetch("set_duty_max", duty_max.toString() + '\0')
    }

    onSubmitDutyMin(event) {
        console.log("onSubmitDutyMin()")
        event.preventDefault()
        let duty_min = Math.max(0.001, parseFloat(document.getElementById('duty_min_input_id').value))
        duty_min = Math.min(99.999, duty_min)
        console.log("Min Duty: " + duty_min.toString())
        this.post_and_fetch("set_duty_min", duty_min.toString() + '\0')
    }
}

const logic = new Logic()
