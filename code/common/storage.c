#include "storage.h"
#include <string.h>

const SPIConfig EEPROM_SPIDCONFIG = {
  NULL,
  PORT_SPI2_NSS,
  PAD_SPI2_NSS,
  0, // Up to 20Mhz
  SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0
};

static SPIEepromFileConfig eeVersionsCfg = {
  EEPROM_VERSIONS_START,
  EEPROM_VERSIONS_END,
  EEPROM_SIZE,
  EEPROM_PAGE_SIZE,
  MS2ST(EEPROM_WRITE_TIME_MS),
  &EEPROM_SPID,
  &EEPROM_SPIDCONFIG,
};

#ifdef MTL_APP
static SPIEepromFileConfig eeSettingsCfg = {
  EEPROM_SETTINGS_START,
  EEPROM_SETTINGS_END,
  EEPROM_SIZE,
  EEPROM_PAGE_SIZE,
  MS2ST(EEPROM_WRITE_TIME_MS),
  &EEPROM_SPID,
  &EEPROM_SPIDCONFIG,
};

static SPIEepromFileConfig eeTablesCfg = {
  EEPROM_TABLES_START,
  EEPROM_SIZE,
  EEPROM_SIZE,
  EEPROM_PAGE_SIZE,
  MS2ST(EEPROM_WRITE_TIME_MS),
  &EEPROM_SPID,
  &EEPROM_SPIDCONFIG,
};
#endif

static SPIEepromFileStream versionsFile;
static EepromFileStream *versionsFS;
static version_t version_buf = {0};
#ifdef MTL_APP
static SPIEepromFileStream settingsFile, tablesFile;
static EepromFileStream *settingsFS, *tablesFS;

static const uint32_t magic_key = 0xABEF1289;
static tables_t tables_buf = {0xABEF1289, 0, {}, {}};
static settings_t settings_buf = {0};
static uint32_t counters[EEPROM_TABLES_SIZE / EEPROM_TABLES_PAGE_SIZE];

#define openSettingsFS SPIEepromFileOpen(&settingsFile, &eeSettingsCfg, EepromFindDevice(EEPROM_DEV_25XX)
#define openTablesFS SPIEepromFileOpen(&tablesFile, &eeTablesCfg, EepromFindDevice(EEPROM_DEV_25XX))
#endif

#ifdef MTL_APP
version_t versions[2] = {{0, 0, 0, 0},
                         {VERSION_PROTOCOL, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH}};

// Default settings
settings_t settings = {
    8500, // knockFreq
    3000, // knockRatio
    500,  // tpsMinV
    4500, // tpsMaxV
    3,    // fuelMinTh
    3,    // fuelMaxChange
    70,   // AfrMinVal*10
    220,  // AfrMaxVal*10
    0,    // AfrOffset
    0,    // sensorsInput
    0,    // afrInput
    0,    // reserved1
    0,    // reserved2
    0     // reserved3
};
#else
version_t versions[2] = {{VERSION_PROTOCOL, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH},
                         {0, 0, 0, 0}};
#endif

void eeInit(void)
{
    spiStart(&EEPROM_SPID, &EEPROM_SPIDCONFIG);
}

#ifdef MTL_APP
CCM_FUNC uint32_t eeFindCurrentPageAddr(void)
{
    uint32_t addr, magic, max_counter = 0;
    uint8_t i = 0;
    const uint32_t from = EEPROM_TABLES_FIRST;
    const uint32_t to = from+EEPROM_TABLES_SIZE;
    msg_t status;

    tablesFS = SPIEepromFileOpen(&tablesFile, &eeTablesCfg, EepromFindDevice(EEPROM_DEV_25XX));

    for (addr=from; addr < to; addr += EEPROM_TABLES_PAGE_SIZE)
	{
        fileStreamSeek(tablesFS, addr);

        status = fileStreamRead(tablesFS, (uint8_t*)&magic, sizeof magic);
        if (status != sizeof magic || magic != magic_key)
		{
            continue;
		}

        status = fileStreamRead(tablesFS, (uint8_t*)&counters[i], sizeof counters[0]);
        if (status != sizeof counters[0])
        {
            counters[i] = 0;
            continue;
        }

        i++;
	}

    // Find highest counter
    addr = from;
    for (i = 0; i < (sizeof counters / sizeof counters[0]); i++)
    {
        if (counters[i] > max_counter)
        {
            max_counter = counters[i];
            addr = i * EEPROM_TABLES_PAGE_SIZE; // Move address to page with highest counter
        }
    }

    fileStreamClose(tablesFS);

    return addr;
}

