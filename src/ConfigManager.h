#ifndef __CONFIGMANAGER_H__
#define __CONFIGMANAGER_H__

#include <DNSServer.h>
#include <EEPROM.h>
#include <FS.h>

#if defined(ARDUINO_ARCH_ESP8266)  // ESP8266
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)  // ESP32
#include <SPIFFS.h>
#include <WebServer.h>
#include <WiFi.h>
#endif

#include <functional>
#include <list>

#include "ArduinoJson.h"

#if defined(ARDUINO_ARCH_ESP8266)  // ESP8266
#define WIFI_OPEN ENC_TYPE_NONE
using WebServer = ESP8266WebServer;
#elif defined(ARDUINO_ARCH_ESP32)  // ESP32
#define WIFI_OPEN WIFI_AUTH_OPEN
#endif

#define MAGIC_LENGTH 2
#define SSID_LENGTH 32
#define PASSWORD_LENGTH 64
// where configs start in memory
// MAGIC_LENGTH + SSID_LENGTH + PASSWORD_LENGTH
#define CONFIG_OFFSET 98

extern bool DEBUG_MODE;

#define DebugPrint(a) (DEBUG_MODE ? Serial.print(a) : false)
#define DebugPrintln(a) (DEBUG_MODE ? Serial.println(a) : false)

extern const char mimeHTML[];
extern const char mimeJSON[];
extern const char mimePlain[];
extern const char mimeCSS[];
extern const char mimeJS[];

enum wifiModes { ap, station };
enum ParameterMode { get, set, both };

/**
 * Base Parameter
 */
class BaseParameter {
 public:
  virtual ParameterMode getMode() = 0;
  virtual void fromJson(JsonObject* json) = 0;
  virtual void toJson(JsonObject* json) = 0;
  virtual void clearData() = 0;
};

/**
 * Config Parameter
 */
template <typename T>
class ConfigParameter : public BaseParameter {
 public:
  ConfigParameter(const char* name, T* ptr, ParameterMode mode = both) {
    this->name = name;
    this->ptr = ptr;
    this->mode = mode;
  }

  ParameterMode getMode() { return this->mode; }

  void update(T value) { *ptr = value; }

  void fromJson(JsonObject* json) {
    if (json->containsKey(name) && json->getMember(name).is<T>()) {
      this->update(json->getMember(name).as<T>());
    }
  }

  void toJson(JsonObject* json) {
    // json->set(name, *ptr);
    json->getOrAddMember(name).set(*ptr);
  }

  void clearData() {
    DebugPrint("Clearing: ");
    DebugPrintln(name);
    *ptr = T();
  }

 private:
  const char* name;
  T* ptr;
  std::function<void(const char*)> cb;
  ParameterMode mode;
};

/**
 * Config String Parameter
 */
class ConfigStringParameter : public BaseParameter {
 public:
  ConfigStringParameter(const char* name,
                        char* ptr,
                        size_t length,
                        ParameterMode mode = both) {
    this->name = name;
    this->ptr = ptr;
    this->length = length;
    this->mode = mode;
  }

  ParameterMode getMode() { return this->mode; }

  void update(const char* value) {
    memset(ptr, 0, length);
    strncpy(ptr, value, length - 1);
  }

  void fromJson(JsonObject* json) {
    if (json->containsKey(name) && json->getMember(name).is<char*>()) {
      const char* value = json->getMember(name).as<const char*>();
      this->update(value);
    }
  }

  void toJson(JsonObject* json) {
    json->getOrAddMember(name).set((const char*)ptr);
  }

  void clearData() {
    DebugPrint("Clearing: ");
    DebugPrintln(name);
    memset(ptr, 0, length);
  }

 private:
  const char* name;
  char* ptr;
  size_t length;
  ParameterMode mode;
};

/**
 * Config Manager
 */
class ConfigManager {
 public:
  ConfigManager() {}

  JsonObject asJson();
  wifiModes getMode();
  String scanNetworks();

  void setAPName(const char* name);
  void setAPPassword(const char* password);
  void setAPFilename(const char* filename);
  void setAPTimeout(const int timeout);
  void setWifiConfigURI(const char* uri);
  void setWifiConnectRetries(const int retries);
  void setWifiConnectInterval(const int interval);
  void setWebPort(const int port);
  void loop();
  void streamFile(const char* file, const char mime[]);
  void handleNotFound();
  void clearSettings(bool reboot);
  void clearWifiSettings(bool reboot);
  void clearAllSettings(bool reboot);
  void updateFromJson(JsonObject obj);
  void setAPCallback(std::function<void(WebServer*)> callback);
  void setAPICallback(std::function<void(WebServer*)> callback);
  void setInitCallback(std::function<void()> callback);
  void startWebserver();
  void stopWebserver();
  void save();
  bool wifiConnected();

  template <typename T>
  void begin(T& config) {
    this->config = &config;
    this->configSize = sizeof(T);

    EEPROM.begin(CONFIG_OFFSET + this->configSize);
    this->memoryInitialized = true;

    setup();
  }

  template <typename T>
  void addParameter(const char* name, T* variable) {
    parameters.push_back(new ConfigParameter<T>(name, variable));
  }
  template <typename T>
  void addParameter(const char* name, T* variable, ParameterMode mode) {
    parameters.push_back(new ConfigParameter<T>(name, variable, mode));
  }
  void addParameter(const char* name, char* variable, size_t size) {
    parameters.push_back(new ConfigStringParameter(name, variable, size));
  }
  void addParameter(const char* name,
                    char* variable,
                    size_t size,
                    ParameterMode mode) {
    parameters.push_back(new ConfigStringParameter(name, variable, size, mode));
  }

 private:
  wifiModes wifiMode;
  void* config;
  size_t configSize;

  bool memoryInitialized = false;
  bool webserverRunning = false;

  char* apName = (char*)"ConfigManager-Thing";
  char* apPassword = NULL;
  char* apFilename = (char*)"/index.html";
  int apTimeout = 0;
  unsigned long apStart = 0;

  char* wifiConfigURI = (char*)"/";

  int wifiConnectAttempts = 3;
  int wifiConnectRetries = 20;
  int wifiConnectInterval = 500;

  int webPort = 80;

  std::unique_ptr<DNSServer> dnsServer;
  std::list<BaseParameter*> parameters;

  std::unique_ptr<WebServer> server;
  std::function<void(WebServer*)> apCallback;
  std::function<void(WebServer*)> apiCallback;

  std::function<void()> initCallback;

  JsonObject decodeJson(String jsonString);

  void handleAPGet();
  void handleAPPost();
  void handleScanGet();
  void handleSettingsGetREST();
  void handleSettingsPutREST();

  bool wifiConnect(char* ssid, char* password);
  void setup();
  void startAP();
  void startAPApi();
  void startApi();
  void createBaseWebServer();

  void readConfig();
  void writeConfig();
  bool commitChanges();
  void storeWifiSettings(String ssid, String password);
  boolean isIp(String str);
  String toStringIP(IPAddress ip);
};

#endif /* __CONFIGMANAGER_H__ */
