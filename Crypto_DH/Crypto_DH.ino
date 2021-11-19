#include <Curve25519.h>

uint8_t clientSecretKey[32];
uint8_t clientPublicKey[32];

uint8_t serverSecretKey[32];
uint8_t serverPublicKey[32];


void generateKeys(){
  Curve25519::dh1(clientPublicKey, clientSecretKey);
  Curve25519::dh1(serverPublicKey, serverSecretKey);
}

void setup() {
  Serial.begin(9600);
  generateKeys();

  Serial.println("\n");
  
  printHex("Client Private: ", clientSecretKey, 32);
  printHex("Server Private: ", serverSecretKey, 32);
  
  printHex("Client Pub: ", clientPublicKey, 32);
  printHex("Server Pub: ", serverPublicKey, 32);

  boolean clientShared = Curve25519::dh2(serverPublicKey, clientSecretKey);
  boolean serverShared = Curve25519::dh2(clientPublicKey, serverSecretKey);
  
  Serial.println("\n");
 
  printHex("Client Shared: ", clientPublicKey, 32);
  printHex("Server Shared: ", serverPublicKey, 32);
  
}

void loop() {
  // put your main code here, to run repeatedly:

}

void printHex(const char *label, uint8_t *data, int len) {
    Serial.printf("%s:\n", label );

    for ( int i = 0; i < len; i++ ) {
        Serial.printf("%02X " , data[i] );
    }
    Serial.printf("\n\n");
}
