
#ifndef _STORAGE_H_
#define _STORAGE_H_

#include "ch.h"
#include "common.h"
#include "tables.h"
#include "settings.h"

// EEPROM is M95640 (ST)
#define EEPROM_SIZE 8192 // 64Kb, 8KB
#define EEPROM_PAGE_SIZE 32
#define EEPROM_WRITE_TIME_MS 10 // 5ms byte/page write in datasheet
#define EEPROM_SPID SPID2
#define EEPROM_SPIDCONFIG spi2cfg
#define EEPROM_SPLIT 1024
#define EEPROM_TABLES_PAGE_SIZE 1024
#define EEPROM_TABLES_START 1024

extern const SPIConfig EEPROM_SPIDCONFIG;

/* Support functions */
//uint32_t eeFindCurrentPage(void);
//uint32_t eeFindNextPage(void);
//uint8_t eePushPage(uint8_t *buffer, uint32_t len); /* Push data to next page */
//uint8_t eePullPage(uint8_t *buffer, uint32_t len); /* Pull data from current page */

/* Public functions */
void eeInit(void);

void readTablesFromEE(void);
void writeTablesToEE(void);

void readSettingsFromEE(void);
void writeSettingsToEE(void);

#endif
