/* This is a beacon for the LoRaHam protocol by KK4VCZ.
 * https://github.com/travisgoodspeed/loraham/
 * 
 * Modified by LeRoy Miller, KD8BXP
 * Aug 25, 2017 For use with the TTGO WIFI_LoRa_32 Boards
 * https://www.aliexpress.com/item/TTGO-loraSX1278-ESP32-0-96OLED-16-Mt-bytes-128-Mt-bit-433Mhz-for-arduino/32824758955.html
 * 
 * Significant changes:
 * 
 * Library Changed to: arduino-LoRa
 * https://github.com/sandeepmistry/arduino-LoRa
 * 
 */

#define CALLSIGN "KD8BXP-10"
#define COMMENTS "2600mAh ESP32"
 
#include <SPI.h>
#include <LoRa.h> //https://github.com/sandeepmistry/arduino-LoRa

// WIFI_LoRa_32 ports

// GPIO5  -- SX1278's SCK
// GPIO19 -- SX1278's MISO
// GPIO27 -- SX1278's MOSI
// GPIO18 -- SX1278's CS
// GPIO14 -- SX1278's RESET
// GPIO26 -- SX1278's IRQ(Interrupt Request)

#define SS      18
#define RST     14
#define DI0     26
#define BAND    433E6 // Change to 434.0 or other frequency, must match RX's freq!

#define VBATPIN 36 //battery voltage check pin
#define LED 25 //LED Pin

//Returns the battery voltage as a float.
float voltage(){

  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage
  return measuredvbat;
}

void radioon(){
  /*
   * // I don't believe this is needed for this board/library (?)
  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
 
  while (!LoRa.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");
 
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  
 
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
 
  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
  Serial.println("Set power to 23.");
  Serial.print("Max packet length: "); Serial.println(RH_RF95_MAX_MESSAGE_LEN);
*/
}

void radiooff(){
  //arduino-LoRa library provides a sleep, and a idle mode (Sleep is what I beleive we need here) 
  // manual reset
  //digitalWrite(RFM95_RST, LOW);
  LoRa.sleep();
  delay(10);
}

void setup() 
{
  pinMode(LED, OUTPUT);
  
Serial.begin(9600);
Serial.setTimeout(10);
  while (!Serial); //if just the the basic function, must connect to a computer
  delay(1000);
  
  Serial.println("BEACON"); 
  
  SPI.begin(5,19,27,18);
  LoRa.setPins(SS,RST,DI0);
  
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.enableCrc(); //encable CRC checking - off by default
  digitalWrite(LED, LOW);
}


//! Uptime in seconds, correcting for rollover.
long int uptime(){
  static unsigned long rollover=0;
  static unsigned long lastmillis=millis();

  //Account for rollovers, every ~50 days or so.
  if(lastmillis>millis()){
    rollover+=(lastmillis>>10);
    lastmillis=millis();
  }

  return(rollover+(millis()>>10));
}

//Transmits one beacon and returns.
void beacon(){
  static int packetnum=0;
  float vcc=voltage();
  
  //Serial.println("Transmitting..."); // Send a message to rf95_server
  
  char radiopacket[128];
  snprintf(radiopacket,
           128,
           "BEACON %s %s VCC=%d.%03d count=%d uptime=%ld",
           CALLSIGN,
           COMMENTS,
           (int) vcc,
           (int) (vcc*1000)%1000,
           packetnum,
           uptime());

  radiopacket[sizeof(radiopacket)] = 0;
  
  //Serial.println("Sending..."); delay(10);
  LoRa.beginPacket();
  LoRa.print(radiopacket);
  LoRa.endPacket();
  //rf95.send((uint8_t *)radiopacket, strlen((char*) radiopacket));
 
  //Serial.println("Waiting for packet to complete..."); delay(10);
  //rf95.waitPacketSent();
  packetnum++;
}

void loop(){
  //Turn the radio on.
  //radioon();
  //Transmit a beacon once every five minutes.
  beacon();
  //Then turn the radio off to save power.
  radiooff();
  
  //Five minute delay between beacons.
  delay(5*60000);
}
