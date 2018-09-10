#ifndef __CONFIGMANAGER_H__
#define __CONFIGMANAGER_H__

#include <DNSServer.h>
#include <EEPROM.h>
#include <FS.h>

#if defined(ARDUINO_ARCH_ESP8266) //ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#elif defined(ARDUINO_ARCH_ESP32) //ESP32
#include <SPIFFS.h>
#include <WiFi.h>
#include <WebServer.h>
#endif

#include <functional>
#include <list>
#include "ArduinoJson.h"
#include <stdint.h>

#define CONFIG_OFFSET 132		// sizeof(wifiDetails) + sizeof(magicHeaderT)

#if defined(ARDUINO_ARCH_ESP8266) //ESP8266
using WebServer = ESP8266WebServer;
#endif

enum Mode {
	ap, api
};

enum ParameterMode {
	get, set, both
};

/**
 * Base Parameter
 */
class BaseParameter {
public:
	BaseParameter(const char *name, ParameterMode mode = both) {
		this->name = name;

		this->mode = mode;

	}
	ParameterMode getMode() {
		return this->mode;
	}

	virtual void fromJson(JsonObject *json) = 0;
	virtual void toJson(JsonObject *json) = 0;

protected:
	const char *name;
	ParameterMode mode;

};

/**
 * Config Parameter
 */
template<typename T>
class ConfigParameter: public BaseParameter {
public:
	ConfigParameter(const char *name, T *ptr, ParameterMode mode = both, std::function<void(const char*, const T*, const T*)> cb = NULL) :
			BaseParameter(name, mode) {
		this->ptr = ptr;
		this->cb = cb;
	}

	void fromJson(JsonObject *json) {
		if (json->containsKey(name) && json->is<T>(name)) {

			T newValue = json->get<T>(name);

			if (*ptr != newValue) {
				if (cb) {
					cb(name, ptr, &newValue);
				}
				*ptr = newValue;
			}
		}
	}

	void toJson(JsonObject *json) {
		json->set(name, *ptr);
	}

private:
	T *ptr;
	std::function<void(const char* name, const T *oldValue, const T *newValue)> cb;
};

/**
 * Config String Parameter
 */
class ConfigStringParameter: public BaseParameter {
public:
	ConfigStringParameter(const char *name, char *ptr, size_t length, ParameterMode mode = both, std::function<void(const char*, const char*, const char*)> cb =
	NULL) :
			BaseParameter(name, mode) {
		this->ptr = ptr;
		this->length = length;
		this->cb = cb;
	}

	void fromJson(JsonObject *json) {
		if (json->containsKey(name) && json->is<char *>(name)) {
			const char * newValue = json->get<const char *>(name);

			if (strncmp(newValue, ptr, length) != 0) {
				if (cb) {
					cb(name, ptr, newValue);
				}

				memset(ptr, '\0', length);
				strncpy(ptr, const_cast<char*>(newValue), length - 1);

			}
		}
	}

	void toJson(JsonObject *json) {
		json->set(name, ptr);
	}

private:
	char *ptr;
	size_t length;
	std::function<void(const char* name, const char* oldValue, const char* newValue)> cb;
};

/**
 * Config Manager
 */
class ConfigManager {
public:
	ConfigManager() {
	}

	Mode getMode();
	void setAPName(const char *name);
	void setAPFilename(const char *filename);
	void setAPTimeout(const int timeout);
	void setWifiConnectRetries(const int retries);
	void setWifiConnectInterval(const int interval);
	void setAPCallback(std::function<void(WebServer*)> callback);
	void setAPICallback(std::function<void(WebServer*)> callback);
	void loop();

	template<typename T>
	void begin(T &config, const uint32_t *hostnamePostfix = NULL) {
		this->config = &config;
		this->configSize = sizeof(T);
		EEPROM.begin(CONFIG_OFFSET + this->configSize);

		setup(hostnamePostfix);
	}

	template<typename T>
	void addParameter(const char *name, T *variable, ParameterMode mode = both, std::function<void(const char*, const T*, const T*)> cb = NULL) {
		parameters.push_back(new ConfigParameter<T>(name, variable, both, cb));
	}

//	template<typename T>
//	void addParameter(const char *name, T &variable, ParameterMode mode = both, std::function<void(const char*, T&, T&)> cb = NULL) {
//		parameters.push_back(new ConfigParameter<T>(name, variable, mode, cb));
//	}

	void addParameter(const char *name, char *variable, size_t size, ParameterMode mode = both, std::function<void(const char*, const char*, const char*)> cb =
	NULL) {
		parameters.push_back(new ConfigStringParameter(name, variable, size, mode, cb));
	}
	void save();

private:
	Mode mode;
	void *config;
	size_t configSize;

	char *apName = (char *) "Thing";
	char *apFilename = (char *) "/index.html";
	int apTimeout = 0;
	unsigned long apStart = 0;

	int wifiConnectRetries = 20;
	int wifiConnectInterval = 500;

	std::unique_ptr<DNSServer> dnsServer;
	std::unique_ptr<WebServer> server;
	std::list<BaseParameter*> parameters;

	std::function<void(WebServer*)> apCallback;
	std::function<void(WebServer*)> apiCallback;

	JsonObject &decodeJson(String jsonString);

	void handleAPGet();
	void handleAPPost();
	void handleRESTGet();
	void handleRESTPut();
	void handleNotFound();

	bool wifiConnected();
	void setup(const uint32_t *hostnamePostfix = NULL);
	void startAP();
	void startApi();

	void readConfig();
	void writeConfig();
};

#endif /* __CONFIGMANAGER_H__ */
