#include"continuous_read_thread.h"
#include <QDebug>

void ContinuousReadADCThread::run() {
  // 超时时间 = 数据长度 / 采样率
  timeout_msec_ =
    (static_cast<double>(data_length_) / base_device_.toStrongRef()->GetCurrentSampleRate() * 1000) + 500;

  const auto base_device = base_device_.toStrongRef();

  int index = 0;
  auto buffer = buffer_.lock();

  // 上次读取数据的时间
  auto buffer_data_time_last = std::chrono::steady_clock::now();
  while (true) {
    if (base_device->TryReadADCData(channel_, buffer->data() + index)) {
      index++;
      buffer_data_time_last = std::chrono::steady_clock::now();
      if (index >= buffer->capacity()) {
        break;
      }
      continue;
    }

    if (!CheckDataEmpty()
       && CheckTimeout(buffer_data_time_last, timeout_msec_)) {
      break;
    }
  }
  buffer->resize(index);
  qDebug() << "channel " << channel_ << " data size = " << index;
  emit complete(channel_);
}

bool ContinuousReadADCThread::CheckDataEmpty() {
  return buffer_.lock()->empty();
}

bool ContinuousReadADCThread::CheckTimeout(const std::chrono::steady_clock::time_point &start, const int timeout_msec) {
  auto diff = std::chrono::steady_clock::now() - start;
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(diff);

  if (duration.count() >= timeout_msec) {
    // 超时
    return true;
  }

  return false;
}
