/*
 * This ESP32 code is created by esp32io.com
 *
 * This ESP32 code is released in the public domain
 *
 * For more detail (instruction and wiring diagram), visit https://esp32io.com/tutorials/esp32-button-toggle-led
 */

#define BUTTON_PIN 16  // ESP32 pin GIOP16, which connected to button
#define LED_PIN    5  // ESP32 pin GIOP18, which connected to led

// The below are variables, which can be changed
int led_state = LOW;    // the current state of LED
int button_state;       // the current state of button
int last_button_state;  // the previous state of button


void setup() {
  Serial.begin(9600);                // initialize serial
  pinMode(BUTTON_PIN, INPUT_PULLUP); // set ESP32 pin to input pull-up mode
  pinMode(LED_PIN, OUTPUT);          // set ESP32 pin to output mode

  button_state = digitalRead(BUTTON_PIN);
}

void loop() {
  last_button_state = button_state;      // save the last state
  button_state = digitalRead(BUTTON_PIN); // read new state

  if (last_button_state == HIGH && button_state == LOW) {
     Serial.println("The button is pressed");
  
      //toggle state of LED
      led_state = !led_state;
  
      // control LED arccoding to the toggled state
      digitalWrite(LED_PIN, led_state);
      delay(200);
    }
  
}
