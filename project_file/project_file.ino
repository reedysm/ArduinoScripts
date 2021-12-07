#include "WiFi.h"
#include "BluetoothSerial.h"
#include <Arduino_JSON.h>
#define BUTTON_PIN 16 
#define LED_PIN    5  
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

//global variables
//initial states
uint8_t led_state = LOW;    // the current state of LED
uint8_t button_state;       // the current state of button
uint8_t last_button_state;  // the previous state of button
bool isBluetoothOn = false;
bool isWIFIOn = false;
uint8_t responseArray[1024];
//empty user and password fields
String user;
String password;
//device name
String deviceName = "IoTLock-Steve";
//bluetooth counter
int counter = 0;
int responseIndex = 0;

//esp32 server port
const uint ServerPort = 5679;
//create wifi server variable
WiFiServer wifiServer(ServerPort);

//empty IP address variable
String ipAddress = "";

//timeout 300000 is 5 minutes in milliseconds
//timeout 30000 is 1 minute in ms
const int CONNECTION_TIMEOUT = 300000;
BluetoothSerial SerialBT;

/********************************************************
*                      Utility                          *
********************************************************/
//return JSON object of string
JSONVar parseStringtoJSON(String s){
  JSONVar j = JSON.parse(s);
  if(JSON.typeof(j) == "undefined"){
    Serial.println("Parsing input failed!");
    return JSON.parse("{}");
  }
  return j;
}

//convert ascii array to string
String parseResponseToString(){
    Serial.println("\nReceived Data: ");
    String resp = uintConvertToString(responseArray, responseIndex);
    //print to serial
    Serial.println(resp);
    return resp;
}


String uintConvertToString(uint8_t* a, int arraySize){
    int i;
    String s = "";
    //iterate through array and convert ascii character to char
    for (i = 0; i < arraySize; i++) {
        s = s + (char) a[i];
    }
    return s;
}

//used to free up buffers when bluetooth connection is done
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
  //after 5 minutes close bluetooth connections
  if(counter == CONNECTION_TIMEOUT){
    endBluetoothConnection();
  }
  //20 ms delay
  delay(20);

  //keep track of time
  counter+=20;
}


void sendResponseToBluetooth(String message){
  //obtain json object from string
  JSONVar myObject = parseStringtoJSON(message);

  //variables to check if ssid name and password was given
  bool ssidGiven = 0;
  bool passGiven = 0;

  //if username was given
  if (myObject.hasOwnProperty("username")) {
    //assign to username variable
    user = myObject["username"];
    Serial.print("\nUsername set");
  }

  //if password field was given  
  if (myObject.hasOwnProperty("password")) {
    //assign password variable
    password = myObject["password"];
    Serial.print("\nPassword set");
  }
  //buffer to hold wifi name and password
  char wifiName [64];
  char wifiPassword [64]; 
  if (myObject.hasOwnProperty("ssid")) {
      //variable to hold wifiname string
      String wn;
      //assign recieved wifi name to variable
      wn = myObject["ssid"];
      //convert string to char array and assign it to wifiName variable
      wn.toCharArray(wifiName, wn.length()+1);
      Serial.print("\nssid set");
      //set flag to true
      ssidGiven = 1;
    }
   if (myObject.hasOwnProperty("ssidPassword")) {
      //create wifi password variable
        String wp;
        //assign recieved password string to variable
       wp = myObject["ssidPassword"];
       //convert wp to char array and assign it to the wifiPassword variable
       wp.toCharArray(wifiPassword, wp.length()+1);
      Serial.print("\nssid password set");
      //set pass given to true
      passGiven = 1;
    }

    //if wifi name and pass given
    if(passGiven && ssidGiven){
      Serial.println(wifiName);
      Serial.println(wifiPassword);
      //begin to initialize wifi
      if(initializeWifi(wifiName, wifiPassword)){
        //if wifi initialized
        //obtain led state
        uint8_t ledValue = digitalRead(BUTTON_PIN);
        //respond with led state, device name, and ip address
        String messageResponse = "{\"name\": \"" + deviceName +  "\", \"ipAddress\": \"" + ipAddress + "\", \"initialState\": "+ (String) ledValue + "}";
        //send response over bluetooth
        SerialBT.println(messageResponse);
        Serial.print("\nWifi Connected - Sent response");
      }else{
        //if wifi could not be initialized, send error over bluetooth.
        String messageResponse = "error";
        SerialBT.println(messageResponse);
        Serial.print("\nWifi Not Connected - Sent response");
      }
    }
}


void readData(){
    //read a character
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
    //close bluetooth connections
    isBluetoothOn = false;
    counter = 0;
    Serial.println("Closing bluetooth connection!");
    SerialBT.flush();  
    SerialBT.disconnect();
    SerialBT.end();
}

/********************************************************
*                        Wi-Fi                          *
********************************************************/

bool initializeWifi(char * ssid, char * ssidPassword) {
    // WiFi.mode(WIFI_STA);
    //begin connection to wifi
    WiFi.begin(ssid, ssidPassword);
    //if connection failed return false
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("WiFi Failed");
        return false;
    }
    //if succeeded assign IP and begin wifiServer
    Serial.println("\nConnected to the WiFi network");
    Serial.println(WiFi.localIP());
    ipAddress = (String) WiFi.localIP().toString();
    wifiServer.begin();
    isWIFIOn = true;
    return isWIFIOn;
}


//connects, handles request, kills connection
void handleConnections(){
  //create android client
  WiFiClient androidClient = wifiServer.available();
  //if android client is connetion
  if(androidClient){
    //while android client connected
    while(androidClient.connected()){
      //while android client is available
      while(androidClient.available() > 0){
        //read string from client until r character
        String req = androidClient.readStringUntil('\r');
        //get json object from string
        JSONVar myObject = JSON.parse(req);
        Serial.println(req);
        //create and assign client requested username for authentication
        String reqUser;
        reqUser = myObject["username"];
        //create and assign client requested password for authentication
        String reqPassword;
        reqPassword = myObject["password"];
        //create and assign message variable
        int message;
        message = myObject["message"];
        //pass data to handle the request
        handleRequest(reqUser, reqPassword, message, androidClient);
      }
    }
  }
}


//Checks username, password, and handles message if correct
void handleRequest(String reqUser, String reqPass, int message, WiFiClient androidClient){
  //if the username and password match then the client is authenticated
  if(reqUser == user && reqPass == password){
    //based off message
    switch(message){
      case 0:
        //turn off light
        digitalWrite(LED_PIN, message);
        androidClient.println(digitalRead(LED_PIN));  
        break;
      case 1: 
        //turn on light
        digitalWrite(LED_PIN, message);
        androidClient.println(digitalRead(LED_PIN));  
        break;
      default:
        //send status of light
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
  button_state = digitalRead(BUTTON_PIN); //read initial button state
  
}

void loop() {
  //get old button state
  last_button_state = button_state;
  //read new button state
  button_state = digitalRead(BUTTON_PIN);

  //if button is pressed
  if (last_button_state == HIGH && button_state == LOW) {
    //begin bluetooth broadcast with device name
    SerialBT.begin(deviceName); 
    Serial.println("The device started, now you can pair it with bluetooth!");
    //set bluetooth flag to true
    isBluetoothOn = true;
  }
  //bluetooth is on
  if(isBluetoothOn){
    //handle bluetooth logic
    handleBluetooth();
  }
  //is WiFi is setup
  if(isWIFIOn){
    //handle wifi logic
    handleConnections();
  }
}
