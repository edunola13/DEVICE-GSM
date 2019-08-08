#include <Sim800L.h>
#include <common_initial.h>

#define RX  11
#define TX  10

Sim800L GSM(RX, TX);

uint8_t status = 0;
bool ring = false;
bool sms = false;
uint8_t index = 1; // Index from read SMS
String msg; // Last SMS read
String number = ""; // Number calling

void initGsm() {
  GSM.begin(9600);
  bool config = false;
  while (!config) {
    delay(500);
    config = GSM.setMessageStorage("SM");
    config = config && GSM.setMessageFormat("1");
  }
  Serial.println("GSM Ready");
}

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
