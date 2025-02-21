#ifndef BASE_WAVE_WIDGET_H
#define BASE_WAVE_WIDGET_H

#include "./qcustomplot.h"

class BaseWaveWidget : public QCustomPlot {
  public:
    explicit BaseWaveWidget(QWidget *parent = nullptr);
    virtual ~BaseWaveWidget() override = default;

    // 初始化ADC通道个数以及通道颜色
    void InitChannel(int channel_number, const QVector<Qt::GlobalColor> &channel_color);
    void InitChannelState(const QVector<bool> &channel_state);
    void SetSampleRate(int sample_rate);

    void AddData(int channel, double temp_data);

    void Clear();
    void Clear(int channel);

  private:
    int channel_number_ = 0; // ADC通道个数
    int sample_rate_ = 1;
    int max_wave_length_ = 70000000; // 单通道最大采样点数
    QVector<Qt::GlobalColor> channel_color_; // 通道颜色

};

#endif //BASE_WAVE_WIDGET_H
