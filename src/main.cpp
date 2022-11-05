#include <Arduino.h>
#include <nvs_flash.h>
#include "WebInterface.hpp"
#include "Pins.hpp"
#include "LEDControl.hpp"
#include "AlarmControl.hpp"

WebInterface *web_interface;
LEDControl *led_control;
AlarmControl *alarm_control;

void setup()
{
  // COMPLETELY ERASE NVS
  // nvs_flash_erase(); // erase the NVS partition and...
  // nvs_flash_init(); // initialize the NVS partition.
  // while(true);

  Serial.begin(115200);
  delay(2000);
  Serial.println("Hi there! Booting now..");
  
  led_control = new LEDControl(Pins::pwm1, MCPWM_UNIT_0);

  alarm_control = new AlarmControl(led_control);

  web_interface = new WebInterface(alarm_control);
}

void loop()
{
  vTaskDelay(pdMS_TO_TICKS(500));

  /*
  Serial.println("now ON");
  laser_control->setOnMode();
  vTaskDelay(pdMS_TO_TICKS(2000));

  Serial.println("now OFF");
  laser_control->setOffMode();
  vTaskDelay(pdMS_TO_TICKS(2000));

  Serial.println("now PWM");
  laser_control->setPwmMode();
  vTaskDelay(pdMS_TO_TICKS(5000));
  */
}