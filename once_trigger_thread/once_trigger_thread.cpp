﻿#include "once_trigger_thread.h"
#include <QApplication>
#include <QDebug>

void OnceTriggerADCThread::start(Priority priority) {
  thread_is_running_ = true;
  QThread::start(priority);

  const auto base_device = base_device_.toStrongRef();
  collect_data_length_ = base_device->GetCurrentDataLength();
  qDebug()<<"collect_data_length_==="<<collect_data_length_;
  qDebug()<<"channel_==="<<channel_;
  data_.clear();
  data_.reserve(collect_data_length_);


}

void OnceTriggerADCThread::quit() {
  thread_is_running_ = false;

  QThread::quit();
}

void OnceTriggerADCThread::run() {
  qInfo(u8"OnceTriggerADCThread::run() start");
  const auto base_device = base_device_.toStrongRef();

  while (thread_is_running_) {
    auto temp_data = std::make_unique<float[]>(collect_data_length_);
    bool success = base_device->TryReadADCData(channel_, temp_data.get(), collect_data_length_);

    if(!thread_is_running_){
      break;
    }
     qDebug()<<"base_device->TryReadADCData";
    if (!success) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      continue;
    }
     qDebug()<<"TryReadADCData==success";
  //  qInfo("channel %d data size: %d", channel_, collect_data_length_);
    for (int j = 0; j < collect_data_length_; j++) {
      data_.emplace_back(temp_data[j]);
    }

    if(!CheckDataEmpty())
    {
      emit complete(channel_);
      qDebug()<<"emit complete(channel_)====="<<channel_;
      // 获取到足够的数据后，清空多余的数据
      base_device->ClearDataBuffer(channel_);
      break;
    }
  }

  qInfo(u8"OnceTriggerADCThread::run() end");
}

bool OnceTriggerADCThread::CheckDataEmpty() {
  return data_.empty();
}
