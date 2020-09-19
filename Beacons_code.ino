
#include "BLEDevice.h"
#include "BLEUtils.h"
#include "BLEServer.h"
#include "esp_sleep.h"
#include "sys/time.h"

#define GPIO_DEEP_SLEEP_DURATION     10  // sleep x seconds and then wake up

/******************** BLE ************************************/
BLEAdvertising *pAdvertising;
char BEACON_UUID[] =    "8ec76ea3-6602-0106-0609-09556e695368"; // for iBeacon
String product_url = "bit.ly/Brizo64a";
char beacon_data[36];
int bec = 0;
int url_length;
struct timeval now;
float value;

RTC_DATA_ATTR static uint32_t counter; // remember number of boots in RTC Memory

void iBeacon(){
  beacon_data[0] = 0x02;      // Length
  beacon_data[1] = 0x01;      // 
  beacon_data[2] = 0x06;      // 
  beacon_data[3] = 0x1A;      // Length
  beacon_data[4] = 0xFF;      // Flag - Manufacture Specific Data
  beacon_data[5] = 0x4C;      // Apple INC.
  beacon_data[6] = 0x00;      // ...
  beacon_data[7] = 0x02;      // Type is Beacon.
  beacon_data[8] = 0x15;      // Length of next data is 21 byte
  int pt =9;
  for(int i=0;i<sizeof(BEACON_UUID) / sizeof(char)-1;i++)
  {
    
    if(BEACON_UUID[i] != '-'){
      //Serial.print(BEACON_UUID[i]);
      if((BEACON_UUID[i] >='A' && BEACON_UUID[i] <='F') || (BEACON_UUID[i] >='a' && BEACON_UUID[i] <='f'))
          BEACON_UUID[i] = 9+(BEACON_UUID[i]%16);
      if((BEACON_UUID[i+1] >='A' && BEACON_UUID[i+1] <='F') || (BEACON_UUID[i+1] >='a' && BEACON_UUID[i+1] <='f'))
          BEACON_UUID[i+1] = 9+(BEACON_UUID[i+1]%16);
      beacon_data[pt] = ((BEACON_UUID[i])<<4)+((int)BEACON_UUID[i+1] & 0x0F);
      i++;
      pt++;}
  }
  beacon_data[25] = 0x00;     // major
  beacon_data[26] = 0x10;     // ..
  beacon_data[27] = 0x00;     // minor
  beacon_data[28] = 0x01;     //...
  beacon_data[29] = -35;     // Tx Power
  
  }
  
void EddyStoneUID(){
    //beacon_data[0] = 0x20;    // Eddystone Frame Type (Unencrypted Eddystone - TLM)
  beacon_data[0] = 0x02;      // Length
  beacon_data[1] = 0x01;      // 
  beacon_data[2] = 0x06;      // 
  beacon_data[3] = 0x03;      // Length
  beacon_data[4] = 0x03;      // Flag - Complete list of 16-bit Service UUIDs data type value
  beacon_data[5] = 0xAA;      // 16bit Eddystone UUID
  beacon_data[6] = 0xFE;      // ...

  beacon_data[7] = 23;      // Length
  beacon_data[8] = 0x16;      // Frame Type - Service Data
  beacon_data[9] = 0xAA;      // Eddystone
  beacon_data[10] = 0xFE;      // 
  beacon_data[11] = 0x00;      // Frame Type - UID
  beacon_data[12] = 0x40;      // Tx power 4dBm?
  //NameSpace
  beacon_data[13] = 0x1A;
  beacon_data[14] = 0xCE;
  beacon_data[15] = 0xCD;
  beacon_data[16] = 0xBE;
  beacon_data[17] = 0x1A;
  beacon_data[18] = 0x01;
  beacon_data[19] = 0x89;
  beacon_data[20] = 0x99;
  beacon_data[21] = 0x81;
  beacon_data[22] = 0x66;
  //InstanceID
  beacon_data[23] = 0x18;
  beacon_data[24] = 0x20;
  beacon_data[25] = 0x30;
  beacon_data[26] = 0x22;
  beacon_data[27] = 0xEF;
  beacon_data[28] = 0xCD;
  //RFU
  beacon_data[29] = 0x40;
  beacon_data[30] = 0x40;
  

  }
