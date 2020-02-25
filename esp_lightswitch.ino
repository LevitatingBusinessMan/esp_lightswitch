#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define buttonPin 5
#define ledPin 2

#define timeout 500

#define STASSID "Fernhout Rein"
#define STAPSK  "****"

const char* ssid     = STASSID;
const char* password = STAPSK;

WiFiUDP Udp;
WiFiClient client;

//Array of the last byte of an IP
unsigned char ipArray[16] = {0};

char discoveryPacket[] = 
  "M-SEARCH * HTTP/1.1\r\n"
  "HOST: 239.255.255.250:1982\r\n"
  "MAN: \"ssdp:discover\"\r\n"
  "ST: wifi_bulb\r\n";

unsigned long pressTime;

void setup() {

  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  
  Serial.begin(9600);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  //Blink led when connected
  digitalWrite(ledPin, HIGH);
  delay(300);
  digitalWrite(ledPin, LOW);
  delay(300);
  digitalWrite(ledPin, HIGH);
  delay(300);
  digitalWrite(ledPin, LOW);
  delay(300);
  digitalWrite(ledPin, HIGH);
  
}

void loop() {

  //Stop listening after a second, then listen for button press again
  if (millis() > pressTime + timeout) {

    //When timeout is reached for first time
    if (pressTime > 0) {
      //Close server
      Udp.stop();
      //Set IPArray back to 0's
      for (int i = 0; i < 16; i++)
        ipArray[i] = 0;

      Serial.println();
    }
    pressTime = 0;
    
  
    if (digitalRead(buttonPin) == HIGH) {
      pressTime = millis();
      Serial.println("Detected button press");
      digitalWrite(ledPin, LOW);
  
      Udp.begin(1982);
      Udp.beginPacket("239.255.255.250", 1982);
      Udp.write(discoveryPacket);
      Udp.endPacket();
         
    } else {
      digitalWrite(ledPin, HIGH);
    }
  
  
  }

  //Listen for packets
  else {
    int packetSize = Udp.parsePacket();
    if (packetSize)
    {
      Serial.printf("Received packet from: %s\n", Udp.remoteIP().toString().c_str());

      //Check if we handled this IP already
      for (int i = 0; i < 16; i++) {
        if (ipArray[i] == Udp.remoteIP()[3])
          return;
        else if (ipArray[i] == 0) {
           ipArray[i] = Udp.remoteIP()[3];
           break;
        }
        
      }
      
      //Connect to light
      if (!client.connect(Udp.remoteIP(), 55443)) {
        Serial.println("Connection failed");
        return;
      }
  
      //Send toggle command
      client.println("{ \"id\": 0, \"method\": \"toggle\", \"params\":[]}");
      client.stop();
      Serial.printf("Toggled %s\n", Udp.remoteIP().toString().c_str());
      
    }
  }

}
