#include <SoftwareSerial.h>

#define DOMO_DEBUG
#define DOMO_SPEED 9600
//#define USE_WDT
//#define WDT_TIME WDTO_8S

#include <common_initial.h>
#include "messages.h"
#include "gsm.h"
#include "interface.h"

void setup() {
  initialGeneric();
  DEB_DO_PRINTLN(MSG_START);
  initGsm();
  initI2c();
}

void loop()
{
  updateStatus();
  if (status == 0) {
    if (sms == false) { // If true wait for read from I2C
      msg = GSM.readSms(index);
      if (msg != "") {
        msg = msg.substring(msg.indexOf("+CMGR:"), msg.length() - 4);
        // Serial.println(msg);
        sms = true;
        index++;
      }
    }
  }

  deleteSms();

  delay(500);
}

// void updateSerial();
// void loop()
// {
// //  delay(1000);
// //  Serial.println("Uno");
// //  text = GSM.readSms(1);
// //  Serial.println(text);
// //  Serial.println("FIN");
//  updateSerial();
// }
//
// void updateSerial()
// {
//  while (Serial.available())
//  {
//    GSM.write(Serial.read());//Forward what Serial received to Software Serial Port
//  }
//  while(GSM.available())
//  {
//    Serial.println("a");
//    String rta = GSM._readSerial();
//    Serial.println(rta.endsWith("OK\r\n"));
//    Serial.println(rta.endsWith("ERROR\r\n"));
//    Serial.println(rta);//Forward what Software Serial received to Serial Port
//    Serial.println("------------");
//  }
// }
