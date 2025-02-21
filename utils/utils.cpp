#include <QByteArray>
#include <QException>
#include <QFile>
#include <QIODevice>
#include <QProgressDialog>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QString>
#include <vector>
#include "utils/utils.h"
#include "libdaq/device/DAQ217/DAQ217.h"


QString SampleRateIntToQString(const int sample_rate) {
  int temp = sample_rate;
  QString unit = "";
  while (temp >= 1000) {
    temp /= 1000;
    if (unit == "") {
      unit = "K";
    } else if (unit == "K") {
      unit = "M";
    }
  }
  return QString::number(temp) + unit;
}

int QStringToSampleRate(const QString &str) {
  int temp = str.left(str.length() - 1).toInt(); // Extract the numeric part
  QString unit = str.right(1); // Extract the unit

  if (unit == 'K') {
    temp *= 1000;
  } else if (unit == 'M') {
    temp *= 1000000;
  } else {
    // 小于1k
    temp *= 10;
  }

  return temp;
}




bool String2Mac(const std::string &str, std::vector<uint8_t> &mac_address) {
    if (str.size() != 17) {
        return false;
    }

    QString qStr = QString::fromStdString(str);

    // 使用正则表达式匹配MAC地址格式
    QRegularExpression re("([a-fA-F0-9]{2}[:-]){5}[a-fA-F0-9]{2}");
    QRegularExpressionMatch match = re.match(qStr);

    if (!match.hasMatch()) {
        return false;
    }

    // 提取并转换每个十六进制数到uint8_t
    QStringList parts = qStr.split(':', Qt::SkipEmptyParts);
    mac_address.reserve(6); // Reserve space to avoid reallocations

    for (const QString &part : parts) {
        bool ok;
        uint8_t value = part.toUInt(&ok, 16);
        if (!ok) { // Conversion to uint8_t failed
            return false;
        }
        mac_address.push_back(value);
    }

    return true;
}


bool Mac2String(const std::vector<uint8_t> &mac_address, std::string &str) {
  int value_size = 6;
  if (mac_address.size() != value_size) {
    return false;
  }

  int str_size = 17;
  char c_str[20];
  memset(c_str, 0, sizeof(c_str));
  snprintf(c_str, sizeof(c_str), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_address[0],
           mac_address[1],
           mac_address[2],
           mac_address[3],
           mac_address[4],
           mac_address[5]);
  str = std::string(c_str);
  return true;
}


bool IpAddress2String(const std::vector<uint8_t> &address, std::string &str) {
  int address_size = 4;
  if (address.size() != address_size) {
    return false;
  }

  char str_buffer[128];
  memset(str_buffer, 0, sizeof(str_buffer));
  snprintf(str_buffer, sizeof(str_buffer), "%d.%d.%d.%d", address[0], address[1], address[2], address[3]);
  str = std::string(str_buffer);
  return true;
}

bool String2IpAddress(const std::string &str, std::vector<uint8_t> &address) {
  QString s = QString::fromStdString(str);
  const QStringList parts = s.split(".", Qt::SkipEmptyParts);
  if (parts.size() != 4) {
    return false;
  }

  address.resize(4);
  for (int i = 0; i < 4; ++i) {
    bool ok;
    uint8_t byteValue = parts[i].toUInt(&ok);
    if (!ok || byteValue > 255) { // 验证每个部分是否为有效的0-255之间的数字
      return false;
    }
    address[i] = byteValue;
  }
  return true;
}

double QStringToVoltage(const QString &str) {
  // 定义正则表达式，匹配 "±开头，随后是1或多个数字，或小数点及小数部分，以V结尾"
  QRegularExpression regex("±(\\d+\\.?\\d*)V");

  // 尝试匹配整个字符串
  QRegularExpressionMatch match = regex.match(str);

  if (!match.hasMatch()) {
    return -1;
  }

  // 从匹配结果中提取数字部分（第一个括号内的内容）
  QString voltageStr = match.captured(1);

  // 将提取的字符串转换为浮点数
  bool conversionOk;
  double voltage = voltageStr.toDouble(&conversionOk);

  if (conversionOk) {
    // 如果需要返回整数，可以将浮点数转换为整数
    return static_cast<int>(voltage); // 去掉小数部分，返回整数
  } else {
    // 数字转换失败，理论上不应该发生，因为正则已确保匹配到的是数字
    return -1; // 或者抛出异常，表示无法转换
  }

}


QString GenerateRandomMacAddress() {
    QString macAddress;

    for (int i = 0; i < 6; ++i) {
        // 生成一个0到255之间的随机数，即一个字节的范围
        quint8 byte = QRandomGenerator::global()->generate() % 256;

        // 将字节转换为两位的十六进制字符串，并添加到macAddress
        macAddress.append(byte < 16 ? "0" : "");
        macAddress.append(macAddress.number(byte, 16).toUpper());

        // 在每个字节之间插入冒号，除了最后一个字节
        if (i < 5) {
            macAddress.append(":");
        }
    }

    return macAddress;
}

QString DecimalToHex(int decimalValue)
{
    // 使用QString的toHex()方法，配合QString::number()来转换10进制到16进制
    // hex: 表示输出为16进制
    // uppercase: 转换后的16进制字符为大写
    // 省略第二个参数默认为无前缀
    return QString::number(decimalValue, 16).toUpper();
}


int HexToDecimal(const QString &hexString)
{
    bool ok;
    int decimalValue = hexString.toInt(&ok, 16); // 尝试将字符串以16进制格式转换为十进制

    if (!ok) {
        // 如果转换失败（即输入的字符串不是有效的16进制数）
        // 可以选择抛出异常、返回错误代码或默认值等处理方式
        return 0; // 这里简单返回0作为错误处理
    }

    return decimalValue; // 成功转换后返回十进制值
}

QSharedPointer<libdaq::device::LockzhinerBaseDevice> NewDevice(){
  return QSharedPointer<libdaq::device::DAQ217>::create();
}

bool IsIPAddress(const QString &str) {
  // 定义IP地址的正则表达式
  QRegularExpression ipRegex(
    "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"
  );

  // 进行匹配
  QRegularExpressionMatch match = ipRegex.match(str);

  // 返回匹配结果
  return match.hasMatch();
}

QString Uint8ToString(const uint8_t &value) {
  return QString("0x%1").arg(value, 4, 16, QChar('0'));
}
