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

const char* ssid     = "YOUR_WLAN_SSID";
const char* password = "YOUR_WLAN_KEY";
 
ESP8266WebServer server(80);

void handle_root() {
//onewire code: begin///////////////////////////////////////////////////////////////////////////////////////////
while (1) {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
  String hc; //html content
    
  if ( !ds.search(addr)) {
    hc += "<br/>No more addresses";
    ds.reset_search();
    break;  // all sensors read, stop here and continue on webserver part
  }

  for( i = 0; i < 8; i++) {
    if (addr[i]<0x10) {hc += "0";}   //print trailing zeroes
    hc += String(addr[i], HEX);
    if(i<7)
    {  
      hc += '-';
    }
    else 
    {
      hc += ' ';
    }
    hc.toUpperCase();     //print the serial with nicwe large characters instead of mixing small chars with numbers
  }
 
  if (OneWire::crc8(addr, 7) != addr[7]) {
      hc += "CRC-is-not-valid ";
      return;
  }
 
  switch (addr[0]) {
    case 0x10:
      hc += "DS18S20 ";
      type_s = 1;
      break;
    case 0x28:
      hc += "DS18B20 ";
      type_s = 0;
      break;
    case 0x22:
      hc += "DS1822 ";
      type_s = 0;
      break;
    default:
      hc += "Device-is-not-DS18x20-family ";
      return;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);          // start conversion, with parasite power on at the end
  
  delay(1000);                // maybe 750ms is enough, maybe not

  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);             // Read Scratchpad

  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }

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
  hc += celsius;
  hc += ' ';
 
  //print CRC
  hc += (String(OneWire::crc8(data, 8), HEX));
  
  hc += "<br/>";
  server.send(200, "text/html", hc); //send data to client
}          
//onewire code: end/////////////////////////////////////////////////////////////////////////////////
  delay(100);
}
 
void setup(void)
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  server.on("/", handle_root);
  
  server.begin();
}
 
void loop(void)
{
  server.handleClient();
} 
 
