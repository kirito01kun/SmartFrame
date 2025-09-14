#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

AsyncWebServer server(8081);

bool armed = false;

// Simple log structure
struct Log {
  String timestamp;
  String type;
  String sensor;
};

#define MAX_LOGS 50
Log logs[MAX_LOGS];
int logCount = 0;

void addLog(const String &type, const String &sensor) {
  if (logCount >= MAX_LOGS) {
    // shift logs left
    for (int i = 1; i < MAX_LOGS; i++) logs[i-1] = logs[i];
    logCount--;
  }
  logs[logCount++] = {getTimeStamp(), type, sensor};
}

String getTimeStamp() {
  time_t now = time(nullptr);
  struct tm * timeinfo = localtime(&now);
  char buf[25];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", timeinfo);
  return String(buf);
}

void setup() {
  Serial.begin(115200);

  // Connect WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi connected!");
  Serial.println(WiFi.localIP());

  // Initial boot log
  addLog("boot", "system");

  // Status endpoint
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", "{\"ok\":true}");
  });

  // Logs endpoint
  server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request){
    StaticJsonDocument<2048> doc;
    JsonArray arr = doc.createNestedArray("logs");
    for(int i=0;i<logCount;i++){
      JsonObject obj = arr.createNestedObject();
      obj["timestamp"] = logs[i].timestamp;
      obj["type"] = logs[i].type;
      obj["sensor"] = logs[i].sensor;
    }
    doc["armed"] = armed;
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  // Arm endpoint
  server.on("/arm", HTTP_POST, [](AsyncWebServerRequest *request){
    armed = true;
    addLog("armed", "system");
    request->send(200, "application/json", "{\"ok\":true}");
  });

  // Disarm endpoint
  server.on("/disarm", HTTP_POST, [](AsyncWebServerRequest *request){
    armed = false;
    addLog("disarmed", "system");
    request->send(200, "application/json", "{\"ok\":true}");
  });

  server.begin();

  // Timer for random motion events
  xTaskCreate([](void*){
    while(true){
      if(armed){
        addLog("motion", "pir1");
      }
      vTaskDelay(10000 / portTICK_PERIOD_MS); // 10s
    }
  }, "MotionTask", 4096, NULL, 1, NULL);
}

void loop() {
  // Nothing needed here, server handles requests
}
