#ifndef DAQ_IPC_CONTINUOUS_READ_THREAD_H
#define DAQ_IPC_CONTINUOUS_READ_THREAD_H

#include <libdaq/device/base_device.h>
#include <QSharedPointer>
#include <QRunnable>

using libdaq::device::LockzhinerBaseDevice;

class ContinuousReadADCThread : public QObject, public QRunnable {
Q_OBJECT

public:
  ContinuousReadADCThread(const QSharedPointer<LockzhinerBaseDevice> &base_device,
                          int channel, int data_length,
                          const std::shared_ptr<std::vector<float>> &buffer)
    : base_device_{base_device}, channel_{channel}, data_length_(data_length), buffer_{buffer} {}

protected:
  void run() override;
  bool CheckDataEmpty();
  bool CheckTimeout(const std::chrono::steady_clock::time_point &start, const int timeout_msec);  // 检查是否超时

signals:
  void complete(int channel);


private:
  QWeakPointer<LockzhinerBaseDevice> base_device_; // 设备示例
  int channel_; // 当前接受的是哪个通道的数据
  int data_length_; // 数据长度
  std::weak_ptr<std::vector<float>> buffer_;
  int timeout_msec_ = 500; // 数据超时时间 单位：ms
};

#endif //DAQ_IPC_CONTINUOUS_READ_THREAD_H