CCM_FUNC uint32_t eeFindNextPageAddr(void)
{
    uint32_t addr = eeFindCurrentPageAddr();

	/* Check if at last page */
    if(addr >= EEPROM_TABLES_LAST)
	{
        addr = EEPROM_TABLES_FIRST;
	}
    else
    {
        addr += EEPROM_TABLES_PAGE_SIZE;
    }

	return addr;
}

CCM_FUNC uint32_t eeFindPrevPageAddr(void)
{
    uint32_t addr = eeFindCurrentPageAddr();

    /* Check if at first page */
    if(addr == EEPROM_TABLES_FIRST)
    {
        addr = EEPROM_TABLES_LAST;
    }
    else
    {
        addr -= EEPROM_TABLES_PAGE_SIZE;
    }

    return addr;
}

CCM_FUNC uint8_t eePushPage(int32_t addr, uint8_t *buffer, crc_t *crc, uint32_t len)
{
    // Check max length
    if (len > EEPROM_TABLES_PAGE_SIZE-sizeof(crc_t))
        return 1;

    tablesFS = SPIEepromFileOpen(&tablesFile, &eeTablesCfg, EepromFindDevice(EEPROM_DEV_25XX));
    *crc = getCrc(buffer, len);

    fileStreamSeek(tablesFS, addr);
    if (fileStreamWrite(tablesFS, buffer, len) != len)
    {
        fileStreamClose(tablesFS);
        return 2;
    }

    // CRC at the end of the page
    fileStreamSeek(tablesFS, addr+EEPROM_TABLES_PAGE_SIZE-sizeof(crc_t));
    if (fileStreamWrite(tablesFS, (uint8_t*)crc, sizeof(crc_t)) != sizeof(crc_t))
    {
        fileStreamClose(tablesFS);
        return 3;
    }

    fileStreamClose(tablesFS);
    return 0;
}

CCM_FUNC uint8_t eePullPage(int32_t addr, uint8_t *buffer, crc_t *crc, uint32_t len)
{
    // Check max length
    if (len > EEPROM_TABLES_PAGE_SIZE-sizeof(crc_t))
        return 1;

    tablesFS = SPIEepromFileOpen(&tablesFile, &eeTablesCfg, EepromFindDevice(EEPROM_DEV_25XX));

    fileStreamSeek(tablesFS, addr);
    if (fileStreamRead(tablesFS, buffer, len) != len)
    {
        fileStreamClose(tablesFS);
        return 2;
    }

    // CRC at the end of the page
    fileStreamSeek(tablesFS, addr+EEPROM_TABLES_PAGE_SIZE-sizeof(crc_t));
    if (fileStreamRead(tablesFS, (uint8_t*)crc, sizeof(crc_t)) != sizeof(crc_t))
    {
        fileStreamClose(tablesFS);
        return 4;
    }

    fileStreamClose(tablesFS);
    return 0;
}

CCM_FUNC uint8_t readTablesFromEE(void)
{
    crc_t crc1, crc2 = 0;
    size_t len = sizeof(tables_buf);

    if (eePullPage(eeFindCurrentPageAddr(), (uint8_t*)&tables_buf, &crc1, len) != 0)
        return 1;

    crc2 = getCrc((uint8_t*)&tables_buf, len);

    if (crc1 != crc2)
    {
        // Try previous page
        if (eePullPage(eeFindPrevPageAddr(), (uint8_t*)&tables_buf, &crc1, len) != 0)
            return 2;
        crc2 = getCrc((uint8_t*)&tables_buf, len);
        if (crc1 != crc2)
            return 3;
    }

    memcpy(tables_buf.afr, tableAFR, sizeof tableAFR);
    memcpy(tables_buf.knock, tableKnock, sizeof tableKnock);

    return 0;
}

CCM_FUNC uint8_t writeTablesToEE(void)
{
    crc_t crc1 = 0, crc2 = 0;
    uint8_t res1, res2;
    size_t len =  sizeof(tables_buf);
    memcpy(tableAFR, tables_buf.afr, sizeof tableAFR);
    memcpy(tableKnock, tables_buf.knock, sizeof tableKnock);

    tables_buf.magic = magic_key;
    tables_buf.cnt++;

    res1 = eePushPage(eeFindNextPageAddr(), (uint8_t*)&tables_buf, &crc1, len);
    res2 = eePullPage(eeFindCurrentPageAddr(), (uint8_t*)&tables_buf, &crc2, len);

    // return 0 if write, read and crc comparison are successful
    if (res1 == 0 && res2 == 0 && crc1 == crc2)
        return 0;

    return 1;
}

