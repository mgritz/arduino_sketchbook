// Wir laden die uns schon bekannte WiFi Bibliothek
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Hier geben wir den WLAN Namen (SSID) und den Zugansschlüssel ein
// Definiert "ssid" und "password"
#include "wifipass.h"

const long utcOffsetInSeconds = 3600 * 2;

// Wir setzen den Webserver auf Port 80
WiFiServer server(80);
// NTP Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

// Eine Variable um den HTTP Request zu speichern
String header;

// Hier wird der aktuelle Status des Relais festgehalten
String output4State = "off";

// Die verwendeted GPIO Pins
// D1 = GPIO5 und D2 = GPIO4 - einfach bei Google nach "Amica Pinout" suchen  
#define output4 D4

void setup() {
  Serial.begin(115200);
  // Die definierten GPIO Pins als output definieren ...
  pinMode(output4, OUTPUT);
  // ... und erstmal auf LOW setzen
  digitalWrite(output4, LOW);

  // Per WLAN mit dem Netzwerk verbinden
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Die IP vom Webserver auf dem seriellen Monitor ausgeben
  Serial.println("");
  Serial.println("WLAN verbunden.");
  Serial.println("IP Adresse: ");
  Serial.println(WiFi.localIP());
  server.begin();
  timeClient.begin();
}

void loop(){
  WiFiClient client = server.available();   // Hört auf Anfragen von Clients

  if (client) {                             // Falls sich ein neuer Client verbindet,
    Serial.println("Neuer Client.");          // Ausgabe auf den seriellen Monitor
    String currentLine = "";                // erstelle einen String mit den eingehenden Daten vom Client
    while (client.connected()) {            // wiederholen so lange der Client verbunden ist
      if (client.available()) {             // Fall ein Byte zum lesen da ist,
        char c = client.read();             // lese das Byte, und dann
        Serial.write(c);                    // gebe es auf dem seriellen Monitor aus
        header += c;
        if (c == '\n') {                    // wenn das Byte eine Neue-Zeile Char ist
          // wenn die aktuelle Zeile leer ist, kamen 2 in folge.
          // dies ist das Ende der HTTP-Anfrage vom Client, also senden wir eine Antwort:
          if (currentLine.length() == 0) {
            // HTTP-Header fangen immer mit einem Response-Code an (z.B. HTTP/1.1 200 OK)
            // gefolgt vom Content-Type damit der Client weiss was folgt, gefolgt von einer Leerzeile:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // Hier werden die GPIO Pins ein- oder ausgeschaltet
            if (header.indexOf("GET /4/on") >= 0) {
              Serial.println("GPIO 4 on");
              output4State = "on";
              digitalWrite(output4, HIGH);
            } else if (header.indexOf("GET /4/off") >= 0) {
              Serial.println("GPIO 4 off");
              output4State = "off";
              digitalWrite(output4, LOW);
            }
            
            // Hier wird nun die HTML Seite angezeigt:
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // Es folgen der CSS-Code um die Ein/Aus Buttons zu gestalten
            // Hier können Sie die Hintergrundfarge (background-color) und Schriftgröße (font-size) anpassen
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #333344; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #888899;}</style></head>");
            
            // Webseiten-Überschrift
            client.println("<body><h1>ESP8266 Web Server</h1>");
            
            // Zeige den aktuellen Status, und AN/AUS Buttons for GPIO 4
            client.println("<p>GPIO 4 - State " + output4State + "</p>");
            // Wenn output4State = off, zeige den EIN Button       
            if (output4State=="off") {
              client.println("<p><a href=\"/4/on\"><button class=\"button\">EIN</button></a></p>");
            } else {
              client.println("<p><a href=\"/4/off\"><button class=\"button button2\">AUS</button></a></p>");
            }

            // Zeige Analogspannung
            const int analogPin = A0;
            const int analogVal = analogRead(analogPin);
            client.println(analogVal);

            // Zeige Uhrzeit
            timeClient.update();
            client.println(timeClient.getFormattedTime());
            
            client.println("</body></html>");
            
            // Die HTTP-Antwort wird mit einer Leerzeile beendet
            client.println();
            // und wir verlassen mit einem break die Schleife
            break;
          } else { // falls eine neue Zeile kommt, lösche die aktuelle Zeile
            currentLine = "";
          }
        } else if (c != '\r') {  // wenn etwas kommt was kein Zeilenumbruch ist,
          currentLine += c;      // füge es am Ende von currentLine an
        }
      }
    }
    // Die Header-Variable für den nächsten Durchlauf löschen
    header = "";
    // Die Verbindung schließen
    client.stop();
    Serial.println("Client getrennt.");
    Serial.println("");
  }
}
