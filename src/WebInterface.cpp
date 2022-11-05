#include "WebInterface.hpp"
#include "TaskConfig.hpp"
#include <SPIFFS.h>
#include <FS.h>  // needed to fix asyncwebserver compile issues
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <cstring>
#include <string>
#include "Pins.hpp"
#include <sstream>

// Docu:
// https://registry.platformio.org/libraries/me-no-dev/ESP%20Async%20WebServer

// from https://techtutorialsx.com/2017/12/16/esp32-arduino-async-http-server-serving-a-html-page-from-flash-memory/
// HTML compressor: https://htmlcompressor.com/compressor/ or https://www.willpeavy.com/minifier/
// text to C converter: http://tomeko.net/online_tools/cpp_text_escape.php?lang=en

static const char XML_CODE[] = "<?xml version = \"1.0\"?>\n<inputs>\n<rd>\n%L1%\n</rd>\n<rd>\n%L2%\n</rd>\n<pwr>\n%POWERSTATE%\n</pwr>\n</inputs>";

static const char WIFISSID[] = "A1-962961";
static const char WIFIPW[] = "1x799rkqri6u";

static const char HOSTNAME[] = "wakeuplight";

static WebInterface *instance = nullptr;

WebInterface::WebInterface(AlarmControl *alarm_control) :
  server_(80),
  task_handle_http_(nullptr),
  task_handle_ota_(nullptr),
  alarm_control_(alarm_control)
{
  if (instance)
  {
    Serial.println("ERROR: more than one WebInterface generated");
    ESP.restart();
    return;
  }

  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  if (xTaskCreate(&WebInterface::task_http_wrapper, "task_http", TaskConfig::WiFi_http_stacksize, this, TaskConfig::WiFi_http_priority, &task_handle_http_) != pdPASS)
    Serial.println("WebInterface ERROR init failed");

  instance = this;
}

AlarmControl* WebInterface::getAlarmControl()
{
  return this->alarm_control_;
}

void WebInterface::wifiReconnect()
{
  WiFi.disconnect();
  vTaskDelay(pdMS_TO_TICKS(1000));
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(HOSTNAME);
  vTaskDelay(pdMS_TO_TICKS(1000));
  WiFi.begin(WIFISSID, WIFIPW);
  WiFi.setSleep(false);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
}

