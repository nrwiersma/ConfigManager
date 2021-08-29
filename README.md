![Logo](http://svg.wiersma.co.za/github/project?lang=cpp&title=ConfigManager&tag=wifi%20configuration%20manager)

[![Build Status](https://github.com/nrwiersma/ConfigManager/actions/workflows/test.yml/badge.svg)](https://github.com/nrwiersma/ConfigManager/actions)
[![arduino-library-badge](http://www.ardu-badge.com/badge/ConfigManager.svg)](http://www.ardu-badge.com/ConfigManager)

Wifi connection and configuration manager for ESP8266 and ESP32.

This library was made to ease the complication of configuring Wifi and other
settings on an ESP8266 or ESP32. It is roughly split into two parts, Wifi configuration
and REST variable configuration.

# Requires

* [ArduinoJson](https://github.com/bblanchon/ArduinoJson) version 6

# Quick Start

## Installing

You can install through the Arduino Library Manager. The package name is
**ConfigManager**.

## Usage

Include the library in your sketch

```cpp
#include <ConfigManager.h>
```

Create your `config` and `meta` structs. These are the definitions for your settings.

```cpp
struct Config {
    char name[20];
    bool enabled;
    int8_t hour;
    char password[20];
} config;

struct Metadata {
    int8_t version;
} meta;

```

> Note: These are examples of possible settings. The `config` is required, these values are arbitrary.

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

To access your config data through device code

```cpp
char* name = config.name;
int version = meta.version;
```

To change a value through device code

```cpp
strncpy(config.name, "New Name", 8);
meta.version = 8;
configManager.save();
```

### Additional files saved

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

## Debugging

### Enabling

By default, ConfigManager runs in `DEBUG_MODE` off. This is to allow the serial iterface to communicate as needed.
To turn on debugging, add the following line inside your `setup` routine:

```
DEBUG_MODE = true
```

### Using

To use the debugger, change your `Serial.print` calls to `DebugPrint`. Output will then be toggled via the debugger.

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

### setInitCallback
```
void setInitCallback(std::function<void()> callback)
```
> Sets a function that will be called when the ConfigManager is first initialized. This can be used to
set default values on configuration parameters.

### setAPCallback
```
void setAPCallback(std::function<void(WebServer*)> callback)
```
> Sets a function that will be called when the WebServer is started in AP mode allowing custom HTTP endpoints to be created.

### setAPICallback
```
void setAPICallback(std::function<void(WebServer*)> callback)
```
> Sets a function that will be called when the WebServer is started in API/Settings mode allowing custom HTTP endpoints to be created.

### setWifiConfigURI
```
void setWifiConfigURI(const char* uri)
```
> Changes the URI for the Wifi Configuration Page. Defaults to "/"

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

### setWebPort
```
void setWebPort(const int port)
```
> Sets the port that the web server listens on. Defaults to 80.

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

### clearWifiSettings(bool reboot)

> Sets SSID/Password to `NULL`
> The `bool reboot` indicates if the device should restart after clearing the values.

### clearSettings(bool reboot)

> Sets all managed settings (not Wifi) to `NULL`. This is useful to clear all setting, but maintain Wifi.
> The `bool reboot` indicates if the device should restart after clearing the values.

### clearAllSettings(bool reboot)

> Sets all settings (managed and Wifi) to `NULL`. This is useful to re-initialize the memory of the device.
> The `bool reboot` indicates if the device should restart after clearing the values.

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

### streamFile(const char &ast;file, const char mime[])

```
server->on("/settings.html", HTTPMethod::HTTP_GET, [server](){
    configManager.streamFile(settingsHTMLFile, mimeHTML);
});
```

> Stream a file to the server when using custom routing endpoints.
> See `example/save_config_demo/save_config_demo.ino`

### stopWebserver()
```
void ConfigManager::stopWebserver()
```
> Stops the built-in webserver to allow your custom project
> webserver to run without port/resource interference.
> See `example/custom-http/custom-http.ino`


# Endpoints


## GET /

###### Modes: *AP and API*

> Gets the HTML page that is used to set the Wifi SSID and password.

+ Response 200 *(text/html)*

## POST /

###### Modes: *AP and API*

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

## GET /scan

###### Modes: *AP and API*

> Scans visible networks

+ Will print to Serial Monitor is `DEBUG_MODE = true`

+ Response 200 *(application/json)*

```json
{
  "ssid": "access point name",
  "strength": *int*,
  "security": *bool*
}
```

## GET /settings

###### Modes: *API*

> Gets the settings set in ```addParameter```.

+ Response 200 *(application/json)*

```json
{
  "enabled": true,
  "hour": 3
}
```

## PUT /settings

###### Modes: *API*

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
