/*
 * tables.c
 *
 *  Created on: 20 nov. 2014
 *      Author: Mobyfab
 */

#include "tables.h"

/* Columns are divided by 100 */
uint8_t tableColumns[16] = {0, 10, 20, 30, 40, 50, 60, 70,
                            80, 90, 100, 110, 120, 130,
                            140, 180};
uint8_t tableRows[11] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

uint8_t tableAFR[11][16];
uint8_t tableKnock[11][16];

void writeRows(uint8_t* rows, uint8_t size)
{
  uint8_t i;
  for (i=0; i<size; i++)
  {
    tableRows[i] = rows[i];
  }
}

void writeColumns(uint8_t* columns, uint8_t size)
{
  uint8_t i;
  for (i=0; i<size; i++)
  {
    tableColumns[i] = columns[i];
  }
}

bool findCell(uint8_t tp, uint8_t rpm, uint8_t* row, uint8_t* col)
{
  uint8_t i, maxcol, maxrow;
  bool rowFound = false, colFound = false;

  *row = 0;
  *col = 0;

  maxrow = sizeof(tableRows)-1;
  maxcol = sizeof(tableColumns)-1;

  if (tp >= tableRows[maxrow])
  {
    *row = maxrow;
    rowFound = true;
  }
  else if (tp <= tableRows[0])
  {
    *row = 0;
    rowFound = true;
  }
  else
  {
    for (i=0; i < maxrow; i++)
    {
      uint8_t h = tableRows[i];
      uint8_t h2 = tableRows[i+1];

      if (tp >= h && tp <= h2)
      {
        *row = i;
        rowFound = true;
        break;
      }
    }
  }

  if (rpm >= tableColumns[maxcol])
  {
    *col = maxcol;
    colFound = true;
  }
  else if (rpm <= tableColumns[0])
  {
    *col = 0;
    colFound = true;
  }
  else
  {
    for (i=0; i < maxcol; i++)
    {
      uint8_t h = tableColumns[i];
      uint8_t h2 = tableColumns[i+1];

      if (rpm >= h && rpm <= h2)
      {
        *col = i;
        colFound = true;
        break;
      }
    }
  }

  return rowFound && colFound;
}

