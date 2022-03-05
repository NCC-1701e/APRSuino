/*
  APRSuino
  by: João Faria
  https://jfaria.org
  based on an google search experience v2
  Março 2022
  Version 0.2 alpha
*/

#include <ESP8266WiFi.h>
#define LED D4 // wemos d1 mini led

char SERVER_NAME[] = "euro.aprs2.net";
#define SERVER_PORT 14580
String SERVER_PROMPT = "aprsc";
String SERVER_VERIFIED = "verified";

// define your callsign, passcode and location
#define CALLSIGN "CR7BAV-3"
#define CALLSIGN_PASS "18084"
#define LOCATION "3711.82N/00724.67W" // Rio Guadiana
#define APRS_SYMBOL "Y" // - House, P police, Y Sail boat, > Car, 
#define APRS_INFO "Teste 123"
#define VERSION "3"

#define UpdateIntervalTimer 1000 * 60 * 3// ms * seconds * minutes
#define timeOutTimer 15000
#define responseTimer 2000

unsigned long startIntervalTime;
int i = 0;

//WiFi 
char ssid[] = "WiFi";        // SSID
char pass[] = "Password";  // password

WiFiClient  client;

void setup() {
  pinMode(LED, OUTPUT);
  blink_led(250);

  // Initialize a serial connection for debug and stuff
  Serial.begin(115200);
  while (!Serial) { // this is because of the leonard board bug....
    delay(50);
  }
  Serial.println("\nHello there!");

  WiFi.mode(WIFI_STA); // this is because of the esp8266 BUG!!!!
  //WiFi.setOutputPower(20);

  scanWiFi(); // for debug only
  startWifi();

  startIntervalTime = millis() + UpdateIntervalTimer;

}

void loop() {
  
  if (WiFi.status() == WL_CONNECTED )
  {
    blink_led(500); 
    i = i + 1;
    if (i % 10 == 0) {
      Serial.print(".");
    }
    if (i % 60 == 0) {
      Serial.print(i/60);
    }

    if ( millis() - startIntervalTime > UpdateIntervalTimer )
    {
      boolean sent = false;

      if ( client.connect(SERVER_NAME, SERVER_PORT) )
      {
        Serial.println("\n>> Connecting to server...");
        blink_led(500);

        if ( server_response(&client, SERVER_PROMPT) )
        {
          blink_led(500);
          //Serial.println("Connected:");
          String auth = "";
          auth += "user ";
          auth += CALLSIGN;
          auth += " pass ";
          auth += CALLSIGN_PASS;
          auth += " vers APRSuino ";
          auth += VERSION;
          client.println(auth);
          Serial.print("$ "); Serial.println(auth);

          if ( server_response(&client, SERVER_VERIFIED) )
          {
            blink_led(500);
            //Serial.println("\nLogin ok");
            String packet = "";
            packet += CALLSIGN;
            packet += ">APRS,TCPIP*,WIDE2-2,qAR,";
            packet += CALLSIGN;
            packet += ":!";
            packet += LOCATION;
            packet += APRS_SYMBOL;
            packet += APRS_INFO;

            client.println(packet);
            Serial.print("$ "); Serial.println(packet);

            Serial.println(">> Packet sent");
            blink_led(500); blink_led(500);
            client.stop();
            sent = true;
          }  
          else
          {
            Serial.println(">> Login failed.");
          }
        }  
        else
        {
          Serial.println(">> No response from server");
        }
      }  
      else
      {
        Serial.println(">> Could not connect to server.");
      }

      if (sent == true)
      {
        startIntervalTime = millis();
        i = 0;
      }
    }
  } else {
    startWifi();
  }

}

void startWifi(void) {
  Serial.println("\n  ------------\n | Start WiFi |\n  ------------");
  blink_led(250);
  WiFi.begin(ssid, pass); // Connect to the network
  Serial.print("Connecting to "); Serial.print(ssid); Serial.println(" ...");
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    blink_led(250);
    Serial.print(++i); Serial.print('.');
  }
  Serial.print("\nConnected to "); Serial.println(ssid);
  Serial.print("IP address: "); Serial.println(WiFi.localIP());
}


void scanWiFi() {
  Serial.println("\n  ----------\n | WiFiScan |\n  ----------");
  Serial.print("Scanning... ");

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("Done!");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1); Serial.print(": "); Serial.print(WiFi.SSID(i));
      Serial.print(" ("); Serial.print(WiFi.RSSI(i)); Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
}


boolean server_response(Stream* stream, String checkResponse)
{
  int c;
  String serverStream = "";
  boolean found = false;
  boolean timeOut = false;
  unsigned long startTime;
  unsigned long stopTime = timeOutTimer;

  startTime = millis();

  delay(50);

  while ( timeOut != true )
  {
    // wait for bytes from server
    while ( !stream->available() )
    {
      if ( millis() - startTime > stopTime )
      {
        //Serial.println("\n Time out...");
        timeOut = true;
        break;
      }
      delay(2);
    }

    if ( stream->available() ) {
      c = stream->read();
      serverStream += char(c);
      Serial.print( char(c));
      if (serverStream.indexOf(checkResponse) != -1) {
        found = true;
        stopTime = responseTimer;
      }
    }

  }

  delay(500);
  return found;
}

void blink_led(int delay_ms)
{
  digitalWrite(LED, LOW); // Led ON
  delay(delay_ms);
  digitalWrite(LED, HIGH); // Led OFF
  delay(delay_ms);
}
