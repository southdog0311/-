void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  pinMode(16, OUTPUT);  
  Serial.println("Start touch work");
}

void alarm(){ // shine two LEDs 
  digitalWrite(2, HIGH);
  delay(50);
  digitalWrite(16, LOW);
  delay(50);
  digitalWrite(2, LOW);
  delay(50);
  digitalWrite(16, HIGH);
  delay(50);
}

void loop() {  
  Serial.println(touchRead(T3)); // print out resistance number
  if(touchRead(T3) < 40){ // trigger alarm() when touch the wire
    Serial.println("detected hand");
    alarm();
  }
  else{ // turn off light
    Serial.println("clear");
    digitalWrite(2, LOW);
    digitalWrite(16, LOW);
  }
}
