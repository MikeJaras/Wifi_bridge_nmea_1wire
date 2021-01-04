// ESP32 WiFi <-> 3x UART Bridge
// by AlphaLima
// www.LK8000.com
// modified by MikeJaras 2020
//  1-wire temperature added 2021


#include "config.h"
#include <esp_wifi.h>
#include <WiFi.h>

#ifdef ONEWIRE
#include <stdio.h>
#include <string.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#endif

#ifdef BLUETOOTH
#include <BluetoothSerial.h>
BluetoothSerial SerialBT; 
#endif

#ifdef OTA_HANDLER  
#include <ArduinoOTA.h> 
#endif // OTA_HANDLER

HardwareSerial Serial_one(1);
HardwareSerial Serial_two(2);
HardwareSerial* COM[NUM_COM] = {&Serial, &Serial_one , &Serial_two};

#define MAX_NMEA_CLIENTS 4
#ifdef PROTOCOL_TCP
#include <WiFiClient.h>
WiFiServer server_0(SERIAL0_TCP_PORT);
WiFiServer server_1(SERIAL1_TCP_PORT);
WiFiServer server_2(SERIAL2_TCP_PORT);
WiFiServer *server[NUM_COM]={&server_0,&server_1,&server_2};
WiFiClient TCPClient[NUM_COM][MAX_NMEA_CLIENTS];
#endif


uint8_t buf1[NUM_COM][bufferSize];
uint16_t i1[NUM_COM]={0,0,0};

uint8_t buf2[NUM_COM][bufferSize];
uint16_t i2[NUM_COM]={0,0,0};

uint8_t buf3[NUM_COM][bufferSize];
uint16_t i3[NUM_COM]={0,0,0};

uint8_t BTbuf[bufferSize];
uint16_t iBT =0;


#ifdef ONEWIRE 
const int oneWireBus = 4;                 // GPIO where the DS18B20 is connected to
#define led 13                            // define led-pin for blink 
unsigned long myTime;
unsigned long previousMillis = 0;         // will store last time LED was updated
unsigned long currentMillis = 0;
const long interval = 1000;               // interval at which to blink (milliseconds) and send frequesensy to wifi and bt
unsigned long previousMillis_2 = 0;       // will store last time temperature was updated
unsigned long currentMillis_2 = 0;
const long interval_2 = 15000;            // interval at which temperature is read (milliseconds)
int ledState = LOW;                       // ledState used to set the LED
bool readState = true;                    // used to read 1-wire sensor every second time
bool readTemp  = true;                    // used to cycle temp.readings
String st = "$POV,T,85.00*3E";
float t = 85;
 
OneWire oneWire(oneWireBus);              // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire);      // Pass the oneWire reference to Dallas Temperature sensor 

int nmea0183_checksum(const char *nmea_data)
  {
    int crc = 0;
    int i;
  
    // i = 1 , ignore the first $ sign,  no checksum in sentence
    for (i = 1; i < strlen(nmea_data); i ++) { 
      crc ^= nmea_data[i];
    }
    return crc;
  }

int sendToWifi(String st)
  {
//      st = "\n" + st + "\n";
      st = st + "\n";
      unsigned int stLength = st.length();

      for(int num_x= 0; num_x < NUM_COM ; num_x++)
      {
        //if(debug) COM[DEBUG_COM]->println("Writing temperature to wifi");
        if(COM[num_x] != NULL) 
        {
           for(byte cln_x = 0; cln_x < MAX_NMEA_CLIENTS; cln_x++)
            {
              if(TCPClient[num_x][cln_x]) 
                for(int i = 0; i < stLength; i++) 
                {
                  TCPClient[num_x][cln_x].write(st[i]); 
                }
            }
        }
      }
    return 0;
  }

int sendToBT(String st){

  #ifdef BLUETOOTH        
      st = st + "\n";
      unsigned int stLength = st.length();
        // now send to Bluetooth:
      if(SerialBT.hasClient())      
        for(int i = 0; i < stLength; i++) 
          {
                  SerialBT.write(st[i]); 
          }
  #endif
}
  
#endif


void setup() {

  delay(500);

  COM[0]->begin(UART_BAUD0, SERIAL_PARAM0, SERIAL0_RXPIN, SERIAL0_TXPIN);
  COM[1]->begin(UART_BAUD1, SERIAL_PARAM1, SERIAL1_RXPIN, SERIAL1_TXPIN);
  COM[2]->begin(UART_BAUD2, SERIAL_PARAM2, SERIAL2_RXPIN, SERIAL2_TXPIN);
  
  if(debug) COM[DEBUG_COM]->print("\n\nWiFi serial bridge version ");
  if(debug) COM[DEBUG_COM]->println(VERSION);
  
#ifdef ONEWIRE
  pinMode(led, OUTPUT);
  sensors.begin();        // Start the DS18B20 sensor
  sensors.setWaitForConversion(false);  // no wait for sensor reading
#endif //ONEWIRE

#ifdef MODE_AP 
  if(debug) COM[DEBUG_COM]->println("Open ESP Access-Point mode");
  WiFi.mode(WIFI_AP);                     //AP mode (phone connects directly to ESP) (no router)
  WiFi.softAP(ssid, pw);                  // configure ssid and password for softAP
  delay(2000);                            // workaround mj
  WiFi.softAPConfig(ip, ip, netmask);     // configure ip address for softAP 
#endif //MODE_AP 


#ifdef MODE_STA
   if(debug) COM[DEBUG_COM]->println("Open ESP Station mode");
    // STATION mode (ESP connects to router and gets an IP)
    // Assuming phone is also connected to that router
    // from RoboRemo you must connect to the IP of the ESP
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pw);
  if(debug) COM[DEBUG_COM]->print("try to Connect to Wireless network: ");
  if(debug) COM[DEBUG_COM]->println(ssid);
  while (WiFi.status() != WL_CONNECTED) {   
    delay(500);
    if(debug) COM[DEBUG_COM]->print(".");
  }
  if(debug) COM[DEBUG_COM]->println("\nWiFi connected");
