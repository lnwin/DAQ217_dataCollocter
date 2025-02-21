#include "uint_validator.h"


// 添加默认构造参数，允许在构造时设置范围
UIntValidator::UIntValidator(quint64 minimum, quint64 maximum, QObject *parent)
  : QValidator(parent), min_(minimum), max_(maximum) {
  // 确保最小值不大于最大值
  if (min_ > maximum) {
    qSwap(min_, max_);
  }
}

QValidator::State UIntValidator::validate(QString &input, int &pos) const {
  // 空输入是合法的，可以允许用户逐步输入
  if (input.isEmpty()) {
    return QValidator::Intermediate;
  }

  // 检查输入是否为有效的无符号整数
  bool ok;
  quint64 value = input.toULongLong(&ok);

  if (!ok) {
    return QValidator::Invalid;
  }

  // 只有当范围被明确设定时，才进行范围检查和修正
  if (min_ != std::numeric_limits<quint64>::min() && value < min_) {
    input = QString::number(min_);
    pos = 0; // 设置光标位置到输入的开始
    return QValidator::Intermediate;
  } else if (max_ != std::numeric_limits<quint64>::max() && value > max_) {
    input = QString::number(max_);
    pos = 0; // 设置光标位置到输入的开始
    return QValidator::Intermediate;
  }

  return QValidator::Acceptable;
}