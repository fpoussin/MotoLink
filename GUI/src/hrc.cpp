#include "hrc.h"

Hrc::Hrc(QObject *parent) : QObject(parent) {
  mHasMap = false;
  msError = tr("Error: ");

  const int defaultRpm[] = {0,     2000,  4000,  5500,  7000,  8000,
                            9000,  10000, 11000, 12000, 13000, 14000,
                            15000, 16000, 16500, 18000};

  const int defaultTps[] = {0, 2, 5, 7, 12, 25, 35, 50, 70, 85, 100};

  for (uint i = 0; i < sizeof(defaultRpm) / sizeof(int); i++) {
    mDefaultRpm.append(defaultRpm[i]);
  }

  for (uint i = 0; i < sizeof(defaultTps) / sizeof(int); i++) {
    mDefaultTps.append(defaultTps[i]);
  }
}

int Hrc::getDefaultRpmAt(int index) { return mDefaultRpm.at(index); }

int Hrc::getDefaultTpsAt(int index) { return mDefaultTps.at(index); }

bool Hrc::openFile(const QString &filename) {
  QFile file(filename);

  if (!file.open(QIODevice::ReadOnly)) {
    qWarning() << msError << tr("Couldn't open ") << filename;
    file.close();
    return false;
  }

  mFileContent = file.readAll();
  file.close();

  if (!this->hexToArray(&mFileContent, &mMapArray)) {
    return false;
  }

  mHasMap = true;
  return this->checkMapType();
}

bool Hrc::saveFile(const QString &filename) {
  if (!mHasMap) {
    qWarning() << msError << tr("No HRC map loaded");
    return false;
  }

  QFile file(filename);

  if (!file.open(QIODevice::WriteOnly)) {
    qWarning() << msError << tr("Couldn't open ") << filename;
    file.close();
    return false;
  }

  if (!this->arrayToHex(&mMapArray, &mFileContent)) {
    return false;
  }

  file.write(mFileContent.toLocal8Bit());
  file.close();

  return true;
}

bool Hrc::hexToArray(QString *hex, QByteArray *array) {
  QStringList stringList;
  QTextStream textStream(hex);
  QByteArray tmp_array;

  while (true) {
    QString line = textStream.readLine();
    if (line.isNull())
      break;
    else
      stringList.append(line);
  }

  /* each line */
  const QString *curLine;
  const uint size = stringList.size();
  bool ok;
  quint8 calculated_checksum;
  quint8 hex_checksum;
  for (uint i = 0; i < size; i++) {
    curLine = &stringList.at(i);
    QByteArray tmp_line;

    if (!curLine->startsWith(':')) {
      qWarning() << msError << tr("Line does not start with ':' at") << i;
      continue;
    }

    /* End of file */
    if (curLine->startsWith(HEX_EOF)) {
      break;
    }

    calculated_checksum = 0;
    for (uint j = 0; j < 4; j++) {
      calculated_checksum += curLine->mid(1 + (j * 2), 2).toInt(&ok, 16) & 0xFF;
      if (!ok) {
        qWarning() << msError << tr("Failed to parse line header")
                   << curLine->mid(1, 8);
      }
    }

    uint data_len = curLine->mid(1, 2).toInt(&ok, 16);
    if (!ok) {
      qWarning() << msError << tr("Failed to parse length")
                 << curLine->mid(1, 2);
      return false;
    }

    /* each byte - there should be 16 per line */
    for (uint j = 0; j < data_len; j++) {
      int cur_val = curLine->mid(9 + (j * 2), 2).toInt(&ok, 16) & 0xFF;
      calculated_checksum += cur_val;
      tmp_line.append(cur_val);
      if (!ok) {
        qWarning() << msError << tr("Failed to parse number")
                   << curLine->mid(1 + j, 2);
        return false;
      }
    }
    hex_checksum = curLine->mid(9 + (16 * 2), 2).toInt(&ok, 16) & 0xFF;
    calculated_checksum = 0x100u - calculated_checksum;
    if (!ok || calculated_checksum != hex_checksum) {
      qWarning() << msError << tr("Checksum failed at line") << i + 1;
      qWarning() << calculated_checksum << hex_checksum;
      return false;
    }
    tmp_array.append(tmp_line);
  }

  *array = tmp_array;

  return true;
}

bool Hrc::arrayToHex(QByteArray *array, QString *hex) {
  QString tmp_hex;
  QString cur_line;
  QString tmp_str;
  const uint step = 16;
  const uint lines = array->size() / step;
  quint16 address = 0;
  quint8 checksum;

  for (uint i = 0; i < lines; i++) {
    address = (i + 2) * step;
    checksum = step;
    checksum += address >> 8;
    checksum += address & 0xFF;

    cur_line = HEX_START;                              /* : */
    cur_line.append(tmp_str.sprintf("%02X", step));    /* Length */
    cur_line.append(tmp_str.sprintf("%04X", address)); /* Address */
    cur_line.append("00");                             /* Record type */

    for (uint j = 0; j < step; j++) {
      cur_line.append(
          tmp_str.sprintf("%02X", (array->at((step * i) + j) & 0xFF)));
      checksum += array->at((step * i) + j) & 0xFF;
    }
    cur_line.append(
        tmp_str.sprintf("%02X", (0x100 - checksum) & 0xFF)); /* Checksum */
    tmp_hex.append(cur_line + "\n");
  }
  tmp_hex.append(HEX_EOF);

  *hex = tmp_hex;
  return true;
}

bool Hrc::checkMapType(void) {
  const int data_size = mMapArray.size();

  /* First basic check */
  if (data_size == sizeof(cbr600rr07_map_t)) {
    cbr600rr07_map_t *tmp = (cbr600rr07_map_t *)mMapArray.data();

    /* Compare map signature, just like how the HRC soft does */
    if (strcmp(CBR600RR07_SIGN_STRING, tmp->signature) == 0) {
      qDebug() << tr("CBR600RR7 Map loaded");
      memcpy((void *)&mCbr600rr07_map, (void *)tmp, sizeof(cbr600rr07_map_t));
      return true;
    }
  }

  qWarning() << msError << tr("Unknown Map Type");
  return false;
}