#endif //MODE_STA
  
#ifdef BLUETOOTH
  if(debug) COM[DEBUG_COM]->println("Open Bluetooth Server");  
  if(debug) COM[DEBUG_COM]->print("Bluetooth SSID: ");  
  if(debug) COM[DEBUG_COM]->println(ssid);  
  SerialBT.begin(ssid);             //Bluetooth device name
#endif //BLUETOOTH
 
#ifdef OTA_HANDLER  
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    // if DNSServer is started with "*" for domain name, it will reply with
    // provided IP to all DNS request
  ArduinoOTA.begin();
#endif // OTA_HANDLER    

#ifdef PROTOCOL_TCP
  COM[0]->println("Starting TCP Server 1.");  
  if(debug) COM[DEBUG_COM]->println("Starting TCP Server 1");  
  server[0]->begin(); // start TCP server 
  server[0]->setNoDelay(true);
  COM[1]->println("Starting TCP Server 2.");
  if(debug) COM[DEBUG_COM]->println("Starting TCP Server 2");  
  server[1]->begin(); // start TCP server 
  server[1]->setNoDelay(true);
  COM[2]->println("Starting TCP Server 3.");
  if(debug) COM[DEBUG_COM]->println("Starting TCP Server 3");  
  server[2]->begin(); // start TCP server   
  server[2]->setNoDelay(true);
#endif //PROTOCOL_TCP

 esp_err_t esp_wifi_set_max_tx_power(50);  //lower WiFi Power

}


void loop() 
{  
#ifdef OTA_HANDLER  
  ArduinoOTA.handle();
#endif // OTA_HANDLER
  
#ifdef BLUETOOTH
  // receive from Bluetooth:
  if(SerialBT.hasClient()) 
  {
    while(SerialBT.available())
    {
      BTbuf[iBT] = SerialBT.read(); // read char from client app
      if(iBT <bufferSize-1) iBT++;
    }          
    for(int num= 0; num < NUM_COM ; num++)
      COM[num]->write(BTbuf,iBT); // now send to UART(num):          
    iBT = 0;
  }  
#endif  //BLUETOOTH

#ifdef ONEWIRE
  // get temperature reading from sensor
  currentMillis = millis();
  currentMillis_2 = currentMillis;

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    digitalWrite(led, ledState);
    
    #ifdef PROTOCOL_TCP
      sendToWifi(st);
    #endif // PROTOCOL_TCP
    #ifdef BLUETOOTH
      sendToBT(st);
    #endif //BLUETOOTH
  }

  if (currentMillis_2 - previousMillis_2 >= interval_2) {
  previousMillis_2 = currentMillis_2;    
  readTemp = true;
  }

  if (readTemp) {
   if (readState) {
      readState = false;
      sensors.requestTemperatures(); // initiate temperature sensor
   }
    // time to collect the temperature
    if ((!readState) and (millis() - previousMillis_2 >= 750)) {
      readTemp = false;
      readState = true;
      float t = sensors.getTempCByIndex(0);

      // NMEA output in openvario format
      // $POV,T,23.52*35
      st = "$POV,T," + String(t, 1) ;
      st = st + "*" + String(nmea0183_checksum(st.c_str()), HEX);
      st.toUpperCase();
      if(debug) COM[DEBUG_COM]->println(st);
    }
}

#endif //ONEWIRE

#ifdef PROTOCOL_TCP
  for(int num= 0; num < NUM_COM ; num++)
  {
    if (server[num]->hasClient())
    {
      for(byte i = 0; i < MAX_NMEA_CLIENTS; i++){
        //find free/disconnected spot
        if (!TCPClient[num][i] || !TCPClient[num][i].connected()){
          if(TCPClient[num][i]) TCPClient[num][i].stop();
          TCPClient[num][i] = server[num]->available();
          if(debug) COM[DEBUG_COM]->print("New client for COM"); 
          if(debug) COM[DEBUG_COM]->print(num); 
          if(debug) COM[DEBUG_COM]->println(i);
          continue;
        }
      }
      //no free/disconnected spot so reject
      WiFiClient TmpserverClient = server[num]->available();
      TmpserverClient.stop();
    }
  }
#endif //PROTOCOL_TCP
 
  for(int num= 0; num < NUM_COM ; num++) {
    if(COM[num] != NULL) {
      for(byte cln = 0; cln < MAX_NMEA_CLIENTS; cln++) {               
        if(TCPClient[num][cln]) {
          while(TCPClient[num][cln].available())
          {
            buf1[num][i1[num]] = TCPClient[num][cln].read(); // read char from client app
            if(i1[num]<bufferSize-1) i1[num]++;
          } 

          COM[num]->write(buf1[num], i1[num]); // now send to UART(num):
          i1[num] = 0;
        }
      }
  
      if(COM[num]->available()) {
        while(COM[num]->available())
        {     
          buf2[num][i2[num]] = COM[num]->read(); // read char from UART(num)
          if(i2[num]<bufferSize-1) i2[num]++;
        }
        // now send to WiFi:
        for(byte cln = 0; cln < MAX_NMEA_CLIENTS; cln++)
        {   
          if(TCPClient[num][cln])                     
            TCPClient[num][cln].write(buf2[num], i2[num]);
        }
        
#ifdef BLUETOOTH        
        // now send to Bluetooth:
        if(SerialBT.hasClient())      
          SerialBT.write(buf2[num], i2[num]);               
#endif  

        i2[num] = 0;


        
      }
    }
  }
}
