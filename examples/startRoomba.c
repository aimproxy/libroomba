#include "libroomba.h"

int main(int argc, char const *argv[]) {

  int rc;

  rc = initRoomba("192.168.1.94", "3178480822035600", ":1:1553461121:VD9uFW2Omfq336Vu");
  if (rc == MQTT_CODE_SUCCESS) {
    printf("Connected\n");
  }

  rc = sendCommand("cmd", argv[1]);
  if (rc == MQTT_CODE_SUCCESS) {
    printf("Sent\n");
  }

  return rc;
}
