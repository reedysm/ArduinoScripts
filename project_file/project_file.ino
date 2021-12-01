#include "WiFi.h"
#include "BluetoothSerial.h"
#include <Arduino_JSON.h>
#define BUTTON_PIN 16 
#define LED_PIN    5  
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

int led_state = LOW;    // the current state of LED
int button_state;       // the current state of button
int last_button_state;  // the previous state of button
bool isBluetoothOn = false;

uint8_t responseArray[1024];
String user;
String password;
String deviceName = "IoTLock-Steve";
int counter = 0;
int responseIndex = 0;

const uint ServerPort = 5679;
WiFiServer wifiServer(ServerPort);
const char * ssid = "reedy-home";
const char * wifiPassword = "uOzb1rmjc1";
String ipAddress = "";

//timeout 300000 is 5 minutes in milliseconds
//timeout 30000 is 1 minute in ms
const int CONNECTION_TIMEOUT = 30000;
BluetoothSerial SerialBT;


void initializeWifi() {
    // WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, wifiPassword);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("WiFi Failed");
        while(1) {
            delay(1000);
        }
    }
    Serial.println("\nConnected to the WiFi network");
    Serial.println(WiFi.localIP());
    ipAddress = (String) WiFi.localIP().toString();
    wifiServer.begin();
}

void handleBluetooth(){
  //if there was data sent to esp32
  if (SerialBT.available()) {
      readData();
  }
  //after 5 minutes close bluetooth connections, and turn off light
  if(counter == CONNECTION_TIMEOUT){
    endBluetoothConnection();
  }
  //20 ms delay
  delay(20);

  //keep track of time
  counter+=20;
}

String parseResponseToString(){
    Serial.println("\nReceived Data: ");
    String resp = uintConvertToString(responseArray, responseIndex);
    //print to serial
    Serial.println(resp);
    
    return resp;
    //send string
    //SerialBT.print(resp);

    //old code
    //response where each index contains ascii of character
    //Serial.write(&responseArray[0], responseIndex);
    //echo response back to android
    //SerialBT.write(&responseArray[0], responseIndex);
    
    
}

String uintConvertToString(uint8_t* a, int arraySize)
{
    int i;
    String s = "";
    for (i = 0; i < arraySize; i++) {
        s = s + (char) a[i];
    }
    return s;
}

void freeArrays(){
  //cleanup
    responseIndex = 0;
    //free array memory
    memset(responseArray, 0, 1024);
}

void endBluetoothConnection(){
  isBluetoothOn = false;
    Serial.println("Closing bluetooth connection!");
    SerialBT.flush();  
    SerialBT.disconnect();
    SerialBT.end();
    digitalWrite(LED_PIN, LOW);
}

void sendResponseToBluetooth(String message){
  uint8_t u = 0;
  uint8_t p = 0;
  JSONVar myObject = JSON.parse(message);

  if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
  }

  if (myObject.hasOwnProperty("username")) {
    //Serial.print("myObject[\"username\"] = ");
    //Serial.println(myObject["username"]);
    user = myObject["username"];
    Serial.print("\nUsername set");
  }

  
  if (myObject.hasOwnProperty("password")) {
    //Serial.print("myObject[\"password\"] = ");
    //Serial.println(myObject["password"]);
    password = myObject["password"];
    Serial.print("\nPassword set");

  }
  int ledValue = digitalRead(BUTTON_PIN);
  String messageResponse = "{\"name\": \"" + deviceName +  "\", \"ipAddress\": \"" + ipAddress + "\", \"initialState\": " + (String) ledValue +  "}";
  SerialBT.println(messageResponse);
  Serial.print("\nSent response");
  //String messageResponse = "{recieved_user:  test, recieved_password: alsotest}";
 
}

void readData(){
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
      String jsonMessage = parseResponseToString();
      sendResponseToBluetooth(jsonMessage);
      //send response
      freeArrays(); 
    }
    
  }

void setup() {
  Serial.begin(9600);               // initialize serial
  pinMode(BUTTON_PIN, INPUT_PULLUP); // set ESP32 pin to input pull-up mode
  pinMode(LED_PIN, OUTPUT);          // set ESP32 pin to output mode
  button_state = digitalRead(BUTTON_PIN);
  initializeWifi();
}

void loop() {
  last_button_state = button_state;
  button_state = digitalRead(BUTTON_PIN);
  
  if (last_button_state == HIGH && button_state == LOW) {
    SerialBT.begin(deviceName); 
    Serial.println("The device started, now you can pair it with bluetooth!");
    digitalWrite(LED_PIN, HIGH);
    isBluetoothOn = true;
  }
  //button was pressed
  if(isBluetoothOn){
    handleBluetooth();
  }

  
  
}
