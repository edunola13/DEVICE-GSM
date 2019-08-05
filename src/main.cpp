#include <Sim800L.h>
#include <SoftwareSerial.h>
#include <Wire.h>

#define RX  11
#define TX  10

#define SLAVE_ADDRESS 0x04

Sim800L GSM(RX, TX);

uint8_t status = 0;
bool ring = false;
bool sms = false;
uint8_t index = 1; // Index from read SMS
String msg; // Last SMS read
String number = ""; // Number calling

struct REQUEST {
  char action[2];
  char body[30];
};
int actualI = 1;
struct RESPONSE {
  int i; // Indice X
  int f; // De J
  char body[28];
};

REQUEST req;
RESPONSE res;

void updateStatus() {
  String data = GSM._readSerial(50);
  while (data.length() > 2) {
    uint8_t i = data.indexOf("\r\n");
    if (i == -1) {
      break;
    }
    String line = data.substring(0, i);
    data = data.substring(i + 1);
    if (line.indexOf("RING") != -1) {
      if (! ring) {
        ring = true;
        number = "";
      }
    } else if (line.indexOf("+CLIP:") != -1) {
      i = line.indexOf("\"");
      number = line.substring(i + 1);
      i = number.indexOf("\"");
      number = number.substring(0, i);
    } else if (line.indexOf("NO CARRIER") != -1) {
      ring = false;
      number = "";
    } else if (line.indexOf("+CMTI:") != -1) {
      //+CMTI: "SM",5 -> Nuevo mensaje
      // NEW SMS
    }
  }

  status = GSM.getCallStatus();
}

bool sendSms() {
  updateStatus();
  if (status == 0) {
    char* text = "Saldo";
    char* number = "444";
    bool r = GSM.sendSms(number, text);
    if (!r){
      Serial.println("Send SMS");
    }
  }
  return false;
}

bool deleteSms() {
  updateStatus();
  uint8_t status = GSM.getCallStatus();
  if (index > 1 && status == 0 && sms == false) {
    bool d = false;
    msg = GSM.readSms(index);
    if (msg == "") {
      msg = GSM.readSms(index);
      if (msg == "") {
        d = true;
      }
    }

    if (d) {
       GSM.delAllSms();
       Serial.println("Delete SMS");
       index = 1;
    }
  }
}

//I2C
void sendData(){
//  Serial.println("aca");
//    byte dataSend[] = {'a','2','3'}; //1 y 2 son bytes arbitrarios
//   Wire.write(dataSend, 3);

  // A ENVIAR
  // - STATUS
  // - RTA SMS
  // - SMS, Pasar sms = false, asi lee el proximo
  // - LOCALIZACION
}

void receiveData(int byteCount){
  Serial.println(byteCount);
  byte data[32]; // Por ahi se puede poner el bytecount
  uint8_t i = 0;
  while(Wire.available()) {
    data[i] = Wire.read();
    i++;
  }
  memcpy(&req, data, sizeof(data));

  // ACCIONES
  // - Llamar a X
  // - Atender llamada
  // - Enviar SMS
  // - Actualizar localizacion
  // - Indicar lo que voy a pedir en el proximo sendData
}

void setup() {
  Serial.begin(9600);
  Serial.println("Arranco");
  GSM.begin(9600);
  bool config = false;
  while (!config) {
    delay(500);
    config = GSM.setMessageStorage("SM");
    config = config && GSM.setMessageFormat("1");
  }
  Serial.println("GSM Ready");

  Wire.begin(SLAVE_ADDRESS);
  Wire.onRequest(sendData);
  Wire.onReceive(receiveData);
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

  delay(10000);
}

//void loop()
//{
////  delay(1000);
////  Serial.println("Uno");
////  text = GSM.readSms(1);
////  Serial.println(text);
////  Serial.println("FIN");
//  updateSerial();
//}

//void updateSerial()
//{
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
//}
