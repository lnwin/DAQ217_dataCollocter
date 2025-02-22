#include "scroll_mode_thread.h"
#include <QDebug>

void ScrollModeADCThread::start(Priority priority) {
  thread_is_running_ = true;
  QThread::start(priority);

  data_length_threshold_ = base_device_.toStrongRef()->GetCurrentDataLength();
  data_.resize(4);
  for (int i = 0; i < data_.size(); i++) {
    data_[i].reserve(2 * data_length_threshold_);
  }
}

void ScrollModeADCThread::quit() {
  thread_is_running_ = false;
  QThread::wait();
  QThread::quit();
}

void ScrollModeADCThread::run() {

  while (thread_is_running_) {

    bool success = ReadADCData();

    if (!success) {
      continue;
    }
      current_data_length_++;

    if (current_data_length_ == data_length_threshold_)
    {
      current_data_length_ = 0;
      emit SendData(data_);

      for (auto &i: data_) {
        i.clear();
        i.reserve(2 * data_length_threshold_);
      }
    }
  }
}

bool ScrollModeADCThread::ReadADCData() {
  bool success = false;
  const auto base_device = base_device_.toStrongRef();
  const auto channel_number = base_device->GetADCLegalChannel().size();
 // qDebug()<<"channel_number ="<<channel_number;
  for (int i = 0; i < channel_number; i++) {
    float temp_data = 0;
    if (!base_device->TryReadADCData(i, &temp_data)) {
      continue;
    }
    data_[i].emplace_back(temp_data);   
    success = true;
  }

  return success;
}
