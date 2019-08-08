#include <Wire.h>

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

REQUEST req;
String reqBody;
RESPONSE res;
String resBody;

void sendData();
void receiveData(int byteCount);
String parseProperty(String* json, String property, uint8_t maxLength);

void initI2c() {
  Wire.begin(SLAVE_ADDRESS);
  Wire.onRequest(sendData);
  Wire.onReceive(receiveData);
}

/*
LA RASPI SIEMPRE VA A HACER UN WRITE -> receiveData hace lo que tiene que hacer y guarda lo que paso
                                     -> sendData retorna lo que debe/paso en el receive
                                     -> de esta forma la raspi siempre sabe que es lo que paso
*/
void sendData() {
  // byte dataSend[] = {'a','2','3'}; //1 y 2 son bytes arbitrarios
  // Wire.write(dataSend, 3);
  // A ENVIAR
  if (req.action == 11) {
    // - Status
  } else if(req.action == 12) {
    // - Rta SMS
  } else if(req.action == 13) {
    // - SMS, Pasar sms = false, asi lee el proximo
  } else if(req.action == 14) {
    // - localizacion
  }
}

void receiveData(int byteCount) {
  // Clean REQUEST
  memset(&req, 0, sizeof(req));
  byte data[byteCount];
  uint8_t i = 0;
  while(Wire.available()) {
    data[i] = Wire.read();
    i++;
  }
  memcpy(&req, data, sizeof(data));
  Serial.println(req.action);
  Serial.println(req.part);
  Serial.println(req.of);
  Serial.println(req.body);
  // ACCIONES
  if (req.action == 1) {
    // - Llamar a X
    reqBody = String(req.body);
    String n = parseProperty(&reqBody, String('n'), 17);
    char number[n.length()];
    n.toCharArray(number, n.length() + 1);
    GSM.callNumber(number);
    // GUARDAR RESULTADO???
    // CUANDO CORTO???
  } else if(req.action == 2) {
    GSM.answerCall();
    // GUARDAR RESULTADO???
    // CUANDO CORTO??? -> QUE CORTE EL QUE LLAMA
  } else if(req.action == 3) {
    reqBody += String(req.body);
    if (req.part == req.of) {
      String n = parseProperty(&reqBody, String('n'), 17);
      char number[n.length()];
      n.toCharArray(number, n.length() + 1);
      String b = parseProperty(&reqBody, String('b'), 17);
      char body[b.length()];
      n.toCharArray(body, b.length() + 1);
      GSM.sendSms(number, body);
      // GUARDAR RESULTADO???
    }
  } else if(req.action == 4) {
    GSM.calculateLocation();
    // GUARDAR RESULTADO???
  } // Ignorar el resto
}

String parseProperty(String* json, String property, uint8_t maxLength){
  String val= "\"" + property + "\":";
  int i= json->indexOf(val);
  if(i != -1){
    String subStr= json->substring(i + val.length() + 1, i + val.length() + 1 + maxLength + 1);
    i= subStr.indexOf("\"");
    subStr= subStr.substring(0, i);
    return subStr;
  }else{
    return "null";
  }
}
