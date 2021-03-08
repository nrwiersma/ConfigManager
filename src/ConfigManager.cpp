#include "ConfigManager.h"

const byte DNS_PORT = 53;
const char magicBytes[MAGIC_LENGTH] = {'C', 'M'};
const char magicBytesEmpty[MAGIC_LENGTH] = {'\0', '\0'};

const char mimeHTML[] PROGMEM = "text/html";
const char mimeJSON[] PROGMEM = "application/json";
const char mimePlain[] PROGMEM = "text/plain";
const char mimeCSS[] PROGMEM = "text/css";
const char mimeJS[] PROGMEM = "application/javascript";

bool DEBUG_MODE = false;

//
// Setup and Loop
//
void ConfigManager::setup() {
  char magic[MAGIC_LENGTH];
  char ssid[SSID_LENGTH];
  char password[PASSWORD_LENGTH];

  DebugPrint(F("MAC: "));
  DebugPrintln(WiFi.macAddress());

  DebugPrintln(F("Checking for magic initialization"));
  EEPROM.get(0, magic);

  if (memcmp(magic, magicBytes, MAGIC_LENGTH) == 0) {
    DebugPrintln(F("Reading saved configuration"));
    readConfig();

    EEPROM.get(MAGIC_LENGTH, ssid);

    if (strlen(ssid) > 0) {
      DebugPrint(F("SSID: \""));
      DebugPrint(ssid);
      DebugPrintln(F("\""));

      EEPROM.get(MAGIC_LENGTH + SSID_LENGTH, password);

      int attempt = 0;
      bool success = false;

      while (attempt < this->wifiConnectAttempts && !success) {
        attempt++;
        DebugPrintln(F(""));
        DebugPrint(F("Wifi connection attempt "));
        DebugPrintln(attempt);
        success = wifiConnect(ssid, password);
      }

      if (success) {
        startApi();
      } else {
        DebugPrintln(F(""));
        DebugPrintln(F("Wifi connection could not be established"));
      }

    } else {
      DebugPrintln(F("No SSID found"));
    }
  } else {
    // We are at a cold start, don't bother timing out.
    if (initCallback) {
      initCallback();
    }
    apTimeout = 0;
    DebugPrintln(F("MagicBytes mismatch"));
  }

  if (this->getMode() != station) {
    startAP();
    startAPApi();
  }
}

void ConfigManager::loop() {
  if (this->getMode() == ap) {
    if (apTimeout > 0 && ((millis() - apStart) / 1000) > (uint16_t)apTimeout) {
      ESP.restart();
    }

    if (dnsServer) {
      dnsServer->processNextRequest();
    }
  }

  if (server && this->webserverRunning) {
    server->handleClient();
  }
}

wifiModes ConfigManager::getMode() {
  return this->wifiMode;
}

//
// ConfigManager AP Utilities
//
void ConfigManager::setAPName(const char* name) {
  this->apName = (char*)name;
}

void ConfigManager::setAPPassword(const char* password) {
  this->apPassword = (char*)password;
}

void ConfigManager::setAPFilename(const char* filename) {
  this->apFilename = (char*)filename;
}

void ConfigManager::setAPTimeout(const int timeout) {
  this->apTimeout = timeout;
}

void ConfigManager::startAP() {
  this->wifiMode = ap;

  DebugPrintln(F("Starting Access Point"));

  WiFi.mode(WIFI_AP);
  WiFi.softAP(apName, apPassword);

  delay(500);  // Need to wait to get IP

  IPAddress ip(192, 168, 1, 1);
  IPAddress NMask(255, 255, 255, 0);
  WiFi.softAPConfig(ip, ip, NMask);

  DebugPrint("AP Name: ");
  DebugPrintln(apName);

  IPAddress myIP = WiFi.softAPIP();
  DebugPrint("AP IP address: ");
  DebugPrintln(myIP);

  dnsServer.reset(new DNSServer);
  dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer->start(DNS_PORT, "*", ip);

  apStart = millis();
}

