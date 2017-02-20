
#ifndef _STORAGE_H_
#define _STORAGE_H_

#include "ch.h"
#include "common.h"

#ifdef MTL_APP
#include "tables.h"
#endif

// EEPROM is M95640 (ST)
#define EEPROM_SIZE 8192 // 64Kb, 8KB
#define EEPROM_PAGE_SIZE 32
#define EEPROM_WRITE_TIME_MS 7 // 5ms byte/page write in datasheet
#define EEPROM_SPID SPID2
#define EEPROM_SPIDCONFIG spi2cfg

#define EEPROM_SETTINGS_START 0
#define EEPROM_SETTINGS_END 999
#define EEPROM_VERSIONS_START 1000
#define EEPROM_VERSIONS_END 1023
#define EEPROM_TABLES_PAGE_SIZE 1024
#define EEPROM_TABLES_START 1024
#define EEPROM_TABLES_FIRST 0
#define EEPROM_TABLES_LAST (EEPROM_SIZE-EEPROM_TABLES_START-EEPROM_TABLES_PAGE_SIZE)
#define EEPROM_TABLES_SIZE (EEPROM_SIZE-EEPROM_TABLES_START)

extern const SPIConfig EEPROM_SPIDCONFIG;

/* Public functions */
void eeInit(void);

uint8_t readVersionFromEE(uint8_t idx, version_t* dst);
uint8_t writeVersionToEE(uint8_t idx, const version_t* src);

#define VERSION_IDX_BL  0
#define VERSION_IDX_APP 1
extern version_t versions[2];

#ifdef MTL_APP
extern settings_t settings;
uint8_t readTablesFromEE(void);
uint8_t writeTablesToEE(void);

uint8_t readSettingsFromEE(void);
uint8_t writeSettingsToEE(void);
#endif

#endif
