#include "canbus.h"
#include "sensors.h"
#include "storage.h"
#include "prot_obd.h"
#include "prot_yamaha.h"

const CANFilter canfilter_obd[] = {{1, 0, 0, 0, 0x07D0, 0x07E8}};
const CANFilter canfilter_yam[] = {{1, 0, 0, 0, YAMAHA_SID_MAIN, YAMAHA_SID_MAIN},
                                   {1, 0, 0, 0, 0x215, 0x215}}; // TODO

void checkCanFilters(CANDriver *canp, const CANConfig *config) {

  static uint8_t filter = 0;

  if (settings.sensorsInput == SENSORS_INPUT_OBD_CAN && filter != 1) {

    filter = 1;
    canStop(canp);
    canSTM32SetFilters(canp, 1, 1, canfilter_obd);
    canStart(canp, config);
  }

  else if (settings.sensorsInput == SENSORS_INPUT_YAMAHA_CAN && filter != 2) {

    filter = 2;
    canStop(canp);
    canSTM32SetFilters(canp, 1, 2, canfilter_yam);
    canStart(canp, config);
  }

  else {
      canStop(canp);
      canSTM32SetFilters(canp, 1, 0, NULL);
      canStart(canp, config);
  }
}

bool serveCanOBDPidRequest(CANDriver *canp, CANTxFrame *txmsg, const CANRxFrame *rxmsg)
{
  uint8_t i;
  float ftmp;
  bool pass = true;
  obd_msg_t* rxobd = (obd_msg_t*)rxmsg->data8;
  obd_msg_t* txobd = (obd_msg_t*)txmsg->data8;

  // Check Mode 1
  if (rxobd->mode != OBD_MODE_QUERY_LIVEDATA)
    return false;

  // Test addresses 0x7DF then 0x7D0-0x7D7
  if (rxmsg->SID != 0x7DF) {
    pass = false;

    for (i = 0; i <= 7; i++) {
      if (rxmsg->SID == 0x7D0 + i)
        pass = true;
    }
  }

  if (!pass) return false;

  txmsg->SID = 0x7E8; // Recipient address
  txmsg->IDE = CAN_IDE_STD;
  txmsg->RTR = CAN_RTR_DATA;
  txmsg->DLC = 8; // As per OBD standard
  txmsg->data64[0] = 0; // Clear data
  txobd->len = 0x03; // Default data length (mode + pid + 8b data)
  txobd->mode = OBD_MODE_REPLY_LIVEDATA; // Show live data
  txobd->pid = rxobd->pid; // PID

  // Read PID and process
  switch (rxobd->pid) {

    // Setup PIDs
    case OBD_PID_SUPPORT: // Supported PIDs 0x01-0x20
      txobd->len = 0x06;
      txobd->data[0] = 0x10; // 0x04 OBD_PID_LOAD
      txobd->data[1] = 0x18; // 0x0C OBD_PID_RPM, 0x0D OBD_PID_SPEED
      txobd->data[2] = 0xB0; // 0x11 OBD_PID_TPS, 0x13 OBD_PID_AFR_CNT, 0x14 OBD_PID_LAMBDA
      txobd->data[3] = 0x11; // 0x1C OBD_PID_STANDARD, 0x20 OBD_PID_SUPPORT2
      break;

    case OBD_PID_SUPPORT2: // Supported PIDs 0x21-0x40
      txobd->len = 0x06;
      txobd->data[0] = 0x10; // 0x24 OBD_PID_AFR
      txobd->data[3] = 0x01; // 0x40 OBD_PID_SUPPORT3
      break;

    case OBD_PID_SUPPORT3: // Supported PIDs 0x41-0x60
      txobd->len = 0x06;
      txobd->data[0] = 0x60; // 0x42 OBD_PID_AFR, 0x43 OBD_PID_ABS_LOAD
      break;

    case OBD_PID_SUPPORT4: // Supported PIDs 0x61-0x80
    case OBD_PID_SUPPORT5: // Supported PIDs 0x81-0xA0
    case OBD_PID_SUPPORT6: // Supported PIDs 0xA1-0xC0
    case OBD_PID_SUPPORT7: // Supported PIDs 0xC1-0xE0
    case OBD_PID_CODES: // Error codes - no codes
      txobd->len = 0x06;
      break;


    // 0x01-1F
    case OBD_PID_LOAD:
    case OBD_PID_TPS:
      txobd->data[0] = 0xFF & ((uint16_t)sensors_data.tps * 128) / 100;
      break;
    case OBD_PID_AFR_CNT:
      txobd->data[0] = 0xFF; // How many oxygen sensors we have
      break;
    case OBD_PID_LAMBDA:
      txobd->len = 0x06;
      ftmp = ((float)sensors_data.afr / 1.47f);
      txobd->data[0] = 0xFF & ((uint16_t)((2.0f / 65536.0f) * ftmp)); // Lambda LSB
      txobd->data[1] = 0xFF & (((uint16_t)((2.0f / 65536.0f) * ftmp)) >> 8); // Lambda MSB
      txobd->data[2] = 0x00;
      txobd->data[3] = 0x00;
      break;
    case OBD_PID_RPM:
      txobd->len = 0x04;
      txobd->data[0] = 0xFF & sensors_data.rpm; // RPM LSB
      txobd->data[1] = 0xFF & (sensors_data.rpm >> 8); // RPM MSB
      break;
    case OBD_PID_SPEED:
      txobd->data[0] = sensors_data.spd;
      break;
    case OBD_PID_STANDARD:
      txobd->data[0] = 0x0D; // JOBD, EOBD, and OBD II
      break;

    // 0x21-3F
    case OBD_PID_AFR:
      // AFR to lambda
      ftmp = ((float)sensors_data.afr / 1.47f);

      txobd->len = 0x06;
      txobd->data[0] = 0xFF & ((uint16_t)((2.0f / 65536.0f) * ftmp)); // Lambda LSB
      txobd->data[1] = 0xFF & (((uint16_t)((2.0f / 65536.0f) * ftmp)) >> 8); // Lambda MSB
      txobd->data[2] = 0xFF & (sensors_data.an3 >> 8); // Volts MSB
      txobd->data[3] = 0xFF & (sensors_data.an3); // Volts LSB
      break;


    // 0x41-5F
    case OBD_PID_VBAT:
      txobd->len = 0x04;
      txobd->data[0] = 0xFF & (sensors_data.an1); // VBAT LSB
      txobd->data[0] = 0xFF & (sensors_data.an1 >> 8); // VBAT MSB
      break;
    case OBD_PID_ABS_LOAD:
      txobd->len = 0x04;
      txobd->data[0] = 0xFF & (sensors_data.tps); // Load LSB
      txobd->data[0] = 0xFF & (sensors_data.tps >> 8); // Load MSB
      break;

    default:
      break;
    }

  canTransmit(canp, CAN_ANY_MAILBOX, txmsg, TIME_MS2I(2));
  return true;
}