void ConfigManager::startAPApi() {
  DebugPrintln(F("AP Api Mode"));
  createBaseWebServer();

  if (apCallback) {
    apCallback(server.get());
  }

  this->startWebserver();
}

void ConfigManager::startApi() {
  DebugPrintln(F("Station Mode"));
  createBaseWebServer();

  server->on("/settings", HTTPMethod::HTTP_GET,
             std::bind(&ConfigManager::handleSettingsGetREST, this));
  server->on("/settings", HTTPMethod::HTTP_PUT,
             std::bind(&ConfigManager::handleSettingsPutREST, this));

  if (apiCallback) {
    apiCallback(server.get());
  }

  this->startWebserver();
}

void ConfigManager::setAPCallback(std::function<void(WebServer*)> callback) {
  this->apCallback = callback;
}

void ConfigManager::setAPICallback(std::function<void(WebServer*)> callback) {
  this->apiCallback = callback;
}

void ConfigManager::setInitCallback(std::function<void()> callback) {
  this->initCallback = callback;
}

//
// ConfigManager Wifi Utilitiees
//
void ConfigManager::setWifiConfigURI(const char* uri) {
  this->wifiConfigURI = (char*)uri;
}

void ConfigManager::setWifiConnectRetries(const int retries) {
  this->wifiConnectRetries = retries;
}

void ConfigManager::setWifiConnectInterval(const int interval) {
  this->wifiConnectInterval = interval;
}

bool ConfigManager::wifiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

bool ConfigManager::wifiConnect(char* ssid, char* password) {
  DebugPrintln(F("Waiting for WiFi to connect"));

  bool connected = false;
  int retry = 0;

  WiFi.begin(ssid, password[0] == '\0' ? NULL : password);

  while (retry < this->wifiConnectRetries && !connected) {
    DebugPrint(F("."));

    connected = this->wifiConnected();

    if (connected) {
      DebugPrintln(F(""));
      DebugPrint(F("Connected to "));
      DebugPrint(ssid);
      DebugPrint(F(" with "));
      DebugPrintln(WiFi.localIP());

      this->wifiMode = station;
      WiFi.mode(WIFI_STA);
    } else {
      delay(wifiConnectInterval);
    }

    retry++;
  }

  return connected;
}

void ConfigManager::storeWifiSettings(String ssid, String password) {
  char ssidChar[SSID_LENGTH];
  char passwordChar[PASSWORD_LENGTH];

  if (!this->memoryInitialized) {
    DebugPrintln(
        F("WiFi Settings cannot be stored before ConfigManager::begin()"));
    return;
  }

  strncpy(ssidChar, ssid.c_str(), SSID_LENGTH);
  strncpy(passwordChar, password.c_str(), PASSWORD_LENGTH);

  DebugPrint(F("Storing WiFi Settings for SSID: \""));
  DebugPrint(ssidChar);
  DebugPrintln(F("\""));

  EEPROM.put(MAGIC_LENGTH, ssidChar);
  EEPROM.put(MAGIC_LENGTH + SSID_LENGTH, passwordChar);
  bool wroteChange = this->commitChanges();

  DebugPrint(F("EEPROM committed: "));
  DebugPrintln(wroteChange ? F("true") : F("false"));
}

void ConfigManager::clearWifiSettings(bool reboot) {
  char ssid[SSID_LENGTH];
  char password[PASSWORD_LENGTH];
  memset(ssid, 0, SSID_LENGTH);
  memset(password, 0, PASSWORD_LENGTH);

  DebugPrintln(F("Clearing WiFi connection."));
  storeWifiSettings(ssid, password);

  if (reboot) {
    ESP.restart();
  }
}

