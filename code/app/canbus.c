#include "canbus.h"
#include "sensors.h"
#include "storage.h"
#include "prot_obd.h"
#include "prot_yamaha.h"

const CANFilter canfilter_obd = {1, 0, 0, 0, 0x07D0, 0x07E8};
const CANFilter canfilter_yam[] = {{1, 0, 0, 0, YAMAHA_SID_MAIN, YAMAHA_SID_MAIN},
                                   {1, 0, 0, 0, 0x215, 0x215}}; // TODO

void checkCanFilters(CANDriver *canp, const CANConfig *config) {

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

void serveCanOBDPidRequest(CANDriver *canp, CANTxFrame *txmsg, const CANRxFrame *rxmsg)
{
  uint8_t i;
  float ftmp;
  bool pass = false;
  // Check address and Mode 1
  if (rxmsg->data8[0] != 0x01 || rxmsg->RTR != CAN_RTR_DATA)
    return;

  // Test addresses 0x7DF 0x7D0-0x7D7
  if (rxmsg->SID != 0x7DF) {

    for (i = 0; i <= 7; i++) {
      if (rxmsg->SID == 0x7D0 + i)
        pass = true;
    }
  }

  if (!pass) return;

  txmsg->SID = 0x7E8; // Recipient address
  txmsg->IDE = CAN_IDE_STD;
  txmsg->RTR = CAN_RTR_DATA;
  txmsg->data64[0] = 0; // Clear data
  txmsg->data8[0] = 0x03; // Default data length (mode + pid + 8b data)
  txmsg->data8[1] = 0x41; // Show live data
  txmsg->data8[2] = rxmsg->data8[2]; // PID

  // Read PID and process
  switch (rxmsg->data8[2]) {

    // Setup PIDs
    case OBD_PID_SUPPORT: // Supported PIDs
      txmsg->data8[0] = 0x06;
      txmsg->data8[3] = 0x08; // 0x4 OBD_PID_LOAD
      txmsg->data8[4] = 0x08; // 0x11 OBD_PID_TPS
      txmsg->data8[5] = 0x0C; // 0x0C OBD_PID_RPM, 0x0D OBD_PID_SPEED
      txmsg->data8[6] = 0x01; // 0x24 OBD_PID_AFR
      break;

    case OBD_PID_SUPPORT2:
      txmsg->data8[0] = 0x06;
      txmsg->data8[3] = 0x03; // 0x42 OBD_PID_VBAT, 0x43 OBD_PID_ABS_LOAD
      break;
    case OBD_PID_SUPPORT3:
    case OBD_PID_SUPPORT4:
    case OBD_PID_SUPPORT5:
    case OBD_PID_SUPPORT6:
      txmsg->data8[0] = 0x06;
      break;

    case OBD_PID_CODES: // Error codes
      txmsg->data8[0] = 0x06;
      break;


    // Main PIDs
    case OBD_PID_LOAD:
    case OBD_PID_TPS:
      txmsg->data8[3] = (uint8_t)(sensors_data.tps * 2.55f);
      break;
    case OBD_PID_AFR_CNT:
      txmsg->data8[3] = 0x01; // How many oxygen sensors we have
      break;
    case OBD_PID_RPM:
      txmsg->data8[0] = 0x04;
      txmsg->data8[3] = 0xFF & ((uint16_t)sensors_data.rpm * 100) >> 8; // RPM MSB
      txmsg->data8[4] = 0xFF & ((uint16_t)sensors_data.rpm * 100); // RPM LSB
      break;
    case OBD_PID_SPEED:
      txmsg->data8[3] = sensors_data.spd;
      break;
    case OBD_PID_AFR:
      // AFR to lambda
      ftmp = ((float)sensors_data.afr / 1.47f);

      txmsg->data8[0] = 0x06;
      txmsg->data8[3] = 0xFF & (((uint16_t)((2.0f / 65536.0f) * ftmp)) >> 8); // Lambda MSB
      txmsg->data8[4] = 0xFF & ((uint16_t)((2.0f / 65536.0f) * ftmp)); // Lambda LSB
      txmsg->data8[5] = 0xFF & (sensors_data.an9 >> 8); // Volts MSB
      txmsg->data8[6] = 0xFF & (sensors_data.an9); // Volts LSB
      break;


    // 0x40+
    case OBD_PID_VBAT:
      txmsg->data8[0] = 0x04;
      txmsg->data8[3] = 0xFF & (sensors_data.an7 >> 8); // VBAT MSB
      txmsg->data8[3] = 0xFF & (sensors_data.an7); // VBAT LSB
      break;
    case OBD_PID_ABS_LOAD:
      txmsg->data8[0] = 0x04;
      txmsg->data8[3] = 0xFF & (sensors_data.tps >> 8); // Load MSB
      txmsg->data8[3] = 0xFF & (sensors_data.tps); // Load LSB
      break;

    default:
      txmsg->data8[3] = 0;
      break;
    }

  canTransmit(canp, CAN_ANY_MAILBOX, txmsg, MS2ST(50));
}

void makeCanOBDPidRequest(CANTxFrame *txmsg, uint8_t pid) {

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

void readCanOBDPidResponse(const CANRxFrame *rxmsg) {

  uint8_t i;
  bool pass = false;
  // Check it's mode 1
  if (rxmsg->data8[1] != 0x41)
    return;

  // Test addresses 0x7DF 0x7D0-0x7D7
  if (rxmsg->SID != 0x7E8) {

    for (i = 0; i <= 7; i++) {
      if (rxmsg->SID == 0x7E0 + i)
        pass = true;
    }
  }

  if (!pass) return;

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
void readCanYamahaPid(const CANRxFrame *rxmsg) {

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

  makeCanOBDPidRequest(txmsg, OBD_PID_LOAD);
  canTransmit(canp, CAN_ANY_MAILBOX, txmsg, MS2ST(50));

  makeCanOBDPidRequest(txmsg, OBD_PID_RPM);
  canTransmit(canp, CAN_ANY_MAILBOX, txmsg, MS2ST(50));

  makeCanOBDPidRequest(txmsg, OBD_PID_SPEED);
  canTransmit(canp, CAN_ANY_MAILBOX, txmsg, MS2ST(50));
}
