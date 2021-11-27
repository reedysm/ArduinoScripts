#include "WiFi.h"
#include <Arduino_JSON.h>

const char * ssid = "dormsecure";
const char * password = "wiredisfaster";
const uint ServerPort = 5679;
const uint8_t LED_PIN = 5; 
uint8_t blinkState = LOW;
WiFiServer wifiServer(ServerPort);
String username = "Dan";
String pass = "1234";

void setup() {
  initializeWifi();
  pinMode(LED_PIN, OUTPUT);
}

void initializeWifi() {
  Serial.begin(115200);
    // WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("WiFi Failed");
        while(1) {
            delay(1000);
        }
    }
    Serial.println("Connected to the WiFi network");
    Serial.println(WiFi.localIP());
 
    wifiServer.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  CheckForConnections();
  
//  if (SendTimer.TimePassed_Milliseconds(400))
//  {
//    // Send Data
//  }
}
 
void CheckForConnections()
{
  WiFiClient client = wifiServer.available();
  uint8_t u = 0;
  uint8_t p = 0;

  //this is probably terrible, looking for nonblocking way to do it
  if(!client) {
    delay(500);
    blinkState = !blinkState;
    digitalWrite(LED_PIN, blinkState);
  }
  
  if (client) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("Client Connected");
    while (client.connected()) {
      while (client.available()>0) {
        String req = client.readString();
        Serial.println(req);
        JSONVar myObject = JSON.parse(req);

        if (JSON.typeof(myObject) == "undefined") {
          Serial.println("Parsing input failed!");
          break;
        }

        if (myObject.hasOwnProperty("username")) {
          Serial.print("myObject[\"username\"] = ");
          Serial.println(myObject["username"]);
          if(myObject["username"] == username)
            u = 1;
        }

        
        if (myObject.hasOwnProperty("password")) {
          Serial.print("myObject[\"password\"] = ");
          Serial.println(myObject["password"]);
          if(myObject["password"] == pass)
            p = 1;
        }

        if (p && u){
          client.write("yes");
        }
        else{
          client.write("no");
        }

      delay(10);
      }
 
    client.stop();
    Serial.println("Client disconnected");
    digitalWrite(LED_PIN, LOW);

 
    }
  }
}