void EddyStoneUrl(){
  int count;
  url_length = product_url.length();
  
    //beacon_data[0] = 0x20;    // Eddystone Frame Type (Unencrypted Eddystone - TLM)
  beacon_data[0] = 0x02;      // Length
  beacon_data[1] = 0x01;      // 
  beacon_data[2] = 0x06;      // 
  beacon_data[3] = 0x03;      // Length
  beacon_data[4] = 0x03;      // Flag - Complete list of 16-bit Service UUIDs data type value
  beacon_data[5] = 0xAA;      // 16bit Eddystone UUID
  beacon_data[6] = 0xFE;      // ...

  
  beacon_data[7] = url_length+6;      // Length
  beacon_data[8] = 0x16;      // Frame Type - Service Data
  beacon_data[9] = 0xAA;      // Eddystone
  beacon_data[10] = 0xFE;      // 
  beacon_data[11] = 0x10;      // Frame Type - URL
  beacon_data[12] = 0x00;      // Tx power 4dBm?
  beacon_data[13] = 0x03;      // URL Scheme Prefix - https://
  for(count=0; count<url_length; count++) {
    beacon_data[14+count] = product_url.charAt(count);
  }
  }



void EddyStoneTLM(){
  beacon_data[0] = 0x02;      // Length
  beacon_data[1] = 0x01;      // 
  beacon_data[2] = 0x06;      // 
  beacon_data[3] = 0x03;      // Length
  beacon_data[4] = 0x03;      // Flag - Complete list of 16-bit Service UUIDs data type value
  beacon_data[5] = 0xAA;      // 16bit Eddystone UUID
  beacon_data[6] = 0xFE;      // ...

  beacon_data[7] = 17;      // Length
  beacon_data[8] = 0x16;      // Frame Type - Service Data
  beacon_data[9] = 0xAA;      // Eddystone
  beacon_data[10] = 0xFE;      // 
  beacon_data[11] = 0x20;      // Frame Type - TLM
  beacon_data[12] = 0x00;      // VERSION?
  beacon_data[13] = 0x00;      // BATTERY voltage
  beacon_data[14] = 90;        // ..
  beacon_data[15] = 32;      // TEMPARATURE
  beacon_data[16] = 0x33;
  //ADV Count
  beacon_data[17] = counter /(16777216);
  beacon_data[18] = counter/(65536);
  beacon_data[19] = counter/(256);
  beacon_data[20] = counter;
  Serial.println(counter);
  //Boot time
  float val = value*10;
  beacon_data[21] = val /(16777216);
  beacon_data[22] = val/(65536);
  beacon_data[23] = val/(256);
  beacon_data[24] = val;

  Serial.println(value); //prints time since program started
        // Create BLE device
  
  }

void EddyStoneEID(){

  beacon_data[0] = 0x02;      // Length
  beacon_data[1] = 0x01;      // 
  beacon_data[2] = 0x06;      // 
  beacon_data[3] = 0x03;      // Length
  beacon_data[4] = 0x03;      // Flag - Complete list of 16-bit Service UUIDs data type value
  beacon_data[5] = 0xAA;      // 16bit Eddystone UUID
  beacon_data[6] = 0xFE;      // ...

  beacon_data[7] = 13;      // Length
  beacon_data[8] = 0x16;      // Frame Type - Service Data
  beacon_data[9] = 0xAA;      // Eddystone
  beacon_data[10] = 0xFE;      // 
  beacon_data[11] = 0x30;      // Frame Type - URL
  beacon_data[12] = 69;      // Tx power 4dBm?
  beacon_data[13] = 0x03;      
  beacon_data[14] = 0xBE;        
  beacon_data[15] = 0x00;      
  beacon_data[16] = 0xDA;
  beacon_data[17] = 0x00;
  beacon_data[18] = 0x00;
  beacon_data[19] = 0x01;
  beacon_data[20] = 0x00;
  
  }

