/* ESP8266-1 DS18B20 Webserver with DHCP (puzzled together by code snippets from the web)

   Wiring:  
   DS18B20 pin1: gnd
           pin2: ESP8266 pin2, pullup resistor 47k to VCC (pullup min. 4.7k)
           pin3: VCC
    ___
   (123) DS18B20 pinout bottom view

   Note: programming the esp8266-1 was possible just with +5V VCC, the esp8266-1 becomes very hot
   to run the webserver +3.3V is enought, here the esp8266-1 doenst get that hot. i recommend strongly
   to run the esp8266-1 modules as short as possible at +5V !
*/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <OneWire.h>

OneWire ds(2);  // an pin 2

const char* ssid     = "intranet";
const char* password = "2499203400703253";
 
ESP8266WebServer server(80);

void handle_root() {
//////////////////////////////////////////////////////////////////////////////////////////////////1wire-code
while (1) {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
  String hc; //html content
    
  if ( !ds.search(addr)) {
//    Serial.println("No more addresses.");
    hc += "<br/>No more addresses";
//    Serial.println();
    ds.reset_search();
    //delay(250);
    //return;
    break;  // all sensors read, stop here and continue on webserver part
  }
  
  //Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
//    if (addr[i]<0x10) {Serial.print("0");}   //print trailing zeroes
    if (addr[i]<0x10) {hc += "0";}   //print trailing zeroes
//    Serial.print(addr[i], HEX);
    hc += String(addr[i], HEX);
    if(i<7)
    {  
//      Serial.write('-');  //separator between serial no. digits
      hc += '-';
    }
    else 
    {
//      Serial.write(' ');  // no separator after last digit
      hc += ' ';
    }
    
    hc.toUpperCase();     //print the serial with nicwe large characters instead of mixing small chars with numbers
  }
 
  if (OneWire::crc8(addr, 7) != addr[7]) {
//      Serial.print("CRC-is-not-valid ");
      hc += "CRC-is-not-valid ";
      return;
  }
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
//      Serial.print("DS18S20 ");  // or old DS1820
      hc += "DS18S20 ";
      type_s = 1;
      break;
    case 0x28:
//      Serial.print("DS18B20 ");
      hc += "DS18B20 ";
      type_s = 0;
      break;
    case 0x22:
//      Serial.print("DS1822 ");
      hc += "DS1822 ";
      type_s = 0;
      break;
    default:
//      Serial.print("Device-is-not-DS18x20-family ");
      hc += "Device-is-not-DS18x20-family ";
      return;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
//  Serial.print(celsius);
    
  hc += celsius;

//  Serial.print (" ");
  hc += ' ';
 
   //print CRC
//  Serial.print(OneWire::crc8(data, 8), HEX);
  hc += (String(OneWire::crc8(data, 8), HEX));
  
  hc += "<br/>";
  server.send(200, "text/html", hc); 

}          
//////////////////////////////////////////////////////////////////////////////////////////////////1wire-code-end
  delay(100);
}
 
void setup(void)
{
  // You can open the Arduino IDE Serial Monitor window to see what the code is doing
//  Serial.begin(115200);  // Serial connection from ESP-01 via 3.3v console cable
  //dht.begin();           // initialize temperature sensor
 
  // Connect to WiFi network
  WiFi.begin(ssid, password);
//  Serial.print("\n\r \n\rWorking to connect");
 
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
//    Serial.print(".");
  }
//  Serial.println("");
//  Serial.println("DHT Weather Reading Server");
//  Serial.print("Connected to ");
//  Serial.println(ssid);
//  Serial.print("IP address: ");
//  Serial.println(WiFi.localIP());
   
  server.on("/", handle_root);
  
  server.begin();
//  Serial.println("HTTP server started");
}
 
void loop(void)
{
  server.handleClient();
} 
 