void WebInterface::wifiCheckConnectionOrReconnect()
{
  if (WiFi.isConnected())
    return;
  
  Serial.println("WIFI down .. attempting connect!");
  wifiReconnect();
  
  Serial.println("WIFI reconnecting .. waiting for network");
  uint32_t trycount = 0;
  while (!WiFi.isConnected())
  {
    trycount++;
    if (trycount > 150)  // 15s
    {
      trycount = 0;
      Serial.println("WIFI still down .. attempting reconnect!");
      wifiReconnect();
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
  Serial.println("WIFI up again!");
}

// replaces placeholder with values in static html
static String processor_static(const String& var)
{
  Serial.println("processor_static var: " + var);
  
  return String();
}

// replaces placeholder with values in xml file
static String processor_xml(const String& var)
{
  Serial.println("processor_xml var: " + var);

  return String();
}

void WebInterface::task_http_wrapper(void *arg)
{
  WebInterface* web_interface = static_cast<WebInterface *>(arg);
  web_interface->task_http();
}
void WebInterface::task_http()
{
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(WIFISSID);

  wifiCheckConnectionOrReconnect();

  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // be reachable under hostname.local
  if (!MDNS.begin(HOSTNAME))
    Serial.println("Error setting up MDNS responder!");
  else
    Serial.println("mDNS responder started");
  
  // for (uint8_t i = 0; i < 3; i++)
  //   Serial.println("mDNS host " + String(i) + ": " + MDNS.hostname(i));
  
  server_.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Received favicon get");
    request->send(SPIFFS, "/favicon.png", "image/png");
  });

  // FIXME: this crashes?? SPIFFS seems to conflict with hardware timers!
  server_.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  // server_.serveStatic("/view.js", SPIFFS, "/view.js");

  // this alternative just crashes less/same :(
  // route for root / web page
  // server_.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   request->send(SPIFFS, "/index.html", String(), false, processor_static);
  // });
  // it works with the html in flash
  // server_.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   request->send(SPIFFS, "/index.html");
  // });

  server_.on("/view.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/view.js", "text/javascript");
  });

  server_.on("/model.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/model.js", "text/javascript");
  });

  server_.on("/controller.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/controller.js", "text/javascript");
  });

  server_.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });

  // route to update values
  server_.on("/update_readings", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/xml", XML_CODE, processor_xml);
  });

  // route to on_mode
  server_.on("/on_mode", HTTP_POST, [this](AsyncWebServerRequest *request) {
    request->send(200);
    Serial.println("Received on_mode");
    this->alarm_control_->setOnMode();
  });

  // route to off_mode
  server_.on("/off_mode", HTTP_POST, [this](AsyncWebServerRequest *request) {
    request->send(200);
    Serial.println("Received off_mode");
    this->alarm_control_->setOffMode();
  });

  // route to pwm_mode
  server_.on("/alarm_mode", HTTP_POST, [this](AsyncWebServerRequest *request) {
    request->send(200);
    Serial.println("Received alarm_mode");
    this->alarm_control_->setAlarmMode();
  });

  // route to set alarm time
  server_.on("/set_alarm_time", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, 
    [this](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {

      String alarm_time = String(data, len);
      this->alarm_control_->setAlarmTime(alarm_time);
 
      request->send_P(200, "text/plain", String(this->alarm_control_->getAlarmTime()).c_str());
  });

  // route to set duty cycle
  server_.on("/set_fade_minutes", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, 
    [this](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {

      std::stringstream ss;
      ss << (char*)data;
      float fade_minutes;
      ss >> fade_minutes;

      this->alarm_control_->setFadeMinutes(fade_minutes);
 
      request->send_P(200, "text/plain", std::to_string(this->alarm_control_->getFadeMinutes()).c_str());
  });

  server_.on("/set_duty_max", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, 
    [this](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {

      std::stringstream ss;
      ss << (char*)data;
      float duty_max;
      ss >> duty_max;

      this->alarm_control_->setDutyMax(duty_max);
 
      request->send_P(200, "text/plain", std::to_string(duty_max).c_str());
  });

  server_.on("/set_duty_min", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, 
    [this](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {

      std::stringstream ss;
      ss << (char*)data;
      float duty_min;
      ss >> duty_min;

      this->alarm_control_->setDutyMin(duty_min);
 
      request->send_P(200, "text/plain", std::to_string(duty_min).c_str());
  });

  // route to load style.css file
  server_.on("/parameters", HTTP_GET, [this](AsyncWebServerRequest *request) {
    
    String params = this->alarm_control_->getAlarmTime() + " " +
      String(this->alarm_control_->getFadeMinutes()) + " " + 
      String(this->alarm_control_->getMode()) + " " + 
      String(this->alarm_control_->getDutyMax()) + " " +
      String(this->alarm_control_->getDutyMin());

    request->send_P(200, "text/plain", params.c_str());
    Serial.println("send parameters: " + params);
  });

  // route to reset
  server_.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200);
    ESP.restart();
  });

  // respond to GET requests on URL /heap
  server_.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Free heap: " + String(ESP.getFreeHeap()));
  });

  // start webserver
  server_.begin();
  MDNS.addService("http", "tcp", 80);

  // start OTA task when WIFI is available
  if (task_handle_ota_ == nullptr)
    xTaskCreate(task_ota_wrapper, "task_ota", TaskConfig::WiFi_ota_stacksize, this, TaskConfig::WiFi_ota_priority, &task_handle_ota_);
  
  while (1)
  {
    wifiCheckConnectionOrReconnect();

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}


void WebInterface::task_ota_wrapper(void *arg)
{
  static_cast<WebInterface *>(arg)->task_ota();
}
void WebInterface::task_ota()
{
  // over-the-air update:
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // TODO: stop everything (esp. webserver)

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      // SPIFFS.end();
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd of OTA ..");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.begin();
  
  while(1)
  {
    vTaskDelay(pdMS_TO_TICKS(2));
    
    if (WiFi.isConnected())
      ArduinoOTA.handle();
    else
      vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
