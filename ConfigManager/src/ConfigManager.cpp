#include "ConfigManager.h"

#define MAJORVERSION 0
#define MINORVERSION 1

#define SSID_LENGTH 32
#define SSID_PWD_LENGTH 64
#define HOSTNAME_LENGTH 32

const byte DNS_PORT = 53;
const char magicBytes[2] = { 'C', 'M' };

struct magicHeaderT {
	char magicBytes[2] = { 'C', 'M' };
	byte majorVersion = MAJORVERSION;
	byte minorVersion = MINORVERSION;
};
const magicHeaderT magicHeader;

struct wifiDetails {
	char ssid[SSID_LENGTH];
	char password[SSID_PWD_LENGTH];
	char hostname[HOSTNAME_LENGTH];
};

const char mimeHTML[] PROGMEM = "text/html";
const char mimeJSON[] PROGMEM = "application/json";
const char mimePlain[] PROGMEM = "text/plain";

Mode ConfigManager::getMode() {
	return this->mode;
}

void ConfigManager::setAPName(const char *name) {
	this->apName = (char *) name;
}

void ConfigManager::setAPFilename(const char *filename) {
	this->apFilename = (char *) filename;
}

void ConfigManager::setAPTimeout(const int timeout) {
	this->apTimeout = timeout;
}

void ConfigManager::setWifiConnectRetries(const int retries) {
	this->wifiConnectRetries = retries;
}

void ConfigManager::setWifiConnectInterval(const int interval) {
	this->wifiConnectInterval = interval;
}

void ConfigManager::setAPCallback(std::function<void(WebServer*)> callback) {
	this->apCallback = callback;
}

void ConfigManager::setAPICallback(std::function<void(WebServer*)> callback) {
	this->apiCallback = callback;
}

void ConfigManager::loop() {
	if (mode == ap && apTimeout > 0 && ((millis() - apStart) / 1000) > apTimeout) {
		ESP.restart();
	}

	if (dnsServer) {
		dnsServer->processNextRequest();
	}

	if (server) {
		server->handleClient();
	}
}

void ConfigManager::save() {
	this->writeConfig();
}

JsonObject &ConfigManager::decodeJson(String jsonString) {
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
		Serial.println(F("file open failed"));
		server->send(404, FPSTR(mimeHTML), F("File not found"));
		return;
	}

	server->streamFile(f, FPSTR(mimeHTML));

	f.close();
}

void ConfigManager::handleAPPost() {
	bool isJson = server->header("Content-Type") == FPSTR(mimeJSON);
	String ssid;
	String password;
	String hostname;

	if (isJson) {
		JsonObject& obj = this->decodeJson(server->arg("plain"));

		ssid = obj.get<String>("ssid");
		password = obj.get<String>("password");
		hostname = obj.get<String>("hostname");
	} else {
		ssid = server->arg("ssid");
		password = server->arg("password");
		hostname = server->arg("hostname");
	}

	if (ssid.length() == 0 || hostname.length() == 0) {
		server->send(400, FPSTR(mimePlain), F("Invalid ssid or hostname."));
		return;
	}

	wifiDetails details;
	strncpy(details.ssid, ssid.c_str(), SSID_LENGTH);
	strncpy(details.password, password.c_str(), SSID_PWD_LENGTH);
	strncpy(details.hostname, hostname.c_str(), HOSTNAME_LENGTH);

	Serial.printf("details.hostname = %s\n\r", details.hostname);

	EEPROM.put(0, magicHeader);
	EEPROM.put(sizeof(magicHeader), details);

	EEPROM.commit();

	server->send(204, FPSTR(mimePlain), F("Saved. Will attempt to reboot."));

	Serial.flush();

	yield();

	//ESP.restart();
}

void ConfigManager::handleRESTGet() {
	DynamicJsonBuffer jsonBuffer;
	JsonObject& obj = jsonBuffer.createObject();

	std::list<BaseParameter*>::iterator it;
	for (it = parameters.begin(); it != parameters.end(); ++it) {
		if ((*it)->getMode() == set) {
			continue;
		}

		(*it)->toJson(&obj);
	}

	String body;
	obj.printTo(body);

	server->send(200, FPSTR(mimeJSON), body);
}

void ConfigManager::handleRESTPut() {
	JsonObject& obj = this->decodeJson(server->arg("plain"));
	if (!obj.success()) {
		server->send(400, FPSTR(mimeJSON), "");
		return;
	}

	std::list<BaseParameter*>::iterator it;
	for (it = parameters.begin(); it != parameters.end(); ++it) {
		if ((*it)->getMode() == get) {
			continue;
		}

		(*it)->fromJson(&obj);
	}

	writeConfig();

	server->send(204, FPSTR(mimeJSON), "");
}

