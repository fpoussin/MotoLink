/*
 * tables.h
 *
 *  Created on: 20 nov. 2014
 *      Author: Mobyfab
 */

#ifndef TABLES_H_
#define TABLES_H_

#include "common.h"

typedef uint8_t cell_table_t[11][16];
typedef uint8_t cell_rows_t[11];
typedef uint8_t cell_cols_t[16];

extern cell_table_t tableAFR;
extern cell_table_t tableKnock;

extern cell_rows_t tableRows;
extern cell_cols_t tableColumns;

typedef struct {
  uint32_t magic;
  uint32_t cnt;
  cell_table_t afr;
  cell_table_t knock;
} tables_t;

void writeRows(uint8_t* rows, uint8_t size);
void writeColumns(uint8_t* columns, uint8_t size);
bool findCell(uint8_t tp, uint8_t rpm, uint8_t* row, uint8_t* col);

#endif /* TABLES_H_ */
