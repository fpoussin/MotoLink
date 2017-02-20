#include "canbus.h"
#include "sensors.h"
#include "storage.h"
#include "prot_obd.h"
#include "prot_yamaha.h"

const CANFilter canfilter_obd = {1, 0, 0, 0, 0x07E8, 0x07E8};
const CANFilter canfilter_yam[] = {{1, 0, 0, 0, YAMAHA_SID_MAIN, YAMAHA_SID_MAIN},
                                   {1, 0, 0, 0, 0x215, 0x215}}; // TODO

CCM_FUNC void checkCanFilters(CANDriver *canp, const CANConfig *config) {

  static uint8_t filter = 0;

  if (settings.sensorsInput == SENSORS_INPUT_OBD_CAN && filter != 1) {

    filter = 1;
    canStop(canp);
    canSTM32SetFilters(1, 1, &canfilter_obd);
    canStart(canp, config);
  }

  else if (settings.sensorsInput == SENSORS_INPUT_YAMAHA_CAN && filter != 2) {

    filter = 2;
    canStop(canp);
    canSTM32SetFilters(1, 2, canfilter_yam);
    canStart(canp, config);
  }
}

void makeCanOBDFrame(CANTxFrame *txmsg, uint8_t pid) {

  txmsg->IDE = CAN_IDE_STD;
  txmsg->SID = 0x7DF; // ECU Address
  txmsg->RTR = CAN_RTR_DATA;
  txmsg->DLC = 8;
  txmsg->data8[0] = 0x02; // Additional bytes
  txmsg->data8[1] = 0x01; // Show current data
  txmsg->data8[2] = pid;  // The PID
  txmsg->data8[3] = 0x00; // Not used
  txmsg->data32[1] = 0x00; // Not used
}

void readCanOBDPid(CANRxFrame *rxmsg) {

  // Check it's mode 1
  if (rxmsg->data8[1] != 0x41)
    return;

  // Read PID and process
  switch (rxmsg->data8[2]) {
    case OBD_PID_LOAD:
      sensors_data.tps = (100 * rxmsg->data8[3]) >> 8; // x/256
      break;
    case OBD_PID_RPM:
      sensors_data.rpm = ((uint16_t)rxmsg->data8[3] << 8) + rxmsg->data8[4];
      break;
    case OBD_PID_SPEED:
      sensors_data.spd = rxmsg->data8[3];
      break;
    default:
      break;
    }
}

// TODO
void readCanYamahaPid(CANRxFrame *rxmsg) {

  // Read SID and process
  switch (rxmsg->SID) {
    case YAMAHA_SID_MAIN:
      //
      break;
    default:
      break;
    }

}

void sendCanOBDFrames(CANDriver *canp, CANTxFrame *txmsg)
{
  if (canp->state != CAN_READY)
    return;

  makeCanOBDFrame(txmsg, OBD_PID_LOAD);
  canTransmit(canp, CAN_ANY_MAILBOX, txmsg, MS2ST(50));

  makeCanOBDFrame(txmsg, OBD_PID_RPM);
  canTransmit(canp, CAN_ANY_MAILBOX, txmsg, MS2ST(50));

  makeCanOBDFrame(txmsg, OBD_PID_SPEED);
  canTransmit(canp, CAN_ANY_MAILBOX, txmsg, MS2ST(50));
}
