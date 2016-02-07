
#ifndef _STORAGE_H_
#define _STORAGE_H_

#include "ch.h"
#include "common.h"
#include "tables.h"
#include "settings.h"

#define EEPROM_SIZE (64*1024)
#define EEPROM_PAGE_SIZE 32
#define EEPROM_WRITE_TIME_MS 10
#define EEPROM_SPID SPID2
#define EEPROM_SPIDCONFIG spi2cfg
#define EEPROM_SPLIT 4096
#define EEPROM_TABLES_PAGE_SIZE 1024
#define EEPROM_TABLES_START 4096

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
