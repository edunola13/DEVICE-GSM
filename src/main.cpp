#include <SoftwareSerial.h>

#define DOMO_DEBUG
#define DOMO_SPEED 9600
//#define USE_WDT
//#define WDT_TIME WDTO_8S

#include <common_initial.h>
#include "messages.h"
#include "parser.h"
#include "gsm.h"
#include "interface.h"

void setup() {
  initialGeneric();
  DEB_DO_PRINTLN(MSG_START);
  initGsm();
  initI2c();
  refresh = millis() + 60000;
}

void loop()
{
  updateStatus();
  // deleteSms(); -> Eliminar por acciones
  treatRequest();
  prepareRes();

  if (refresh < millis()) {
    refresh = millis() + 60000;
    initI2c();
  }

  delay(250);
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
