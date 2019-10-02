// #include <SoftwareSerial.h>

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
  refresh = millis() + 21000;
  // Reserve Memory
  // reqBody.reserve(255);
  // resBody.reserve(255);
  // msg.reserve(255);
  //
  //
  // NOTA IMPORTANTE DE STRING: no esta bueno tener strings en el main porq nunca se limpian bien
  // se limpian bien cuando se mueren (en funcion por ejemplo), el reservar sirve si no voy a usar mas que eso.
  //
  //
}

void loop()
{
  updateStatus();
  // deleteSms(); -> Eliminar por acciones
  treatRequest();
  prepareRes();

  if (refresh < millis()) {
    refresh = millis() + 21000;
    initGsm();
    initI2c();
  }
}

// void updateSerial();
// void loop()
// {
// //  delay(1000);
// //  Serial.println("Uno");
// //  text = GSM.readSms(1);
// //  Serial.println(text);
// //  Serial.println("FIN");
//   updateSerial();
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