String ConfigManager::scanNetworks() {
  DynamicJsonDocument doc(1024);
  JsonArray jsonArray = doc.createNestedArray();

  DebugPrintln("Scanning WiFi networks...");
  int n = WiFi.scanNetworks();
  DebugPrintln("scan complete");
  if (n == 0) {
    DebugPrintln("no networks found");
  } else {
    DebugPrint(n);
    DebugPrintln(" networks found:");

    for (int i = 0; i < n; ++i) {
      String ssid = WiFi.SSID(i);
      int rssi = WiFi.RSSI(i);
      String security =
          WiFi.encryptionType(i) == WIFI_OPEN ? "none" : "enabled";

      DebugPrint("Name: ");
      DebugPrint(ssid);
      DebugPrint(" - Strength: ");
      DebugPrint(rssi);
      DebugPrint(" - Security: ");
      DebugPrintln(security);

      JsonObject obj = doc.createNestedObject();
      obj["ssid"] = ssid;
      obj["strength"] = rssi;
      obj["security"] = security == "none" ? false : true;
      jsonArray.add(obj);
    }
  }

  String jsonSerialized;
  serializeJson(jsonArray, jsonSerialized);
  return jsonSerialized;
}

//
// ConfigManager Config Utilities
//
JsonObject ConfigManager::decodeJson(String jsonString) {
  DynamicJsonDocument doc(1024);

  if (jsonString.length() == 0) {
    return doc.as<JsonObject>();
  }

  auto error = deserializeJson(doc, jsonString);
  if (error) {
    DebugPrint(F("deserializeJson() failed with code "));
    DebugPrintln(error.c_str());
    return doc.as<JsonObject>();
  }

  return doc.as<JsonObject>();
}

void ConfigManager::clearAllSettings(bool reboot) {
  this->clearSettings(false);
  this->clearWifiSettings(false);
  EEPROM.put(0, magicBytesEmpty);
  EEPROM.commit();
  if (reboot) {
    ESP.restart();
  }
}

void ConfigManager::readConfig() {
  byte* ptr = (byte*)config;

  for (int i = 0; i < (int16_t)configSize; i++) {
    *(ptr++) = EEPROM.read(CONFIG_OFFSET + i);
  }
}

JsonObject ConfigManager::asJson() {
  DynamicJsonDocument doc(1024);
  JsonObject obj = doc.createNestedObject();

  std::list<BaseParameter*>::iterator it;
  for (it = parameters.begin(); it != parameters.end(); ++it) {
    if ((*it)->getMode() == set) {
      continue;
    }

    (*it)->toJson(&obj);
  }

  return obj;
}

bool ConfigManager::commitChanges() {
  EEPROM.put(0, magicBytes);
  return EEPROM.commit();
}

void ConfigManager::writeConfig() {
  byte* ptr = (byte*)config;

  for (int i = 0; i < (int16_t)configSize; i++) {
    EEPROM.write(CONFIG_OFFSET + i, *(ptr++));
  }
  this->commitChanges();
}

void ConfigManager::save() {
  this->writeConfig();
}

void ConfigManager::updateFromJson(JsonObject obj) {
  std::list<BaseParameter*>::iterator it;
  for (it = parameters.begin(); it != parameters.end(); ++it) {
    if ((*it)->getMode() == get) {
      continue;
    }

    (*it)->fromJson(&obj);
  }

  writeConfig();
}

void ConfigManager::clearSettings(bool reboot) {
  DebugPrintln(F("Clearing Settings...."));
  std::list<BaseParameter*>::iterator it;
  for (it = parameters.begin(); it != parameters.end(); ++it) {
    (*it)->clearData();
  }

  writeConfig();

  if (reboot) {
    ESP.restart();
  }
}

//
// ConfigManager HTTP Utilities
//
void ConfigManager::startWebserver() {
  this->server->begin();
  this->webserverRunning = true;
}

void ConfigManager::stopWebserver() {
  this->server->stop();
  DebugPrintln(F("ConfigManager Webserver Stopped"));
  this->webserverRunning = false;
}

void ConfigManager::setWebPort(const int port) {
  this->webPort = port;
}

