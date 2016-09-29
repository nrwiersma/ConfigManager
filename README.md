# ConfigManager

ESP8266 Wifi connection and configuration manager.

This library was made to ease the complication of configuring Wifi and other
settings on an ESP8266. It is roughly split into two parts, Wifi configuration
and REST variable configuration.

# Requires

* [ArduinoJson](https://github.com/bblanchon/ArduinoJson)

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
configManager.begin(config);
```

In your loop function, run the manager loop

```cpp
configManager.loop();
```

Upload the ```index.html``` file found in the ```data``` directory into the SPIFFS.
Instructions on how to do this vary based on your IDE. Below are links instructions
on the most common IDEs:

* [Arduino IDE](https://github.com/esp8266/Arduino/blob/master/doc/filesystem.md#uploading-files-to-file-system)

* [Platform IO](http://docs.platformio.org/en/stable/platforms/espressif.html#uploading-files-to-file-system-spiffs)

# Documentation

## Methods

### setAPName
```
void setAPName(const char *name)
```
> Sets the name used for the access point.

### setAPFilename
```
void setAPFilename(const char *filename)
```
> Sets the path in SPIFFS to the webpage to be served by the access point.

### setAPICallback
```
void setAPICallback(std::function<void(ESP8266WebServer*)> callback)
```
> Sets a callback allowing customized http endpoints to be set when the api is setup.

### addParameter
```
template<typename T>
void addParameter(const char *name, T *variable)
```
> Adds a parameter to the REST interface.

### addParameter (string)
```
void addParameter(const char *name, char *variable, size_t size)
```
> Adds a character array parameter to the REST interface.

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