CCM_FUNC uint8_t readSettingsFromEE()
{
    crc_t crc1 = 0, crc2 = 0;
    const size_t len = sizeof settings;
    settingsFS = SPIEepromFileOpen(&settingsFile, &eeSettingsCfg, EepromFindDevice(EEPROM_DEV_25XX));

    fileStreamSeek(settingsFS, 0);
    if (fileStreamRead(settingsFS, (uint8_t*)&settings_buf, len) != len)
    {
        fileStreamClose(settingsFS);
        return 1;
    }

    // Fetch CRC from EE
    if (fileStreamRead(settingsFS, (uint8_t*)&crc1, sizeof crc1) != sizeof crc1)
    {
        fileStreamClose(settingsFS);
        return 2;
    }

    // Calculate CRC from EE data
    crc2 = getCrc((uint8_t*)&settings_buf, len);

    if (crc1 != crc2)
    {
        fileStreamClose(settingsFS);
        return 3;
    }

    fileStreamClose(settingsFS);
    memcpy(&settings, &settings_buf, len);

    return 0;
}

CCM_FUNC uint8_t writeSettingsToEE()
{
    const size_t len = sizeof settings;
    crc_t crc = 0;
    memcpy(&settings_buf, &settings, len);
    crc = getCrc((uint8_t*)&settings_buf, len);

    settingsFS = SPIEepromFileOpen(&settingsFile, &eeSettingsCfg, EepromFindDevice(EEPROM_DEV_25XX));

    fileStreamSeek(settingsFS, 0);
    // Copy data to EE
    if (fileStreamWrite(settingsFS, (uint8_t*)&settings_buf, len) != len)
    {
        fileStreamClose(settingsFS);
        return 1;
    }

    // Copy CRC to EE
    if (fileStreamWrite(settingsFS, (uint8_t*)&crc, sizeof crc) != sizeof crc)
    {
        fileStreamClose(settingsFS);
        return 2;
    }

    fileStreamClose(settingsFS);
    return 0;
}
#endif

CCM_FUNC uint8_t readVersionFromEE(uint8_t idx, version_t* dst)
{
    crc_t crc1 = 0, crc2 = 0;
    const size_t len = sizeof(version_t);
    versionsFS = SPIEepromFileOpen(&versionsFile, &eeVersionsCfg, EepromFindDevice(EEPROM_DEV_25XX));

    fileStreamSeek(versionsFS, (len + sizeof(crc_t)) * idx);
    if (fileStreamRead(versionsFS, (uint8_t*)&version_buf, len) != len)
    {
        fileStreamClose(versionsFS);
        return 1;
    }

    // Fetch CRC from EE
    if (fileStreamRead(versionsFS, (uint8_t*)&crc1, sizeof crc1) != sizeof crc1)
    {
        fileStreamClose(versionsFS);
        return 2;
    }

    // Calculate CRC from EE data
    crc2 = getCrc((uint8_t*)&version_buf, len);

    if (crc1 != crc2)
    {
        fileStreamClose(versionsFS);
        return 3;
    }

    fileStreamClose(versionsFS);
    memcpy(dst, &version_buf, len);

    return 0;
}

CCM_FUNC uint8_t writeVersionToEE(uint8_t idx, const version_t* src)
{
    const size_t len = sizeof(version_t);
    crc_t crc = 0;
    memcpy(&version_buf, src, len);
    crc = getCrc((uint8_t*)&version_buf, len);

    versionsFS = SPIEepromFileOpen(&versionsFile, &eeVersionsCfg, EepromFindDevice(EEPROM_DEV_25XX));

    fileStreamSeek(versionsFS, (len + sizeof(crc_t)) * idx);
    // Copy data to EE
    if (fileStreamWrite(versionsFS, (uint8_t*)&version_buf, len) != len)
    {
        fileStreamClose(versionsFS);
        return 1;
    }

    // Copy CRC to EE
    if (fileStreamWrite(versionsFS, (uint8_t*)&crc, sizeof crc) != sizeof crc)
    {
        fileStreamClose(versionsFS);
        return 2;
    }

    fileStreamClose(versionsFS);
    return 0;
}
