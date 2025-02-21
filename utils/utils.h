#ifndef DAQ_IPC_UTILS_H
#define DAQ_IPC_UTILS_H

#include <QString>
#include <QSharedPointer>
#include <string>
#include <vector>
#include "libdaq/device/DAQ125/DAQ125.h"
#include "libdaq/device/base_device.h"

QString SampleRateIntToQString(int sample_rate);
int QStringToSampleRate(const QString &str);

bool String2Mac(const std::string& str, std::vector<uint8_t>& mac_address);
bool Mac2String(const std::vector<uint8_t>& mac_address, std::string& str);

bool IpAddress2String(const std::vector<uint8_t>& address, std::string& str);
bool String2IpAddress(const std::string& str, std::vector<uint8_t>& address);

double QStringToVoltage(const QString& str);

enum class DataFileFormat {
  NONE = -1,
  BIN = 0,
  TXT,
  CSV,
};

QString GenerateRandomMacAddress();
QString DecimalToHex(int decimalValue);
int HexToDecimal(const QString &hexString);

QSharedPointer<libdaq::device::LockzhinerBaseDevice> NewDevice();

bool IsIPAddress(const QString &str);
QString Uint8ToString(const uint8_t &value);

#endif //DAQ_IPC_UTILS_H
