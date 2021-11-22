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
 
  if (client) {
    Serial.println("Client Connected");
    while (client.connected()) {
      while (client.available()>0) {
        String req = client.readStringUntil('\r');
        Serial.println(req);
        
        client.write("A");
      }
 
      delay(10);
    }
 
    client.stop();
    Serial.println("Client disconnected");
 
  }
}
