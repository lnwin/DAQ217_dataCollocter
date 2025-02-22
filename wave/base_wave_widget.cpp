#include "./base_wave_widget.h"

BaseWaveWidget::BaseWaveWidget(QWidget *parent)
  : QCustomPlot(parent) {
  // 设置X轴
  xAxis->setVisible(true); // 显示下方x轴
  //  xAxis->setTickLabels(false);        // 设置不显示x轴刻度
  xAxis->ticker()->setTickCount(10); // 设置刻度为10格
  xAxis->setRange(0, 1);
  xAxis2->setVisible(true); // 显示上方x轴
  xAxis2->setTickLabels(false); // 设置不显示x轴刻度
  xAxis2->ticker()->setTickCount(10); // 设置刻度为10格
  xAxis2->setRange(0, 1);
  xAxis2->setLabel(QStringLiteral("时间(s)"));

  // 设置Y轴
  yAxis->setVisible(true); // 显示左边y轴
  //  yAxis->setTickLabels(false);        // 设置不显示y轴刻度
  yAxis->ticker()->setTickCount(10); // 设置刻度为10格
  yAxis->setRange(-1, 1);
  yAxis2->setVisible(true); // 显示右边y轴
  yAxis2->setTickLabels(false); // 设置不显示x轴刻度
  yAxis2->ticker()->setTickCount(10); // 设置刻度为10格
  yAxis2->setRange(-1, 1);
  yAxis2->setLabel(QStringLiteral("电压(V)"));

  // 设置图例
  legend->setVisible(true); // 设置图例可见
  legend->setBrush(QColor(255, 255, 255, 0)); // 图例背景透明

  setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}

void BaseWaveWidget::InitChannel(int channel_number, const QVector<Qt::GlobalColor> &channel_color) {
  channel_color_ = channel_color;
  channel_number_ = channel_number;
  for (int var = 0; var < channel_number_; ++var) {
    addGraph();
    graph(var)->setPen(QPen(channel_color_.at(var), 1));
    graph(var)->setName("AIN " + QString::number(var + 1));
  }
  replot();
}

void BaseWaveWidget::InitChannelState(const QVector<bool> &channel_state) {
  if(channel_state.size() != channel_number_){
    qWarning() << "channel_state.size() must be equal to channel_number_. But channel_state.size() is "
    << channel_state.size() << " and channel_number_ is " << channel_number_;
  }
  for (int var = 0; var < channel_number_; ++var) {
    graph(var)->setVisible(channel_state[var]);
    legend->item(var)->setVisible(channel_state[var]);
  }
  replot();
}

void BaseWaveWidget::SetSampleRate(int sample_rate) {
  sample_rate_ = sample_rate;
}


void BaseWaveWidget::AddData(int channel, double temp_data) {

  if (channel >= channel_number_ || channel < 0) {
    qWarning() << "Invalid channel number " << channel << ". max channel_number is " << channel_number_;
    return;
  }
  auto data_index = graph(channel)->data()->size();
  if(data_index > max_wave_length_){
    qInfo() << "Wave length is too long. Clear wave data.";
    Clear(channel);
  }
  data_index = graph(channel)->data()->size();
  graph(channel)->addData(static_cast<double>(data_index) / sample_rate_, temp_data);

}

void BaseWaveWidget::Clear() {
  for (int var = 0; var < channel_number_; ++var) {
    graph(var)->data().data()->clear();
  }
}

void BaseWaveWidget::Clear(int channel) {
  if (channel >= channel_number_ || channel < 0) {
    qWarning() << "Invalid channel number " << channel << ". max channel_number is " << channel_number_;
    return;
  }
  graph(channel)->data().data()->clear();
}
