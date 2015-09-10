/*
 * init.c
 *
 *  Created on: Sep 10, 2015
 *      Author: fpoussin
 */

#include "ch.h"
#include "vectors.h"

/**
 * @brief   CCM segment initialization switch.
 */
#if !defined(CRT0_INIT_CCM) || defined(__DOXYGEN__)
#define CRT0_INIT_CCM              TRUE
#endif

/**
 * @brief   ROM image of the CCM segment start.
 * @pre     The symbol must be aligned to a 32 bits boundary.
 */
extern uint32_t _siccm;

/**
 * @brief   CCM segment start.
 * @pre     The symbol must be aligned to a 32 bits boundary.
 */
extern uint32_t _sccm;

/**
 * @brief   CCM segment end.
 * @pre     The symbol must be aligned to a 32 bits boundary.
 */
extern uint32_t _eccm;

/**
 * @brief   ISR Vector table.
 * @pre     The symbol must be aligned to a 32 bits boundary.
 */
extern vectors_t _vectors;

/**
 * @brief   STM32 CCM vectors table.
 */
vectors_t _vectors_CCM;

void __late_init(void)
{

#if CRT0_INIT_CCM
  /* CCM segment initialization.*/
  {
    uint32_t *tp, *dp, *end;

    tp = &_siccm;
    dp = &_sccm;
    while (dp < &_eccm)
      *dp++ = *tp++;

    tp = (uint32_t*)&_vectors;
    dp = (uint32_t*)&_vectors_CCM;
    end = (uint32_t*)((&_vectors_CCM)+sizeof(vectors_t));

    while (dp < end)
      *dp++ = *tp++;

	SCB->VTOR = (uint32_t)&_vectors_CCM;
  }
#endif
}
