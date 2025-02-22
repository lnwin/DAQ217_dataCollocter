#ifndef SCROLL_MODE_THREAD_H
#define SCROLL_MODE_THREAD_H

#include <QThread>
#include <QSharedPointer>
#include "libdaq/device/base_device.h"


class ScrollModeADCThread : public QThread
{
    Q_OBJECT
public:
    ScrollModeADCThread(const QSharedPointer<libdaq::device::LockzhinerBaseDevice> &base_device)
      : base_device_{base_device} {}

signals:
  void SendData(const std::vector<std::vector<float> > &data);

public Q_SLOTS:
  void start(Priority = InheritPriority);
  void quit();

protected:
  void run() override;
  bool ReadADCData();

private:
  std::vector<std::vector<float> > data_;  // 存储读取的数据
  std::vector<std::vector<float> > data_4Plot;  // 存储读取的数据
  std::atomic<bool> thread_is_running_ = false; // 线程是否运行
  QWeakPointer<libdaq::device::LockzhinerBaseDevice> base_device_; // 设备示例
  uint32_t data_length_threshold_ = 0; // 数据长度阈值，每次达到这个数据长度就往外发送数据
  uint32_t current_data_length_ = 0;  // 当前数据长度

};

#endif // SCROLL_MODE_THREAD_H
