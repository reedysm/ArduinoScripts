#include "WiFi.h"
#include "BluetoothSerial.h"
#include <Arduino_JSON.h>
#define BUTTON_PIN 16 
#define LED_PIN    5  
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

uint8_t led_state = LOW;    // the current state of LED
uint8_t button_state;       // the current state of button
uint8_t last_button_state;  // the previous state of button
bool isBluetoothOn = false;
bool isWIFIOn = false;
uint8_t responseArray[1024];
String user;
String password;
String deviceName = "IoTLock-Steve";
int counter = 0;
int responseIndex = 0;

const uint ServerPort = 5679;
WiFiServer wifiServer(ServerPort);
String ipAddress = "";

//timeout 300000 is 5 minutes in milliseconds
//timeout 30000 is 1 minute in ms
const int CONNECTION_TIMEOUT = 300000;
BluetoothSerial SerialBT;

/********************************************************
*                      Utility                          *
********************************************************/

JSONVar parseStringtoJSON(String s){
  JSONVar j = JSON.parse(s);
  if(JSON.typeof(j) == "undefined"){
    Serial.println("Parsing input failed!");
    return JSON.parse("{}");
  }
  return j;
}


String parseResponseToString(){
    Serial.println("\nReceived Data: ");
    String resp = uintConvertToString(responseArray, responseIndex);
    //print to serial
    Serial.println(resp);
    
    return resp;
    //send string
//    SerialBT.print(resp);

    //old code
    //response where each index contains ascii of character
//    Serial.write(&responseArray[0], responseIndex);
    //echo response back to android
//    SerialBT.write(&responseArray[0], responseIndex);
}


String uintConvertToString(uint8_t* a, int arraySize){
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

/********************************************************
*                     Bluetooth                         *
********************************************************/

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


void sendResponseToBluetooth(String message){
  JSONVar myObject = parseStringtoJSON(message);

  bool ssidGiven = 0;
  bool passGiven = 0;
  
  if (myObject.hasOwnProperty("username")) {
    user = myObject["username"];
    Serial.print("\nUsername set");
  }

  
  if (myObject.hasOwnProperty("password")) {
    password = myObject["password"];
    Serial.print("\nPassword set");
  }
  char wifiName [64];
  char wifiPassword [64]; 
  if (myObject.hasOwnProperty("ssid")) {
      String wn;
      wn = myObject["ssid"];
      wn.toCharArray(wifiName, wn.length()+1);
      Serial.print("\nssid set");
      ssidGiven = 1;
    }
   if (myObject.hasOwnProperty("ssidPassword")) {
        String wp;
       wp = myObject["ssidPassword"];
       wp.toCharArray(wifiPassword, wp.length()+1);
      Serial.print("\nssid password set");
      passGiven = 1;
    }

    if(passGiven && ssidGiven){
      Serial.println(wifiName);
      Serial.println(wifiPassword);
      if(initializeWifi(wifiName, wifiPassword)){
        uint8_t ledValue = digitalRead(BUTTON_PIN);
        String messageResponse = "{\"name\": \"" + deviceName +  "\", \"ipAddress\": \"" + ipAddress + "\", \"initialState\": "+ (String) ledValue + "}";
        SerialBT.println(messageResponse);
        Serial.print("\nWifi Connected - Sent response");
      }else{
        String messageResponse = "error";
        SerialBT.println(messageResponse);
        Serial.print("\nWifi Not Connected - Sent response");
      }
    }
   
   
  
    
//  String messageResponse = "{recieved_user:  test, recieved_password: alsotest}";
 
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


void endBluetoothConnection(){
  isBluetoothOn = false;
    Serial.println("Closing bluetooth connection!");
    SerialBT.flush();  
    SerialBT.disconnect();
    SerialBT.end();
    digitalWrite(LED_PIN, LOW);
}

/********************************************************
*                        Wi-Fi                          *
********************************************************/

bool initializeWifi(char * ssid, char * ssidPassword) {
    // WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, ssidPassword);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("WiFi Failed");
        return false;
    }
    Serial.println("\nConnected to the WiFi network");
    Serial.println(WiFi.localIP());
    ipAddress = (String) WiFi.localIP().toString();
    wifiServer.begin();
    isWIFIOn = true;
    return isWIFIOn;
}


//connects, handles request, kills connection
void handleConnections(){
  WiFiClient androidClient = wifiServer.available();

  if(androidClient){
    while(androidClient.connected()){
      while(androidClient.available() > 0){
        //JSONVar jReq = parseStringtoJSON(androidClient.readStringUntil('\r'));
        String req = androidClient.readStringUntil('\r');
        JSONVar myObject = JSON.parse(req);
        Serial.println(req);
        String reqUser;
        reqUser = myObject["username"];
        String reqPassword;
        reqPassword = myObject["password"];
        int message;
        message = myObject["message"];
        
        handleRequest(reqUser, reqPassword, message, androidClient);
      }
    }
  }
}


//Checks username, password, and handles message if correct
void handleRequest(String reqUser, String reqPass, int message, WiFiClient androidClient){
  if(reqUser == user && reqPass == password){
    switch(message){
      case 0:
        digitalWrite(LED_PIN, message);
        androidClient.println(digitalRead(LED_PIN));  
        break;
      case 1: 
        digitalWrite(LED_PIN, message);
        androidClient.println(digitalRead(LED_PIN));  
        break;
      default:
        androidClient.println(digitalRead(LED_PIN));
        return;
    }
  }
}
/********************************************************
*                        Main                           *
********************************************************/

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
    SerialBT.begin(deviceName); 
    Serial.println("The device started, now you can pair it with bluetooth!");
    isBluetoothOn = true;
  }
  //button was pressed
  if(isBluetoothOn){
    handleBluetooth();
  }
  //WiFi is initialized
  if(isWIFIOn){
    handleConnections();
  }
}
