#include <Sim800L.h>
#include <common_initial.h>

#define RX  11
#define TX  10

Sim800L GSM(RX, TX);

uint8_t status = 0; // Status
bool ring = false; // New Call
bool carrier = false; // Carrier
bool sms = false; // New Sms
String msg; // Last SMS read
uint8_t smsI; // Last SMS Index
// String number = ""; // Number calling
char number[20]; // Number calling

void initGsm() {
  GSM.begin(9600);
  bool config = false;
  while (!config) {
    delay(500);
    config = GSM.setMessageStorage("SM");
    config = config && GSM.setMessageFormat("1");
  }
  DEB_DO_PRINTLN("GSM Ready");
}

void updateStatus() {
  String data = GSM._readSerialUntilTime(10);
  while (data.length() > 2) {
    Serial.println(data);
    uint8_t i = data.indexOf("\r\n");
    if (i == -1) {
      break;
    }
    String line = data.substring(0, i);
    data = data.substring(i + 1);
    if (line.indexOf("RING") != -1) {
      if (! ring) {
        ring = true;
        // number = "";
        memset(&number, 255, sizeof(number));
        carrier = false;
      }
    } else if (line.indexOf("+CLIP:") != -1) {
      i = line.indexOf("\"");
      // number = line.substring(i + 1);
      // i = number.indexOf("\"");
      // number = number.substring(0, i);
      line = line.substring(i + 1);
      i = line.indexOf("\"");
      line = line.substring(0, i);
      line.toCharArray(number, line.length() + 1);
    } else if (line.indexOf("NO CARRIER") != -1) {
      carrier = false;
    } else if (line.indexOf("+CMTI:") != -1) {
      //+CMTI: "SM",5 -> Nuevo mensaje
      // NEW SMS
      i = line.indexOf("\",");
      if (i != -1) {
        smsI = line.substring(i + 2).toInt();
      }
      sms = true;
    }
  }
}

bool sendSms(String* data) {
  if (status == 0) {
    String n = parseProperty(data, String('n'), 17);
    String b = parseProperty(data, String('b'), 255);

    char number[n.length() + 1];
    char body[b.length() + 1];
    n.toCharArray(number, n.length() + 1);
    b.toCharArray(body, b.length() + 1);

    bool r = GSM.sendSms(number, body);
    if (!r){ // False is Ok
      Serial.println("Ok");
      return true;
    }
    Serial.println("Err");
  }
  return false;
}

bool readSms(uint8_t index) {
  if (status == 0) {
    msg = GSM.readSms(index);
    if (msg.length() > 255) {
      msg = msg.substring(0, 255);
    }
    if (msg != "") {
      msg = msg.substring(msg.indexOf("+CMGR:"), msg.length() - 4);
    }
    return true;
  }
  return false;
}

bool deleteSms() {
  if (status == 0) {
     if (GSM.delSms(1, 3) == false) { // False is OK
       return true;
     }
  }
  return false;
}
