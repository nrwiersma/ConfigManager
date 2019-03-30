![Logo](http://svg.wiersma.co.za/github/project?lang=cpp&title=ConfigManager&tag=wifi%20configuration%20manager)

[![arduino-library-badge](http://www.ardu-badge.com/badge/ConfigManager.svg)](http://www.ardu-badge.com/ConfigManager)

Wifi connection and configuration manager for ESP8266 and ESP32.

This library was made to ease the complication of configuring Wifi and other
settings on an ESP8266 or ESP32. It is roughly split into two parts, Wifi configuration
and REST variable configuration.

# Requires

* [ArduinoJson](https://github.com/bblanchon/ArduinoJson) version 5

# Quick Start

## Installing

You can install through the Arduino Library Manager. The package name is
**ConfigManager**.

## Usage

Include the library in your sketch

```cpp
#include <ConfigManager.h>
```

Initialize a global instance of the library

```cpp
ConfigManager configManager;
```

In your setup function start the manager

```cpp
configManager.setAPName("Demo");
configManager.setAPFilename("/index.html");
configManager.addParameter("name", config.name, 20);
configManager.addParameter("enabled", &config.enabled);
configManager.addParameter("hour", &config.hour);
configManager.addParameter("password", config.password, 20, set);
configManager.addParameter("version", &meta.version, get);
configManager.begin(config);
```

In your loop function, run the manager loop

```cpp
configManager.loop();
```

Upload the ```index.html``` file found in the ```data``` directory into the SPIFFS.
Instructions on how to do this vary based on your IDE. Below are links instructions
on the most common IDEs:

#### ESP8266

* [Arduino IDE](http://arduino-esp8266.readthedocs.io/en/latest/filesystem.html#uploading-files-to-file-system)

* [Platform IO](http://docs.platformio.org/en/stable/platforms/espressif.html#uploading-files-to-file-system-spiffs)

#### ESP32

* [Arduino IDE](https://github.com/me-no-dev/arduino-esp32fs-plugin)

* [Platform IO](http://docs.platformio.org/en/stable/platforms/espressif32.html#uploading-files-to-file-system-spiffs)

# Documentation

## Methods

### getMode
```
Mode getMode()
```
> Gets the current mode, **ap** or **api**.

### setAPName
```
void setAPName(const char *name)
```
> Sets the name used for the access point.

### setAPPassword
```
void setAPPassword(const char *password)
```
> Sets the password used for the access point. For WPA2-PSK network it should be at least 8 character long.
> If not specified, the access point will be open for anybody to connect to.

### setAPFilename
```
void setAPFilename(const char *filename)
```
> Sets the path in SPIFFS to the webpage to be served by the access point.

### setAPTimeout
```
void setAPTimeout(const int timeout)
```
> Sets the access point timeout, in seconds (default 0, no timeout).
>
> **Note:** *The timeout starts when the access point is started, but is evaluated in the loop function.*

### setAPCallback
```
void setAPCallback(std::function<void(WebServer*)> callback)
```
> Sets a callback allowing customized http endpoints to be set when the access point is setup.

### setAPICallback
```
void setAPICallback(std::function<void(WebServer*)> callback)
```
> Sets a callback allowing customized http endpoints to be set when the api is setup.

### setWifiConnectRetries
```
void setWifiConnectRetries(const int retries)
```
> Sets the number of Wifi connection retires. Defaults to 20.

### setWifiConnectInterval
```
void setWifiConnectInterval(const int interval)
```
> Sets the interval (in milliseconds) between Wifi connection retries. Defaults to 500ms.

### addParameter
```
template<typename T>
void addParameter(const char *name, T *variable)
```
or
```
template<typename T>
void addParameter(const char *name, T *variable, ParameterMode mode)
```
> Adds a parameter to the REST interface. The optional mode can be set to ```set```
> or ```get``` to make the parameter read or write only (defaults to ```both```).

### addParameter (string)
```
void addParameter(const char *name, char *variable, size_t size)
```
or
```
void addParameter(const char *name, char *variable, size_t size, ParameterMode mode)
```
> Adds a character array parameter to the REST interface.The optional mode can be set to ```set```
> or ```get``` to make the parameter read or write only (defaults to ```both```).

### begin
```
template<typename T>
void begin(T &config)
```
> Starts the configuration manager. The config parameter will be saved into
> and retrieved from the EEPROM.

### save
```
void save()
```
> Saves the config passed to the begin function to the EEPROM.

### loop
```
void loop()
```
> Handles any waiting REST requests.

# Endpoints

### GET /

> Gets the HTML page that is used to set the Wifi SSID and password.

+ Response 200 *(text/html)*

### POST /

> Sets the Wifi SSID and password. The form example can be found in the ```data``` directory.

+ Request *(application/x-www-form-urlencoded)*

```
ssid=access point&password=some password
```

+ Request *(application/json)*

```json
{
  "ssid": "access point",
  "password": "some password"
}
```

### GET /settings

> Gets the settings set in ```addParameter```.

+ Response 200 *(application/json)*

```json
{
  "enabled": true,
  "hour": 3
}
```

### PUT /settings

> Sets the settings set in ```addParameter```.

+ Request *(application/json)*

```json
{
  "enabled": false,
  "hour": 4
}
```

+ Response 400 *(application/json)*

+ Response 204 *(application/json)*
