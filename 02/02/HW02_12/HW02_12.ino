#define BITS 10

void setup() {
  pinMode(5, OUTPUT);
  Serial.begin(115200);
  analogSetAttenuation(ADC_11db); // 設定類筆輸入電壓上限 3.6V 
  analogSetWidth(BITS);           // 取樣設成10位元
  ledcSetup(0, 5000, BITS);       // 設定 PWM，通道0、KHz、10位元
  ledcAttachPin(5, 0);            // 指定 LED 接腳5 成 PWM 輸出
}

void loop() {
  uint16_t adc = analogRead(A0); // MQ2 感測值
  adc = map(adc, 0, 255, 0, 1023); // Mapping 至 1023  值
  Serial.printf("MQ2感測值: %u\n", adc);
  ledcWrite(0, adc); // 調節燈的亮度
  delay(30);  
}
