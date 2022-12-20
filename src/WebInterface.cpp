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

#include "Private.hpp"

// Docu:
// https://registry.platformio.org/libraries/me-no-dev/ESP%20Async%20WebServer

// from https://techtutorialsx.com/2017/12/16/esp32-arduino-async-http-server-serving-a-html-page-from-flash-memory/
// HTML compressor: https://htmlcompressor.com/compressor/ or https://www.willpeavy.com/minifier/
// text to C converter: http://tomeko.net/online_tools/cpp_text_escape.php?lang=en

static const char XML_CODE[] = "<?xml version = \"1.0\"?>\n<inputs>\n<rd>\n%L1%\n</rd>\n<rd>\n%L2%\n</rd>\n<pwr>\n%POWERSTATE%\n</pwr>\n</inputs>";

static const char HOSTNAME[] = "wakeuplight";

static WebInterface *instance = nullptr;

WebInterface::WebInterface(AlarmControl *alarm_control) :
  server_(80),
  task_handle_http_(nullptr),
  task_handle_ota_(nullptr),
  alarm_control_(alarm_control),
  wifi_select_idx_(0)
{
  if (instance)
  {
    log_e("ERROR: more than one WebInterface generated");
    ESP.restart();
    return;
  }

  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    log_e("An Error has occurred while mounting SPIFFS");
    return;
  }

  // disable Bluetooth
  btStop();

  if (xTaskCreate(&WebInterface::task_http_wrapper, "task_http", TaskConfig::WiFi_http_stacksize, this, TaskConfig::WiFi_http_priority, &task_handle_http_) != pdPASS)
    log_e("WebInterface ERROR init failed, task_http");

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

  log_d("trying wifi %u with: %s / %s", this->wifi_select_idx_, WIFICREDS[this->wifi_select_idx_].ssid, WIFICREDS[this->wifi_select_idx_].pw);
  WiFi.begin(WIFICREDS[this->wifi_select_idx_].ssid, WIFICREDS[this->wifi_select_idx_].pw);
  this->wifi_select_idx_ += 1;
  this->wifi_select_idx_ %= sizeof(WIFICREDS) / sizeof(WIFICREDS[0]);
  
  WiFi.setSleep(true);
  WiFi.setTxPower(WIFI_POWER_MINUS_1dBm);  // WIFI_POWER_19_5dBm
}

