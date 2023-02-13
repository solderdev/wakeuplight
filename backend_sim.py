import os
import time
from flask import Flask, render_template, request, Response

app = Flask(__name__, static_folder='', template_folder='', root_path=os.path.join(os.getcwd(), 'data'))

_alarm_time = "06:31"
_alarm_weekends = False
_alarm_mode = 0
_fade_min = 44
_duty_max = 99.8
_duty_lights_on = 50.0
_current_duty = 12.34


@app.route('/')
def root():
    return render_template('index.html')


# a generator with yield expression
def gen_currentpercent():
    while True:
        time.sleep(1)
        # DO NOT forget the prefix and suffix
        yield f'\0\0id: {time.time_ns()}\r\nevent: currentpercent\r\ndata: {_current_duty:.3f}\r\n\r\n'


@app.route('/events')
def sse_currentpercent():
    return Response(
        gen_currentpercent(),  # Iterable
        mimetype='text/event-stream'  # mark as a stream response
    )


@app.get('/parameters')
def parameters():
    p_str = f'{_alarm_time} {"true" if _alarm_weekends else "false"} {_fade_min} {_alarm_mode} {_duty_max} {_duty_lights_on} {_current_duty}'
    print(f'params: {p_str.encode()}')
    return p_str


@app.post('/lights_on')
def lights_on():
    global _alarm_mode
    global _current_duty
    _alarm_mode = 0
    _current_duty = _duty_lights_on
    return parameters()


@app.post('/alarm_on')
def alarm_on():
    global _alarm_mode
    global _current_duty
    _alarm_mode = 1
    _current_duty = 1.0
    return parameters()


@app.post('/alarm_off')
def alarm_off():
    global _alarm_mode
    global _current_duty
    _alarm_mode = 2
    _current_duty = 0.0
    return parameters()


@app.post('/set_alarm_time')
def set_alarm_time():
    global _alarm_time
    _alarm_time = request.get_data().decode().strip().rstrip('\x00')
    print(_alarm_time.encode())
    return parameters()


@app.post('/set_alarm_weekend')
def set_alarm_weekend():
    global _alarm_weekends
    _alarm_weekends = request.get_data().decode().strip().rstrip('\x00') == 'true'
    print(f'a WE: {_alarm_weekends}')
    return parameters()


@app.post('/set_fade_minutes')
def set_fade_minutes():
    global _fade_min
    _fade_min = int(request.get_data().decode().strip().rstrip('\x00'))
    print(_fade_min)
    return parameters()


@app.post('/set_duty_max')
def set_duty_max():
    global _duty_max
    _duty_max = float(request.get_data().decode().strip().rstrip('\x00'))
    print(_duty_max)
    return parameters()


@app.post('/set_duty_lights_on')
def set_duty_min():
    global _duty_lights_on
    global _current_duty
    _duty_lights_on = float(request.get_data().decode().strip().rstrip('\x00'))
    _current_duty = _duty_lights_on
    print(_duty_lights_on)
    return parameters()


app.run(debug=True, port=8080, host='0.0.0.0')