void AltBeacon(){
  // Set flags
  beacon_data[0] = 0x02;      // Length
  beacon_data[1] = 0x01;      // 
  beacon_data[2] = 0x06;      //

   //send 28 bytes of Alt beacon
  beacon_data[3] = 0x1B;      // Length
  beacon_data[4] = 0xFF;      // Flag - Manufacture Specific Data
  beacon_data[5] = 0xFF;      // 16bit Device Code
  beacon_data[6] = 0xFF;      // ...
  beacon_data[7] = 0xBE;      // Type - AltBeacon
  beacon_data[8] = 0xAC;      // ..
  beacon_data[9] = 0xAA;      // Beacon ID of 20 byte
  beacon_data[10] = 0xFE;      
  beacon_data[11] = 0x30;      
  beacon_data[12] = 0x40;       
  beacon_data[13] = 0x03;      
  beacon_data[14] = 0xBE;        
  beacon_data[15] = 0x00;      
  beacon_data[16] = 0xDA;
  beacon_data[17] = 0x00;
  beacon_data[18] = 0x00;
  beacon_data[19] = 0x01;
  beacon_data[20] = 0x00;
  beacon_data[21] = 0x03;      
  beacon_data[22] = 0xBE;        
  beacon_data[23] = 0x00;      
  beacon_data[24] = 0xDA;
  beacon_data[25] = 0x00;
  beacon_data[26] = 0x00;
  beacon_data[27] = 0x01;
  beacon_data[28] = 0x00;

  beacon_data[29] = -69;
  beacon_data[30] = 0x00;
  
  }

void GeoBeacon(){
  // Set flags
  beacon_data[0] = 0x02;      // Length
  beacon_data[1] = 0x01;      // 
  beacon_data[2] = 0x06;      //

  beacon_data[3] = 0x1B;      // Length
  beacon_data[4] = 0xFF;      // Flag - Manufacture Specific Data
  beacon_data[5] = 0xE0;      // 16bit Device Code
  beacon_data[6] = 0x47;      // ...
  beacon_data[7] = 0x47;      // Type - GeoBeacon
  beacon_data[8] = 0xE0;      // ..
  
  beacon_data[9] = -69;      // ref RSSI
  
  beacon_data[10] = 200;    // battery volts
        
  beacon_data[11] = 0x00;   //NAC latitude      
  beacon_data[12] = 0x50;       
  beacon_data[13] = 0x05;      
  beacon_data[14] = 0x58;        
  beacon_data[15] = 0x0F;      
  beacon_data[16] = 0x54;
  
  beacon_data[17] = 0x00;   // NAC longitude
  beacon_data[18] = 0x4E;
  beacon_data[19] = 0x07;
  beacon_data[20] = 0x0F;
  beacon_data[21] = 0x4E;      
  beacon_data[22] = 0x4A;
          
  beacon_data[23] = 0xA0;      
  beacon_data[24] = 0xA1;
  beacon_data[25] = 0xA2;
  beacon_data[26] = 0xA3;
  beacon_data[27] = 0xA4;
  beacon_data[28] = 0xA5;
  beacon_data[29] = 0xA6;
  beacon_data[30] = 0xA7;
  
  }



void setup() {

  Serial.begin(115200);
  gettimeofday(&now, NULL);

  value = now.tv_sec + (float)now.tv_usec/1000000;
 
  BLEDevice::init("BEACONS");

  // Create BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  
  pAdvertising = pServer->getAdvertising();
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();

  //setBeacon();
  switch(counter % 7){
  case 0:
      Serial.println("Advertizing started as iBeacon");
      iBeacon();
      oAdvertisementData.addData(std::string(beacon_data, 30));
      break;
  case 1:
      Serial.println("Advertizing started as Eddystone-UID");
      EddyStoneUID();
      oAdvertisementData.addData(std::string(beacon_data, 31));
      break;
  case 2:
      Serial.println("Advertizing started as Eddystone-URL");
      EddyStoneUrl();
      url_length = product_url.length();
      oAdvertisementData.addData(std::string(beacon_data, url_length+14));
      break;
  case 3:
      Serial.println("Advertizing started as Eddystone-TLM");
      EddyStoneTLM();
      oAdvertisementData.addData(std::string(beacon_data, 25));
      break;
  case 4:
      Serial.println("Advertizing started as Eddystone-EID");
      EddyStoneEID();
      oAdvertisementData.addData(std::string(beacon_data, 21));
      break;
  case 5:
      Serial.println("Advertizing started as AltBeacon");
      AltBeacon();
      oAdvertisementData.addData(std::string(beacon_data, 31));
      break;
  case 6:
      Serial.println("Advertizing started as GeoBeacon");
      GeoBeacon();
      oAdvertisementData.addData(std::string(beacon_data, 31));
      break;
  }
  counter++;
  pAdvertising->setAdvertisementData(oAdvertisementData);
  pAdvertising->start();
  delay(100);
  pAdvertising->stop();
  Serial.printf("enter deep sleep\n");
  esp_deep_sleep(1000000LL * GPIO_DEEP_SLEEP_DURATION);
}

  void loop() {
    
  }
