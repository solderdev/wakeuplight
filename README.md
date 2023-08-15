First version of my wakeuplight.

Uses an ESP32 with N-channel MOSFET soldered to a pin to PWM-control a warm-white 5V-LED-strip.

# Features

* webinterface for easy control and live-feedback
* manually activate light and regulate intensity
* set fade start time and duration
* over-complicated fade algorithm for added smoothness
* store multiple WiFi credentials and connect / reconnect
* NTP time sync
* sunrise / sunset calculation and default intesity regulation
* store settings in NVM
* logging to serial out


# Build / run

* Build using PlatformIO
* F1 -> Tasks: Run task -> PlatformIO: Build filesystem image (esp32dev)
* F1 -> Tasks: Run task -> PlatformIO: Upload filesystem image (esp32dev)
* run program