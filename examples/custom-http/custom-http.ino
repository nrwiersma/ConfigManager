const char *settingsHTML = (char *)"/settings.html";
const char *stylesCSS = (char *)"/styles.css";
const char *mainJS = (char *)"/main.js";

const int DEVICENAMELEN = 28;

struct Config {
    char deviceName[DEVICENAMELEN];
    int randomNumber;
    int someNumber;
    bool debugEnabled;
} config;

struct Metadata {
    int8_t version;
} meta;

#include "ConfigManager.h"
ConfigManager configManager;

/**********
***********

Lets serve something with a different Library!!!
This library is only for ESP32
This library has naming conflicts with <Webserver.h>

***********
**********/

#if defined(ARDUINO_ARCH_ESP32)
namespace HTTP {
  #include "esp_http_server.h"
}
HTTP::httpd_handle_t h_server = NULL;
#endif

void customHTTP(int port=80);

void setup() {
    Serial.begin(115200);
    DEBUG_MODE = true;
    DebugPrintln(F(""));

    meta.version = 3;


    // Settings
    configManager.addParameter("deviceName", config.deviceName, DEVICENAMELEN);
    configManager.addParameter("randomNumber", &config.randomNumber, get);
    configManager.addParameter("someNumber", &config.someNumber);
    configManager.addParameter("debugEnabled", &config.debugEnabled);

    // Meta Settings
    configManager.addParameter("version", &meta.version, get);

    configManager.setAPICallback(APICallback);
    configManager.setAPCallback(APCallback);

    configManager.begin(config);

    /**********
    ***********

    Allow for the stop of ConfigMangers built in
    HTTP server, and start your own.

    This example allows HTTP in AP mode, but provides
    a custom HTTP server when in wifi station mode.

    ***********
    **********/

    bool stopHTTP = true;
    if (configManager.getMode() == station && stopHTTP) {
        configManager.stopWebserver();
        customHTTP();
    }

    /**********

    Updating settings variables .. and now you know

    **********/
    config.someNumber = 62;
    configManager.save();
}

void loop() {
    configManager.loop();
    DEBUG_MODE = config.debugEnabled;

    // Add your loop code here
    config.randomNumber = (int) random(1000);
    configManager.save();
}


void APCallback(WebServer *server) {
    server->on("/styles.css", HTTPMethod::HTTP_GET, [server](){
        configManager.streamFile(stylesCSS, mimeCSS);
    });

    DebugPrintln(F("AP Mode Enabled. You can call other functions that should run after a mode is enabled ... "));
}


void APICallback(WebServer *server) {
  server->on("/disconnect", HTTPMethod::HTTP_GET, [server](){
    configManager.clearWifiSettings(false);
  });

  server->on("/config", HTTPMethod::HTTP_GET, [server](){
    configManager.streamFile(settingsHTML, mimeHTML);
  });

  // NOTE: css/js can be embedded in a single page HTML
  server->on("/styles.css", HTTPMethod::HTTP_GET, [server](){
    configManager.streamFile(stylesCSS, mimeCSS);
  });

  server->on("/main.js", HTTPMethod::HTTP_GET, [server](){
    configManager.streamFile(mainJS, mimeJS);
  });

  server->on("/reboot", HTTPMethod::HTTP_GET, [server](){
    ESP.restart();
  });

}

#if defined(ARDUINO_ARCH_ESP32)
static esp_err_t indexHandler(HTTP::httpd_req_t *req) {
    Serial.println("/");
    String resp = "Stored Config randomNumber: " + (String) config.randomNumber;
    return HTTP::httpd_resp_send(req, resp.c_str(), strlen(resp.c_str()));
}
void registerIndex() {
    HTTP::httpd_uri_t uri = {
    .uri       = "serving /",
    .method    = HTTP::HTTP_GET,
    .handler   = indexHandler,
    .user_ctx  = NULL
  };

  HTTP::httpd_register_uri_handler(h_server, &uri);
}

static esp_err_t rebootHandler(HTTP::httpd_req_t *req) {
    Serial.println("serving /reboot");
    const char resp[] = "Rebooting";
    HTTP::httpd_resp_send(req, resp, strlen(resp));
    ESP.restart();
    return ESP_OK;
}
void registerReboot() {
    HTTP::httpd_uri_t uri = {
    .uri       = "/reboot",
    .method    = HTTP::HTTP_GET,
    .handler   = rebootHandler,
    .user_ctx  = NULL
  };

  HTTP::httpd_register_uri_handler(h_server, &uri);
}

static esp_err_t scanHandler(HTTP::httpd_req_t *req) {
    Serial.println("serving /scanner");
    String resp = configManager.scanNetworks();
    return HTTP::httpd_resp_send(req, resp.c_str(), strlen(resp.c_str()));
}
void registerScan() {
    HTTP::httpd_uri_t uri = {
    .uri       = "/scanner",
    .method    = HTTP::HTTP_GET,
    .handler   = scanHandler,
    .user_ctx  = NULL
  };

  HTTP::httpd_register_uri_handler(h_server, &uri);
}

void customHTTP(int port) {
    Serial.println("Setting up custom HTTP");
    HTTP::httpd_config_t h_config = HTTPD_DEFAULT_CONFIG();
    h_config.server_port = port;
    HTTP::httpd_start(&h_server, &h_config);

    registerIndex();
    registerScan();
    registerReboot();

    Serial.print("Ready at: http://");
    Serial.println(WiFi.localIP());
}
#else
void customHTTP(int port) {
    Serial.println("No HTTP Server defined for ESP8266");
}
#endif