void makeCanOBDPidRequest(CANTxFrame *txmsg, uint8_t pid) {

  obd_msg_t* obd = (obd_msg_t*)txmsg->data8;

  txmsg->IDE = CAN_IDE_STD;
  txmsg->SID = 0x7DF; // ECU Address
  txmsg->RTR = CAN_RTR_DATA;
  txmsg->DLC = 8;
  txmsg->data64[0] = 0; // Clear data
  obd->len = 0x02; // Additional bytes
  obd->mode = OBD_MODE_QUERY_LIVEDATA; // Show current data
  obd->pid = pid;  // The PID
}

void readCanOBDPidResponse(const CANRxFrame *rxmsg) {

  uint8_t i;
  bool pass = true;
  obd_msg_t* obd = (obd_msg_t*)rxmsg->data8;

  // Check it's mode 1
  if (obd->mode != OBD_MODE_REPLY_LIVEDATA)
    return;

  // Test addresses 0x7DF 0x7D0-0x7D7
  if (rxmsg->SID != 0x7E8) {
    pass = false;

    for (i = 0; i <= 7; i++) {
      if (rxmsg->SID == 0x7E0 + i)
        pass = true;
    }
  }

  if (!pass) return;

  // Read PID and process
  switch (obd->pid) {
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
  canTransmit(canp, CAN_ANY_MAILBOX, txmsg, TIME_MS2I(50));

  makeCanOBDPidRequest(txmsg, OBD_PID_RPM);
  canTransmit(canp, CAN_ANY_MAILBOX, txmsg, TIME_MS2I(50));

  makeCanOBDPidRequest(txmsg, OBD_PID_SPEED);
  canTransmit(canp, CAN_ANY_MAILBOX, txmsg, TIME_MS2I(50));
}
