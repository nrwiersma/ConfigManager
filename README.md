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

Intialise a global instance of the library

```cpp
ConfigManager configManager;
```

In your setup function start the manager

```cpp
configManager.setAPName("Demo");
configManager.setAPFilename("/index.html");
configManager.addParameter("enabled", &config.enabled);
configManager.addParameter("hour", &config.hour);
configManager.begin(config);
```

In your loop function, run the manager loop

```cpp
configManager.loop();
```

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

### addParameter
```
template<typename T>
void addParameter(const char *name, T *variable)
```
> Adds a parameter to the REST interface.

### begin
```
template<typename T>
void begin(T &config)
```
> Starts the configuration manager. The config parameter will be saved into
> and retrieved from the EEPROM.

### loop
```
void loop()
```
> Handles any waiting REST requests.
