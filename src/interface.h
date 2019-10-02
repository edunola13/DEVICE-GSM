#include <Wire.h>
#include "free.h"

#define SLAVE_ADDRESS 0x04

struct REQUEST {
  uint8_t action;
  uint8_t part;
  uint8_t of;
  char body[29];
};
struct RESPONSE {
  uint8_t part;
  uint8_t of;
  char body[30];
};

long refresh = 0;

uint8_t action = 0;
uint8_t lastAction = 0; // 0:Error, 1:Ok, x:Especifico
bool actionFinish = false;
bool prepareFinish = false;
REQUEST req;
// String reqBody;
char reqBody_array[97];
RESPONSE res;
String resBody;
uint8_t part = 1;
uint8_t of = 1;
char partResBody[30];

void sendData();
void receiveData(int byteCount);

void initI2c() {
  Wire.begin(SLAVE_ADDRESS);
  Wire.onRequest(sendData);
  Wire.onReceive(receiveData);
  DEB_DO_PRINTLN(F("I2C Ready"));
}

/*
LA RASPI SIEMPRE VA A HACER UN WRITE -> receiveData hace lo que tiene que hacer y guarda lo que paso
                                     -> sendData retorna lo que debe/paso en el receive
                                     -> de esta forma la raspi siempre sabe que es lo que paso
*/

void simpleBody() {
  if (lastAction == 1) {
    byte data[] = "{\"s\":\"ok\"}";
    memcpy(&res.body, data, sizeof(data));
  } else {
    byte data[] = "{\"s\":\"error\"}";
    memcpy(&res.body, data, sizeof(data));
  }
}

void treatRequest() {
  if (actionFinish) {
    // Ya tratada
    return;
  }
  String reqBody = String(reqBody_array);
  if (req.part == 1) {
    reqBody = String("");
  }

  String part_x = String(req.body);
  if (part_x.length() > 29) {
    part_x = part_x.substring(0, 29);
  }
  reqBody += part_x;
  if (req.part != req.of) {
    reqBody.toCharArray(reqBody_array, reqBody.length() + 1);
    actionFinish = true;
    return;
  }
  // ACCIONES
  if (req.action == 1) {
    // - Llamar a X
    String n = parseProperty(&reqBody, String('n'), 17);
    char number[n.length() + 1];
    n.toCharArray(number, n.length() + 1);
    lastAction = GSM.callNumber(number) ? 0 : 1;
  } else if(req.action == 2) {
    // - Atender
    lastAction = GSM.answerCall() ? 0 : 1;
  } else if(req.action == 3) {
    // - Cortar
    lastAction = GSM.hangoffCall() ? 0 : 1;
  } else if(req.action == 4) {
    // - Enviar SMS
    lastAction = sendSms(&reqBody) ? 1 : 0;
  } else if(req.action == 5){
    // - Leer SMS
    uint8_t i = parseProperty(&reqBody, String('i'), 3).toInt();
    lastAction = readSms(i) ? 1 : 0;
    if (lastAction == 1) {
      resBody = "{\"b\":\"";
      // resBody += msg;
      resBody += String(msg_array);
      resBody += "\"}";
      // msg = String("");
      // msg.remove(0, 1000);
    }
  } else if(req.action == 6){
    // Eliminar SMS
    lastAction = deleteSms() ? 1 : 0;
  } else if(req.action == 7) {
    // Calcular Location
    lastAction = GSM.calculateLocation() ? 1 : 0;
    // ACA GUARDAR LA LOCALIZACION
  } else if(req.action == 9) {
    // INFO VARIA: IR AGREGANDO, SI ESTA CONECTADO, CHIP_ID, ETC
    String signal = GSM.signalQuality();
    if (signal.indexOf(F("+CSQ:")) != -1) {
      uint8_t i = signal.indexOf(F("+CSQ:"));
      signal = signal.substring(i + 6);
      i = signal.indexOf(F("\r\n"));
      if (i != -1) {
        signal = signal.substring(0, i);
      }
    }
    resBody = String("{\"sig\":\"") + signal + String("\"}");
  } else if(req.action == 10) {
    GSM.setMessageFormat(F("1"));
    status = GSM.getCallStatus(); // Es lo unico que hacemos sin permiso de I2C y que puede llegar a romper la comunicacion
    if (status == 0) {
      ring = false;
    }

    resBody = String("{\"s\":") + status + String(",");
    resBody += String("\"r\":") + ring + String(",");
    if (ring) {
      resBody += String("\"n\":\"") + String(number) + String("\",");
      resBody += String("\"c\":") + carrier + String(",");
    }
    if (smsI) {
      resBody += String("\"i\":") + smsI + String(",");
    }
    resBody += String("\"m\":") + sms + String("}");
    sms = false; // Reseteo
    lastAction = 1;
  }
  actionFinish = true;
  // reqBody = String("");
  Serial.print("Memoria: ");
  Serial.println(freeMemory());
}

void prepareRes() {
  if (prepareFinish) {
    // Ya tratada
    return;
  }
  if(req.action == 30) {
    memset(&partResBody, 255, sizeof(partResBody));
    of = resBody.length() / 30;
    if (of*30 < resBody.length()) {
      of += 1;
    }
    String body = resBody.substring((req.part - 1) * 30, req.part * 30);
    body.toCharArray(partResBody, body.length() + 1);
  }
  prepareFinish = true;
  Serial.print("Memoria: ");
  Serial.println(freeMemory());
}

// Esto hay que hacerlo lo mas rapido posible porque es una interrupcion.
void sendData() {
  res.part = 1;
  res.of = 1;
  // A ENVIAR
  if (! actionFinish || ! prepareFinish) {
    byte data[] = "{\"s\":\"no_end\"}";
    memcpy(&res.body, data, sizeof(data));
  } else {
    if (req.action == 31) {
      // - Lectura de bodies largos y que hay que preparar antes
      res.part = part;
      res.of = of;
      memcpy(&res.body, partResBody, sizeof(partResBody));
    } else {
      //lastAction = 0;
      simpleBody();
    }
  }
  // byte dataSend[32];
  // memcpy(&dataSend, res, sizeof(res));
  Wire.write((char *)&res, 32);
  // byte dataSend[] = {1,1,'c', 'c', 'c', '2', 'd', 's', 'c', 'c', '2', 'a', '1'};
  // Wire.write(dataSend, 13);
}

// Esto hay que hacerlo lo mas rapido posible porque es una interrupcion.
void receiveData(int byteCount) {
  refresh = millis() + 21000; // Actualizo el refresh
  memset(&req, 0, sizeof(req));
  memset(&res, 255, sizeof(res));
  byte data[byteCount];
  uint8_t i = 0;
  while(Wire.available()) {
    data[i] = Wire.read();
    i++;
  }
  memcpy(&req, data, sizeof(data));

  // ACCIONES MENORES A 10 EJECUTAN ALGUNA ACCION
  if (req.action <= 10) {
    action = req.action;
    actionFinish = false;
    lastAction = 0; // Error
    part = 1;
    // resBody = String("");
    resBody.remove(0, 1000);
  }
  // ACCIONES ENTRE 11 Y 20 ES PARA LEER LO QUE PASO CON LAS ACCIONES DEL 1 AL 10
  // ACCION 30 ES PARA INDICAR PAGINA DE LO QUE QUIERO LEER
  if (req.action == 30) {
    part = req.part;
    of = 1;
    prepareFinish = false;
  }
}