void WebInterface::wifiCheckConnectionOrReconnect()
{
  if (WiFi.isConnected())
    return;
  
  log_w("WIFI down .. attempting connect!");
  wifiReconnect();
  
  log_i("WIFI reconnecting .. waiting for network");
  uint32_t trycount = 0;
  while (!WiFi.isConnected())
  {
    trycount++;
    if (trycount > 100)  // 10s
    {
      trycount = 0;
      log_d("WIFI still down .. attempting reconnect!");
      wifiReconnect();
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
  log_i("WIFI up again!");
}

void WebInterface::task_http_wrapper(void *arg)
{
  WebInterface* web_interface = static_cast<WebInterface *>(arg);
  web_interface->task_http();
}
void WebInterface::task_http()
{
  wifiCheckConnectionOrReconnect();

  IPAddress ip = WiFi.localIP();
  log_i("WiFi connected - IP address: %u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);

  // be reachable under hostname.local
  if (!MDNS.begin(HOSTNAME))
    log_e("Error setting up MDNS responder!");
  else
    log_i("mDNS responder started");
  
  // https://realfavicongenerator.net
  server_.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    log_d("Received favicon get");
    request->send(SPIFFS, "/favicon.ico", "image/svg+xml");
  });
  server_.on("/apple-touch-icon.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/apple-touch-icon.png", "image/png");
  });
  server_.on("/apple-touch-icon-60x60.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/apple-touch-icon-60x60.png", "image/png");
  });
  server_.on("/apple-touch-icon-76x76.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/apple-touch-icon-76x76.png", "image/png");
  });
  server_.on("/apple-touch-icon-120x120.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/apple-touch-icon-120x120.png", "image/png");
  });
  server_.on("/apple-touch-icon-152x152.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/apple-touch-icon-152x152.png", "image/png");
  });
  server_.on("/apple-touch-icon-180x180.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/apple-touch-icon-180x180.png", "image/png");
  });
  server_.on("/favicon-32x32.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/favicon-32x32.png", "image/png");
  });
  server_.on("/favicon-16x16.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/favicon-16x16.png", "image/png");
  });
  server_.on("/site.webmanifest", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/site.webmanifest", "text/plain");
  });
  server_.on("/safari-pinned-tab.svg", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/safari-pinned-tab.svg", "image/svg+xml");
  });
  server_.on("/android-chrome-192x192.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/android-chrome-192x192.png", "image/png");
  });
  server_.on("/android-chrome-512x512.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/android-chrome-512x512.png", "image/png");
  });
  server_.on("/mstile-150x150.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/mstile-150x150.png", "image/png");
  });
  server_.on("/browserconfig.xml", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/browserconfig.xml", "text/xml");
  });

  // FIXME: this crashes?? SPIFFS seems to conflict with hardware timers!
  server_.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

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

  // route to on_mode
  server_.on("/on_mode", HTTP_POST, [this](AsyncWebServerRequest *request) {
    request->send(200);
    log_d("Received on_mode");
    this->alarm_control_->setOnMode();
  });

  // route to alarm_off
  server_.on("/alarm_off", HTTP_POST, [this](AsyncWebServerRequest *request) {
    log_d("Received alarm_off");
    request->send(200);
    this->alarm_control_->setAlarmOFF();
    log_d("Received alarm_off done");
  });

  // route to pwm_mode
  server_.on("/alarm_on", HTTP_POST, [this](AsyncWebServerRequest *request) {
    log_d("Received alarm_on");
    request->send(200);
    this->alarm_control_->setAlarmON();
    log_d("Received alarm_on done");
  });

  // route to set alarm time
  server_.on("/set_alarm_time", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, 
    [this](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
      String alarm_time = String(data, len);
      this->alarm_control_->setAlarmTime(alarm_time);
 
      request->send(200, "text/plain", String(this->alarm_control_->getAlarmTime()).c_str());
  });

  // route to set alarm weekend
  server_.on("/set_alarm_weekend", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, 
    [this](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
      bool alarm_weekend = strncmp((const char*)(data), "true", len) == 0;
      this->alarm_control_->setAlarmWeekend(alarm_weekend);
 
      String al_we = (this->alarm_control_->getAlarmWeekend())?String("true"):String("false");
      request->send(200, "text/plain", al_we.c_str());
  });

  // route to set duty cycle
  server_.on("/set_fade_minutes", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, 
    [this](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
      std::stringstream ss;
      ss << (char*)data;
      float fade_minutes;
      ss >> fade_minutes;

      this->alarm_control_->setFadeMinutes(fade_minutes);
 
      request->send(200, "text/plain", std::to_string(this->alarm_control_->getFadeMinutes()).c_str());
  });

  server_.on("/set_duty_max", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, 
    [this](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
      std::stringstream ss;
      ss << (char*)data;
      float duty_max;
      ss >> duty_max;

      this->alarm_control_->setDutyMax(duty_max);
 
      request->send(200, "text/plain", std::to_string(duty_max).c_str());
  });

  server_.on("/set_duty_min", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, 
    [this](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
      std::stringstream ss;
      ss << (char*)data;
      float duty_min;
      ss >> duty_min;

      this->alarm_control_->setDutyMin(duty_min);
 
      request->send(200, "text/plain", std::to_string(duty_min).c_str());
  });

  // route to load style.css file
  server_.on("/parameters", HTTP_GET, [this](AsyncWebServerRequest *request) {
    log_d("Received parameters req");
    String al_we = (this->alarm_control_->getAlarmWeekend())?String("true"):String("false");

    String params = 
      this->alarm_control_->getAlarmTime() + " " +
      al_we + " " +  // TODO check what is actually sent here vs. controller.js:48
      String(this->alarm_control_->getFadeMinutes()) + " " + 
      String(this->alarm_control_->getMode()) + " " + 
      String(this->alarm_control_->getDutyMax(), 3) + " " +
      String(this->alarm_control_->getDutyMin(), 3);

    request->send(200, "text/plain", params.c_str());
    log_d("send parameters: %s", params.c_str());
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

      // stop everything (esp. webserver)
      instance->server_.end();
      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      SPIFFS.end();

      log_i("Start updating %s", type);
    })
    .onEnd([]() {
      log_i("\nEnd of OTA ..");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      log_d("Progress: %u%%", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      log_e("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) log_e("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) log_e("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) log_e("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) log_e("Receive Failed");
      else if (error == OTA_END_ERROR) log_e("End Failed");
    });

  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.begin();
  
  while(1)
  {
    vTaskDelay(pdMS_TO_TICKS(5));
    
    if (WiFi.isConnected())
      ArduinoOTA.handle();
    else
      vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
