#include "BluetoothSerial.h"
#define BUTTON_PIN 16 
#define LED_PIN    5  
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
int led_state = LOW;    // the current state of LED
int button_state;       // the current state of button
int last_button_state;  // the previous state of button
bool isConnected = false;

BluetoothSerial SerialBT;

void setup() {
  Serial.begin(9600);               // initialize serial
  pinMode(BUTTON_PIN, INPUT_PULLUP); // set ESP32 pin to input pull-up mode
  pinMode(LED_PIN, OUTPUT);          // set ESP32 pin to output mode
  button_state = digitalRead(BUTTON_PIN);
  
}

void loop() {
  last_button_state = button_state;
  button_state = digitalRead(BUTTON_PIN);
  
  if (last_button_state == HIGH && button_state == LOW) {
    SerialBT.begin("ESP32test"); 
    Serial.println("The device started, now you can pair it with bluetooth!");
    digitalWrite(LED_PIN, HIGH);
    isConnected = true;
  }
  if(isConnected){
    if (Serial.available()) {
      SerialBT.write(Serial.read());
    }
    if (SerialBT.available()) {
      Serial.write(SerialBT.read());
    }
    delay(20);
  }
}
