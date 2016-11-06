#include "ConfigManager.h"

const byte DNS_PORT = 53;

void ConfigManager::setAPName(const char *name) {
    this->apName = (char *)name;
}

void ConfigManager::setAPFilename(const char *filename) {
    this->apFilename = (char *)filename;
}

void ConfigManager::setAPICallback(std::function<void(ESP8266WebServer*)> callback) {
    this->apiCallback = callback;
}

void ConfigManager::loop() {
    if (server) {
        server->handleClient();
    }
}

void ConfigManager::save() {
    this->writeConfig();
}

JsonObject &ConfigManager::decodeJson(String jsonString)
{
    DynamicJsonBuffer jsonBuffer;

    if (jsonString.length() == 0) {
        return jsonBuffer.createObject();
    }

    JsonObject& obj = jsonBuffer.parseObject(jsonString);

    if (!obj.success()) {
        return jsonBuffer.createObject();
    }

    return obj;
}

void ConfigManager::handleAPGet() {
    SPIFFS.begin();

    File f = SPIFFS.open(apFilename, "r");
    if (!f) {
        Serial.println("file open failed");
        server->send(500, "text/html", "File not found");
        return;
    }

    String content = f.readString();
    server->send(200, "text/html", content);

    f.close();
}

void ConfigManager::handleAPPost() {
    bool isJson = server->header("Content-Type") == "application/json";
    String ssid;
    String password;
    char ssidChar[32];
    char passwordChar[64];

    if (isJson) {
        JsonObject& obj = this->decodeJson(server->arg("plain"));

        ssid = obj.get<String>("ssid");
        password = obj.get<String>("password");
    } else {
        ssid = server->arg("ssid");
        password = server->arg("password");
    }

    if (ssid.length() == 0) {
        server->send(400, "text/plain", "Invalid ssid or password.");
        return;
    }

    strncpy(ssidChar, ssid.c_str(), sizeof(ssidChar));
    strncpy(passwordChar, password.c_str(), sizeof(passwordChar));

    EEPROM.put(0, ssidChar);
    EEPROM.put(32, passwordChar);
    EEPROM.commit();

    server->send(204, "text/plain", "Saved. Will attempt to reboot.");

    ESP.restart();
}

void ConfigManager::handleRESTGet() {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& obj = jsonBuffer.createObject();

    std::list<BaseParameter*>::iterator it;
    for (it = parameters.begin(); it != parameters.end(); ++it) {
        (*it)->toJson(&obj);
    }

    String body;
    obj.printTo(body);

    server->send(200, "application/json", body);
}

void ConfigManager::handleRESTPut() {
    JsonObject& obj = this->decodeJson(server->arg("plain"));
    if (!obj.success()) {
        server->send(400, "application/json", "");
        return;
    }

    std::list<BaseParameter*>::iterator it;
    for (it = parameters.begin(); it != parameters.end(); ++it) {
        (*it)->fromJson(&obj);
    }

    writeConfig();

    server->send(204, "application/json", "");
}

void ConfigManager::handleNotFound() {
    if (!isIp(server->hostHeader()) ) {
        server->sendHeader("Location", String("http://") + toStringIP(server->client().localIP()), true);
        server->send(302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
        server->client().stop();
        return;
    }

    server->send(404, "text/plain", "");
    server->client().stop();
}

bool ConfigManager::wifiConnected() {
    Serial.print("Waiting for WiFi to connect");

    int i = 0;
    while (i < 20) {
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("");
            return true;
        }

        Serial.print(".");

        delay(500);
        i++;
    }

    Serial.println("");
    Serial.println("Connection timed out");

    return false;
}

void ConfigManager::setup() {
    char ssid[32];
    char password[64];

    Serial.println("Reading saved configuration");

    EEPROM.get(0, ssid);
    EEPROM.get(32, password);
    readConfig();

    if (ssid != NULL) {
        WiFi.begin(ssid, password[0] == '\0' ? NULL : password);
        if (wifiConnected()) {
            Serial.print("Connected to ");
            Serial.print(ssid);
            Serial.print(" with ");
            Serial.println(WiFi.localIP());

            WiFi.mode(WIFI_STA);
            startApi();
            return;
        }
    }

    startAP();
}

void ConfigManager::startAP() {
    const char* headerKeys[] = {"Content-Type"};
    size_t headerKeysSize = sizeof(headerKeys)/sizeof(char*);

    Serial.println("Starting Access Point");

    IPAddress ip(192, 168, 1, 1);
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(ip, ip, IPAddress(255, 255, 255, 0));
    WiFi.softAP(apName);

    delay(500); // Need to wait to get IP

    DNSServer dnsServer;
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(DNS_PORT, "*", ip);

    server.reset(new ESP8266WebServer(80));
    server->collectHeaders(headerKeys, headerKeysSize);
    server->on("/", HTTPMethod::HTTP_GET, std::bind(&ConfigManager::handleAPGet, this));
    server->on("/", HTTPMethod::HTTP_POST, std::bind(&ConfigManager::handleAPPost, this));
    server->onNotFound(std::bind(&ConfigManager::handleNotFound, this));
    server->begin();

    while (1) {
        dnsServer.processNextRequest();
        server->handleClient();
    }
}

void ConfigManager::startApi() {
    const char* headerKeys[] = {"Content-Type"};
    size_t headerKeysSize = sizeof(headerKeys)/sizeof(char*);

    server.reset(new ESP8266WebServer(80));
    server->collectHeaders(headerKeys, headerKeysSize);
    server->on("/", HTTPMethod::HTTP_GET, std::bind(&ConfigManager::handleAPGet, this));
    server->on("/", HTTPMethod::HTTP_POST, std::bind(&ConfigManager::handleAPPost, this));
    server->on("/settings", HTTPMethod::HTTP_GET, std::bind(&ConfigManager::handleRESTGet, this));
    server->on("/settings", HTTPMethod::HTTP_PUT, std::bind(&ConfigManager::handleRESTPut, this));

    if (apiCallback) {
        apiCallback(server.get());
    }

    server->onNotFound(std::bind(&ConfigManager::handleNotFound, this));
    server->begin();
}

void ConfigManager::readConfig() {
    byte *ptr = (byte *)config;

    for (int i = 0; i < configSize; i++) {
        *(ptr++) = EEPROM.read(96 + i);
    }
}

void ConfigManager::writeConfig() {
    byte *ptr = (byte *)config;

    for (int i = 0; i < configSize; i++) {
        EEPROM.write(96 + i, *(ptr++));
    }
    EEPROM.commit();
}

boolean ConfigManager::isIp(String str) {
  for (int i = 0; i < str.length(); i++) {
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
