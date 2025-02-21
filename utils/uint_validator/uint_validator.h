#ifndef UINT_VALIDATOR_H
#define UINT_VALIDATOR_H

#include <QValidator>
#include <limits>

class UIntValidator : public QValidator {
Q_OBJECT

public:
  explicit UIntValidator(quint64 minimum = std::numeric_limits<quint64>::min(),
                         quint64 maximum = std::numeric_limits<quint64>::max(),
                         QObject *parent = nullptr);

  State validate(QString &input, int &pos) const override;

protected:
  quint64 min_, max_; // 最小值，最大值
};


#endif // UINT_VALIDATOR_H
