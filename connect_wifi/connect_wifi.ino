#define BUTTON_PIN 16 
#define LED_PIN    5  
#include "WiFi.h"
// The below are variables, which can be changed
int led_state = LOW;    // the current state of LED
int button_state;       // the current state of button
int last_button_state;  // the previous state of button
bool startConnecting = false;
const char* ssid = "dormsecure";
const char* password =  "wiredisfaster";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);                // initialize serial
  pinMode(BUTTON_PIN, INPUT_PULLUP); // set ESP32 pin to input pull-up mode
  pinMode(LED_PIN, OUTPUT);          // set ESP32 pin to output mode
  button_state = digitalRead(BUTTON_PIN);
}

void loop() {
  last_button_state = button_state;
  button_state = digitalRead(BUTTON_PIN);
  
  if (last_button_state == HIGH && button_state == LOW) {
    Serial.println("The button is pressed");
    if(!startConnecting){
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        digitalWrite(LED_PIN, HIGH);
        delay(500);
        digitalWrite(LED_PIN, LOW);
        delay(500);
      }
      digitalWrite(LED_PIN, HIGH);
      startConnecting = true;
    }
  }
}