void ConfigManager::handleNotFound() {
	if (IPAddress().isValid(server->hostHeader())) { //} !isIp(server->hostHeader()) ) {
		server->sendHeader("Location", String("http://") + server->client().localIP().toString(), true);
		server->send(302, FPSTR(mimePlain), ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
		server->client().stop();
		return;
	}

	server->send(404, FPSTR(mimePlain), "");
	server->client().stop();
}

bool ConfigManager::wifiConnected() {
	Serial.printf("sizeof(MagicHeader) %d, sizeof(wifiDetails), %d\r\n", sizeof(magicHeader), sizeof(wifiDetails));
	Serial.print(F("Waiting for WiFi to connect"));

	int i = 0;
	while (i < wifiConnectRetries) {
		if (WiFi.status() == WL_CONNECTED) {
			Serial.println("");
			return true;
		}

		Serial.print(".");

		delay(wifiConnectInterval);
		i++;
	}

	Serial.println("");
	Serial.println(F("Connection timed out"));

	return false;
}

void ConfigManager::setup(const uint32_t *hostnamePostfix) {

	Serial.println(F("Reading saved configuration"));

	magicHeaderT header;

	EEPROM.get(0, header);

	Serial.println("Setup =======");
	Serial.printf("header: magic {%c,%c} version %x.%x\r\n", header.magicBytes[0], header.magicBytes[1], header.majorVersion, header.minorVersion);

	if (memcmp(&header, &magicHeader, sizeof(magicHeader)) == 0) {
		wifiDetails details;
		EEPROM.get(sizeof(magicHeader), details);

		// add "_<ChipID>" to hostname
		if (hostnamePostfix != NULL) {
			char hostname[strlen(details.hostname) + 13];	// 6 hex (=12) + 1 \0 = 13
			Serial.printf("chipID: %06X, hostname: %s, len: %d\n\r", *hostnamePostfix, hostname, strlen(hostname));
			snprintf(hostname, sizeof(hostname), "%s_%0X\0", details.hostname, *hostnamePostfix);
			WiFi.hostname(hostname);
		} else {
			WiFi.hostname(details.hostname);
		}

		WiFi.begin(details.ssid, details.password[0] == '\0' ? NULL : details.password);

		if (wifiConnected()) {
			readConfig();
			Serial.printf("Connected to %s as %s (%s)\r\n", details.ssid, WiFi.hostname().c_str(), WiFi.localIP().toString().c_str());

			WiFi.mode(WIFI_STA);
			startApi();
			return;
		} else {
			Serial.printf("Failed to connect to %s\r\n, ", details.ssid);
		}
	} else {
		Serial.printf("current version [%x.%x] does not match application version [%x.%x]\r\n", header.majorVersion, header.minorVersion, magicHeader.majorVersion,
				magicHeader.minorVersion);
		// We are at a cold start, don't bother timeing out.

		apTimeout = 0;

		startAP();
	}

	return;

}

void ConfigManager::startAP() {
	const char* headerKeys[] = { "Content-Type" };
	size_t headerKeysSize = sizeof(headerKeys) / sizeof(char*);

	mode = ap;

	Serial.println(F("Starting Access Point"));

	IPAddress ip(192, 168, 1, 1);
	WiFi.mode(WIFI_AP);
	WiFi.softAPConfig(ip, ip, IPAddress(255, 255, 255, 0));
	WiFi.softAP(apName);

	delay(500); // Need to wait to get IP

	dnsServer.reset(new DNSServer);
	dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
	dnsServer->start(DNS_PORT, "*", ip);

	server.reset(new WebServer(80));
	server->collectHeaders(headerKeys, headerKeysSize);
	server->on("/", HTTPMethod::HTTP_GET, std::bind(&ConfigManager::handleAPGet, this));
	server->on("/", HTTPMethod::HTTP_POST, std::bind(&ConfigManager::handleAPPost, this));
	server->onNotFound(std::bind(&ConfigManager::handleNotFound, this));

	if (apCallback) {
		apCallback(server.get());
	}

	server->begin();

	apStart = millis();
}

void ConfigManager::startApi() {
	const char* headerKeys[] = { "Content-Type" };
	size_t headerKeysSize = sizeof(headerKeys) / sizeof(char*);

	mode = api;

	server.reset(new WebServer(80));
	server->collectHeaders(headerKeys, headerKeysSize);
	server->on("/", HTTPMethod::HTTP_GET, std::bind(&ConfigManager::handleAPGet, this));
	server->on("/", HTTPMethod::HTTP_POST, std::bind(&ConfigManager::handleAPPost, this));
	server->on("/settings", HTTPMethod::HTTP_GET, std::bind(&ConfigManager::handleRESTGet, this));
	server->on("/settings", HTTPMethod::HTTP_PUT, std::bind(&ConfigManager::handleRESTPut, this));
	server->onNotFound(std::bind(&ConfigManager::handleNotFound, this));

	if (apiCallback) {
		apiCallback(server.get());
	}

	server->begin();
}

void ConfigManager::readConfig() {
	byte *ptr = (byte *) config;

	for (int i = 0; i < configSize; i++) {
		ptr[i] = EEPROM.read(CONFIG_OFFSET + i);
	}
}

void ConfigManager::writeConfig() {
	byte *ptr = (byte *) config;

	for (int i = 0; i < configSize; i++) {
		EEPROM.write(CONFIG_OFFSET + i, ptr[i]);
	}
	EEPROM.commit();
}

