#ifndef ONCE_TRIGGER_THREAD_H
#define ONCE_TRIGGER_THREAD_H

#include <QThread>
#include <QSharedPointer>
#include "libdaq/device/base_device.h"

using namespace libdaq::device;

class OnceTriggerADCThread : public QThread {
  Q_OBJECT
public:
  OnceTriggerADCThread(const QSharedPointer<LockzhinerBaseDevice> &base_device,
                      int channel,
                      std::vector<float>& data)
    : base_device_{base_device}
    , channel_{channel}
    , data_{data}
    {}

signals:
  void complete(int channel);
    void sendMSG2m(QString);
public Q_SLOTS:
  void start(Priority = InheritPriority);
  void quit();

protected:
  void run() override;
  bool CheckDataEmpty();

private:
  int channel_ = 0;  // 采集第几个通道
  std::vector<float>& data_;  // 存储读取的数据
  std::atomic<bool> thread_is_running_ = false; // 线程是否运行
  QWeakPointer<LockzhinerBaseDevice> base_device_; // 设备示例
  int collect_data_length_ = 0; // 要采集的数据长度
};

#endif // ONCE_TRIGGER_THREAD_H
