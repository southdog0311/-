#include <U8g2lib.h>
#include <WiFi.h>
#include <time.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <WebSocketsServer.h>

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

#define BITS 10
#define LED_PIN 32

const char *ssid = "CHT0347";
const char *password = "12345678";
const uint16_t ADC_RES = 1023;

const char* ntpServer = "pool.ntp.org";
const uint16_t utcOffest = 28800;  // UTC+8偏移量
const uint8_t daylightOffset = 0;   // 夏令時間

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

String LDR_val = "0";
String LED_val = "0";
int pwm_val;

void onSocketEvent(AsyncWebSocket *server,
                   AsyncWebSocketClient *client,
                   AwsEventType type,
                   void *arg,
                   uint8_t *data,
                   size_t len)
{

  switch (type) {
    case WS_EVT_CONNECT:
      printf("來自%s的用戶%u已連線\n", client->remoteIP().toString().c_str(), client->id());
      break;
    case WS_EVT_DISCONNECT:
      printf("用戶%u已離線\n", client->id());
      break;
    case WS_EVT_ERROR:
      printf("用戶%u出錯了：%s\n", client->id(), (char *)data);
      break;
    case WS_EVT_DATA:
      printf("用戶%u傳入資料：%s\n", client->id(), (char *)data);
      const size_t capacity = JSON_OBJECT_SIZE(2) + 20;
      DynamicJsonDocument doc(capacity);
      deserializeJson(doc, data);

      const char *device = doc["device"]; // "LED"
      int val = doc["val"];               // 資料值
      if (strcmp(device, "LED") == 0)
      {
        printf("PWM: %d\n", val);
        LED_val = String(1023 - val);
        ledcWrite(0, 1023 - val); // 輸出PWM
      }
      break;
  }
}

String getCurTime(){ // get current time and prettify the format
  struct tm now;                  // 宣告「分解時間」結構變數
  if(!getLocalTime(&now)){          // 取得本地時間
    Serial.println("無法取得時間～");
    return "unable to get time";
  }
  Serial.println(&now, "%Y/%m/%d %H:%M:%S"); // 輸出格式化時間字串
  String cur_time = String(asctime(&now));
  String week = cur_time.substring(0,3);
  String month = cur_time.substring(4,7);
  String date = cur_time.substring(8,10);
  String hour = cur_time.substring(11,13);
  String mins = cur_time.substring(14,16);
  String sec = cur_time.substring(17,19);
  String year = cur_time.substring(20,24);
  String pretty_time = year + "/" + month + "/" + date + "(" + week + ")" + " " + hour + ":" + mins + ":" + sec;
  return pretty_time;
}

void displayOLED(String cur_time){
  String hourMinSec = cur_time.substring(17, 25);
  String date = cur_time.substring(0, 17);
  u8g2.firstPage();
  do
  {
  u8g2.setFont(u8g2_font_profont12_mr);
  // print out LDR device name
  u8g2.setCursor(2, 16);
  u8g2.print("LDR");
  // print out LDR device corresponding value
  u8g2.setCursor(25, 16);
  u8g2.print(LDR_val);
  // print out LED device name
  u8g2.setCursor(50, 16);
  u8g2.print("LED");
  // print out LED device value
  u8g2.setCursor(75, 16);
  u8g2.print(LED_val);
  // print out current date
  u8g2.setCursor(2, 40);
  u8g2.print(date);
  // print out current time
  u8g2.setCursor(2, 60);
  u8g2.print(hourMinSec);
  }while(u8g2.nextPage());
}

void notifyClients() {   
  String pretty_time = getCurTime();
  const size_t capacity = JSON_OBJECT_SIZE(1024);
  DynamicJsonDocument doc(capacity);

  doc["device"] = "LDR";
  doc["val"] = analogRead(A0);
  doc["update-time"] = pretty_time; 

  char data[100];
  serializeJson(doc, data);
  ws.textAll(data);
  // display LED, LDR, current time on OLED
  char tmp[20];
  itoa(doc["val"], tmp, 10);
  LDR_val = String(tmp);
  displayOLED(doc["update-time"]);
}

void setup() {
  u8g2.begin();
  u8g2.enableUTF8Print();    // 啟動 UTF8 支援
  Serial.begin(115200);
  analogSetAttenuation(ADC_11db);
  analogSetWidth(BITS);
  pinMode(LED_PIN, OUTPUT);
  ledcSetup(0, 5000, BITS);    // 設定PWM，通道0、5KHz、10位元
  ledcAttachPin(LED_PIN, 0);
  digitalWrite(LED_PIN, HIGH);

  if (!SPIFFS.begin(true)) {
    Serial.println("無法載入SPIFFS記憶體");
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  printf("\nIP位址：%s\n", WiFi.localIP().toString().c_str());

  server.serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.html");
  server.serveStatic("/favicon.ico", SPIFFS, "/www/favicon.ico");
  server.onNotFound([](AsyncWebServerRequest * req) {
    req->send(404, "text/plain", "Not found");
  });

  ws.onEvent(onSocketEvent); // 附加事件處理程式
  server.addHandler(&ws);
  server.begin();
  Serial.println("HTTP伺服器開工了～");
  configTime(utcOffest, daylightOffset, ntpServer); 
  //delay(1000);
}

void loop() {   
  ws.cleanupClients();
  notifyClients();
  delay(1000);
}