void ConfigManager::createBaseWebServer() {
  const char* headerKeys[] = {"Content-Type"};
  size_t headerKeysSize = sizeof(headerKeys) / sizeof(char*);

  server.reset(new WebServer(this->webPort));
  DebugPrint(F("Webserver enabled on port: "));
  DebugPrintln(webPort);

  server->collectHeaders(headerKeys, headerKeysSize);

  server->on(this->wifiConfigURI, HTTPMethod::HTTP_GET,
             std::bind(&ConfigManager::handleAPGet, this));
  server->on(this->wifiConfigURI, HTTPMethod::HTTP_POST,
             std::bind(&ConfigManager::handleAPPost, this));
  DebugPrintln("Index page registered");

  server->on("/scan", HTTPMethod::HTTP_GET,
             std::bind(&ConfigManager::handleScanGet, this));
  DebugPrintln("Scan page registered");

  server->onNotFound(std::bind(&ConfigManager::handleNotFound, this));
}

void ConfigManager::streamFile(const char* file, const char mime[]) {
  SPIFFS.begin();

  File f;

  if (file[0] == '/') {
    f = SPIFFS.open(file, "r");
  } else {
    size_t len = strlen(file);
    char filepath[len + 1];
    strcpy(filepath, "/");
    strcat(filepath, file);
    f = SPIFFS.open(filepath, "r");
  }

  if (f) {
    server->streamFile(f, FPSTR(mime));
    f.close();
  } else {
    DebugPrint(F("file open failed "));
    DebugPrintln(file);
    handleNotFound();
  }
}

void ConfigManager::handleAPGet() {
  DebugPrint(F("Index Page: "));
  DebugPrintln(apFilename);
  streamFile(apFilename, mimeHTML);
}

void ConfigManager::handleAPPost() {
  bool isJson = server->header("Content-Type") == FPSTR(mimeJSON);
  String ssid;
  String password;

  if (isJson) {
    JsonObject obj = decodeJson(server->arg("plain"));

    ssid = obj.getMember("ssid").as<String>();
    password = obj.getMember("password").as<String>();
  } else {
    ssid = server->arg("ssid");
    password = server->arg("password");
  }

  if (ssid.length() == 0) {
    server->send(400, FPSTR(mimePlain), F("Invalid ssid."));
    return;
  }

  storeWifiSettings(ssid, password);
  if (this->getMode() != station) {
    this->writeConfig();
  }

  server->send(204, FPSTR(mimePlain), F("Saved. Will attempt to reboot."));
  // Allow enough time for the response to be sent before restarting.
  delay(500);

  ESP.restart();
}

void ConfigManager::handleScanGet() {
  String body = scanNetworks();
  server->send(200, FPSTR(mimeJSON), body);
}

void ConfigManager::handleSettingsGetREST() {
  JsonObject obj = asJson();
  String body;
  serializeJson(obj, body);

  DebugPrintln(body);
  server->send(200, FPSTR(mimeJSON), body);
}

void ConfigManager::handleSettingsPutREST() {
  DynamicJsonDocument doc(1024);
  auto error = deserializeJson(doc, server->arg("plain"));
  if (error) {
    server->send(400, FPSTR(mimeJSON), "");
    return;
  }

  JsonObject obj = doc.as<JsonObject>();
  updateFromJson(obj);

  server->send(204, FPSTR(mimeJSON), "");
}

void ConfigManager::handleNotFound() {
  String URI =
      toStringIP(server->client().localIP()) + String(":") + String(webPort);
  String header = server->hostHeader();

  if (!isIp(header) && header != URI) {
    DebugPrint(F("Unknown URL: "));
    DebugPrintln(header);
    server->sendHeader("Location", String("http://") + URI, true);
    server->send(302, FPSTR(mimePlain), "");
    // Empty content inhibits Content-length header so we
    // have to close the socket ourselves.
    server->client().stop();
    return;
  }

  server->send(404, FPSTR(mimePlain), "File Not Found");
}

//
// ConfigManager General Util
//
boolean ConfigManager::isIp(String str) {
  for (uint i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

String ConfigManager::toStringIP(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

#ifdef localbuild
// used to compile the project from the project
// and not as a library from another one
void setup() {}
void loop() {}
#endif
