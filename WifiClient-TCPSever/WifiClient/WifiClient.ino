#include "WiFi.h"

const char * ssid = "dormsecure";
const char * password = "wiredisfaster";
const uint ServerPort = 5679;

WiFiServer wifiServer(ServerPort);

void setup() {
  initializeWifi();

}

void initializeWifi() {
  Serial.begin(115200);
    WiFi.mode(WIFI_STA);
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
 
  if (client) {
    Serial.println("Client Connected");
    while (client.connected()) {
      while (client.available()>0) {
        String req = client.readStringUntil('\r');
        Serial.println(req);

        // Json strucutre {"username": "john123", "password": "adminpassword", "message": 0}
        // 0 means "unlock" the door, 1 means "lock" the door

        // Read the request as json. check the username and password are a match to previously saved values

        // If it they are good, read "message" portion of json
        // If 1 lock the door
        // If 0 unlock the door


        // If username and pass do not match, send back nothing

        // If light was changed, send back new state of light

        // Also need to check if the light is changed in general (like when the button click used to turn on/off light)
        // Then if it changes, check if a client is connected, if so, send back state of LED
        
        client.println("hello world");

      }
 
      delay(10);
    }
 
    client.stop();
    Serial.println("Client disconnected");
 
  }
}
