/*
 * tables.h
 *
 *  Created on: 20 nov. 2014
 *      Author: Mobyfab
 */

#ifndef TABLES_H_
#define TABLES_H_

#include "ch.h"
#include "common.h"

extern uint8_t tableAFR[11][16];
extern uint8_t tableKnock[11][16];

extern uint8_t tableRows[11];
extern uint8_t tableColumns[16];

void writeRows(uint8_t* rows, uint8_t size);
void writeColumns(uint8_t* columns, uint8_t size);
bool findCell(uint8_t tp, uint8_t rpm, uint8_t* row, uint8_t* col);

#endif /* TABLES_H_ */
