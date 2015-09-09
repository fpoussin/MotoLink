/*
 * vectors.h
 *
 *  Created on: Sep 9, 2015
 *      Author: fpoussin
 */

#ifndef COMMON_VECTORS_H_
#define COMMON_VECTORS_H_

#include "cmparams.h"

/**
 * @brief   Type of an IRQ vector.
 */
typedef void  (*irq_vector_t)(void);

/**
 * @brief   Type of a structure representing the whole vectors table.
 */
typedef struct {
  uint32_t      *init_stack;
  irq_vector_t  reset_handler;
  irq_vector_t  nmi_handler;
  irq_vector_t  hardfault_handler;
  irq_vector_t  memmanage_handler;
  irq_vector_t  busfault_handler;
  irq_vector_t  usagefault_handler;
  irq_vector_t  vector1c;
  irq_vector_t  vector20;
  irq_vector_t  vector24;
  irq_vector_t  vector28;
  irq_vector_t  svc_handler;
  irq_vector_t  debugmonitor_handler;
  irq_vector_t  vector34;
  irq_vector_t  pendsv_handler;
  irq_vector_t  systick_handler;
  irq_vector_t  vectors[CORTEX_NUM_VECTORS];
} vectors_t;


#endif /* COMMON_VECTORS_H_ */
