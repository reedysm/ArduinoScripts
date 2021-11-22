#include "WiFi.h"
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

uint8_t responseArray[1024];
const char* user;
const char* password;
int counter = 0;
int responseIndex = 0;
//timeout 300000 is 5 minutes in milliseconds
const int CONNECTION_TIMEOUT = 300000;
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
    SerialBT.begin("IoTLock-Steve"); 
    Serial.println("The device started, now you can pair it with bluetooth!");
    digitalWrite(LED_PIN, HIGH);
    isConnected = true;
  }
  //button was pressed
  if(isConnected){
    //if there was data sent to esp32
    if (SerialBT.available()) {
        //read character
        uint8_t response = SerialBT.read();
        // size check for buffer overflows
        if(responseIndex < 1024){
          //add character to array
          responseArray[responseIndex] = response;
          //move to next spot in array
          responseIndex++;
        }
        //if ascii code is \n which is 10
        if(response == 10){
          //print the response
          Serial.println("Received: ");
          Serial.write(&responseArray[0], responseIndex);

          //echo response back to android
          SerialBT.write(&responseArray[0], responseIndex);

          //cleanup
          responseIndex = 0;
          //free array memory
          memset(responseArray, 0, 1024);
        }
    }
    //after 5 minutes close bluetooth connections, and turn off light
    if(counter == CONNECTION_TIMEOUT){
      isConnected = false;
      Serial.println("Closing bluetooth connection!");
      SerialBT.flush();  
      SerialBT.disconnect();
      SerialBT.end();
      digitalWrite(LED_PIN, LOW);
    }
    //20 ms delay
    delay(20);

    //keep track of time
    counter+=20;
  }
}
