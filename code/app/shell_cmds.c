#include "ch.h"
#include "hal.h"
#include "shell.h"
#include "chprintf.h"

bool dbg_can = false;

static void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]) {
  static const char *states[] = {CH_STATE_NAMES};
  thread_t *tp;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: threads\r\n");
    return;
  }
#if CH_CFG_USE_DYNAMIC
  chprintf(chp, "    addr    stack prio refs     state\r\n");
  tp = chRegFirstThread();
  do {
    chprintf(chp, "%08lx %08lx %4lu %4lu %9s %lu\r\n",
             (uint32_t)tp,
             (uint32_t)tp->p_ctx.r13,
             (uint32_t)tp->p_prio,
             (uint32_t)(tp->p_refs - 1),
             states[tp->p_state]);
    tp = chRegNextThread(tp);
  } while (tp != NULL);
#else
  chprintf(chp, "    addr    stack prio     state\r\n");
  tp = chRegFirstThread();
  do {
    chprintf(chp, "%08lx %08lx %4lu %4lu %lu\r\n",
             (uint32_t)tp,
             (uint32_t)tp->p_ctx.r13,
             (uint32_t)tp->p_prio,
             states[tp->p_state]);
    tp = chRegNextThread(tp);
  } while (tp != NULL);
#endif
}

static void cmd_candbg(BaseSequentialStream *chp, int argc, char *argv[]) {

    (void)argc;
    (void)argv;
    uint8_t in, buf;

    chprintf(chp, "Press any key to quit\n");
    chThdSleepMilliseconds(1000);
    dbg_can = true;
    in = chSequentialStreamRead(chp, &buf, 1);
    while (in == 0) {
        chThdSleepMilliseconds(10);
    }
    dbg_can = false;
}

const ShellCommand sh_commands[] = {
  {"threads", cmd_threads},
  {"candbg", cmd_candbg},
  {NULL, NULL}
};
