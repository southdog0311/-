#include <map>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <U8g2lib.h>
#include "weatherFont.h"
#include <switch.h>          // 按鈕開關
#define BITS 10

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

std::map<String, char> icon_map{
    {"01d", 'B'}, {"02d", 'H'}, {"03d", 'N'}, {"04d", 'Y'}, {"09d", 'R'}, {"10d", 'Q'}, {"11d", 'P'}, {"13d", 'W'}, {"50d", 'J'}, {"01n", 'C'}, {"02n", 'I'}, {"03n", '5'}, {"04n", '%'}, {"09n", '8'}, {"10n", '7'}, {"11n", '6'}, {"13n", '#'}, {"50n", 'K'}};

const char *ssid = "CHT0347";
const char *password = "12345678";
String API_KEY = "db896bbfadae1cf98ae65379f99fe7f8";
String city = "Chiayi,TW";
String chinese_city_name = "";
const byte UP_SW = 13;
const byte DOWN_SW = 27;
int cur_index = 0;

HTTPClient http;

Switch upSW(UP_SW, LOW, true);
Switch downSW(DOWN_SW, LOW, true);

int LEDs[] = {25,23,4,3,19,18,32};    // for LED display
// g,f,e,d,c,b,a 
int zero[] = {0, 1, 1, 1, 1, 1, 1};
int one[] = {0, 0, 0, 0, 1, 1, 0};   // LED states to display number one
int two[] = {1, 0, 1, 1, 0, 1, 1};

void connectWiFi()
{
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nIP位址：");
  Serial.println(WiFi.localIP());
}

String openWeather() // GET weather info
{
  String url = "http://api.openweathermap.org/data/2.5/weather?q=" +
               city + "&appid=" + API_KEY;
  String payload = "";

  if ((WiFi.status() != WL_CONNECTED))
  {
    connectWiFi();
  }
  else
  {
    http.begin(url);
    int httpCode = http.GET(); //Make the request

    if (httpCode > 0)
    {
      payload = http.getString();
      Serial.printf("回應本體：%s\n", payload.c_str());
    }
    else
    {
      Serial.println("HTTP請求出錯了~");
    }

    http.end();
  }
  return payload;
}

void displayWeather(String json) // display weather on OLED
{
  DynamicJsonDocument doc(1024);

  deserializeJson(doc, json);
  JsonObject weather = doc["weather"][0];
  const char *icon = weather["icon"];
  const char *city = doc["name"];

  JsonObject main = doc["main"];
  // 取得絕對溫度，然後轉成攝氏溫度。
  float temp = (float)main["temp"] - 273.15;
  int humid = (int)main["humidity"];

  Serial.printf("天氣圖示：%s\n", icon);
  Serial.printf("攝氏溫度：%.1f\n", temp);
  
  // 在OLED螢幕顯示天氣資訊
  u8g2.firstPage();
  do
  {
    //u8g2.setFont(u8g2_font_profont12_mr);
    u8g2.setFont(u8g2_font_unifont_t_chinese1);  // 使用 chinese1字型檔
    //u8g2.drawUTF8(2, 8, "嘉義");
    u8g2.setFontDirection(0);
    u8g2.clearBuffer();
    u8g2.setCursor(2, 16);
    u8g2.print(chinese_city_name);
    u8g2.setFont(u8g2_font_inr16_mf);
    u8g2.setCursor(60, 36);
    u8g2.print(String(temp, 1) + "\xb0");
    u8g2.setCursor(60, 62);
    u8g2.print(String(humid) + "%");
    u8g2.setFont(weatherFont);
    u8g2.setCursor(2, 62);
    u8g2.print(icon_map[icon]);
  } while (u8g2.nextPage());
}

void displayLEDAndSetLocation(int index){ // 7 段式顯示器目前設定與設定城市
  switch(index){
    case 0: 
    for(int i=0; i<7; i++)  digitalWrite(LEDs[i], zero[i]);
    Serial.println("嘉義天氣");
    city = "Chiayi,TW";
    chinese_city_name = "嘉義";
    break;
    case 1:
    for(int i=0; i<7; i++)  digitalWrite(LEDs[i], one[i]);      
    Serial.println("台南天氣");
    city = "Tainan,TW";
    chinese_city_name = "台南";
    break;
    case 2:
    for(int i=0; i<7; i++)  digitalWrite(LEDs[i], two[i]);
    Serial.println("台中天氣");
    city = "Taichung,TW";
    chinese_city_name = "台中";
    break;
  }
}

void setup()
{
  Serial.begin(115200);
  connectWiFi();
  u8g2.begin();
  u8g2.enableUTF8Print();    // 啟動 UTF8 支援
  analogSetAttenuation(ADC_11db); // 設定類筆輸入電壓上限 3.6V
  analogSetWidth(BITS);           // 取樣設成10位元
  ledcSetup(0, 5000, BITS);       // 設定 PWM，通道0、KHz、10位元
  for (int i = 0; i<7; i++) pinMode(LEDs[i], OUTPUT); // setup LED display  
  displayLEDAndSetLocation(cur_index);
}

void loop()
{
  switch (upSW.check()) {    // 增加 index 選不同城市
    case Switch::RELEASED_FROM_PRESS:
    case Switch::PRESSING:
      Serial.println(cur_index);
      cur_index+=1;
      if(cur_index>2) cur_index = 2;
      displayLEDAndSetLocation(cur_index);
      break;    
  }
   switch (downSW.check()) {  // 遞減 index 選不同城市
    case Switch::RELEASED_FROM_PRESS:
    case Switch::PRESSING:
      Serial.println(cur_index);
      cur_index-=1;
      if(cur_index<0) cur_index = 0;
      displayLEDAndSetLocation(cur_index);
      break;
  }  
  String payload = openWeather();
  if (payload != "")
  {
    displayWeather(payload);
  }

  delay(2000);
}
