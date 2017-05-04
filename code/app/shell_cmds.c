#include "ch.h"
#include "hal.h"
#include "shell.h"
#include "chprintf.h"

bool dbg_can = false;
bool dbg_mts = false;
extern uint16_t irq_pct;

static void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]) {
  static const char *states[] = {CH_STATE_NAMES};
  thread_t *tp;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: threads\r\n");
    return;
  }
  chprintf(chp, "PRIO    STATE   PCT         NAME\r\n");
  tp = chRegFirstThread();
  do {
    chprintf(chp, "%4u %8s %05u %12s\r\n",
             (uint32_t)tp->p_prio,
             states[tp->p_state],
             tp->pct & 0x3FFF,
             tp->p_name);
    tp = chRegNextThread(tp);
  } while (tp != NULL);
  chprintf(chp, "IRQ Pct: %05u\r\n", irq_pct);
}

static void cmd_candbg(BaseSequentialStream *chp, int argc, char *argv[]) {

    (void)argc;
    (void)argv;
    uint8_t in, buf;

    chprintf(chp, "CanBus debug active. Press any key to quit.\r\n");
    chThdSleepMilliseconds(1000);
    dbg_can = true;
    in = chSequentialStreamRead(chp, &buf, 1);
    while (in == 0) {
        chThdSleepMilliseconds(10);
    }
    dbg_can = false;
    chprintf(chp, "Closed CanBus debug.\r\n");
}

static void cmd_mtsdbg(BaseSequentialStream *chp, int argc, char *argv[]) {

    (void)argc;
    (void)argv;
    uint8_t in, buf;

    chprintf(chp, "MTS debug active. Press any key to quit.\r\n");
    chThdSleepMilliseconds(1000);
    dbg_mts = true;
    in = chSequentialStreamRead(chp, &buf, 1);
    while (in == 0) {
        chThdSleepMilliseconds(10);
    }
    dbg_mts = false;
    chprintf(chp, "Closed MTS debug.\r\n");
}

const ShellCommand sh_commands[] = {
  {"threads", cmd_threads},
  {"candbg", cmd_candbg},
  {"mtsdbg", cmd_mtsdbg},
  {NULL, NULL}
};
