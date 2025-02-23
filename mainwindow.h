#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QWidget"
#include "QTreeWidget"
#include "connect_page/connect_page.h"
#include "wave/base_wave_widget.h"
#include "libdaq/backend/usb/usb_backend.h"
#include "libdaq/backend/usb_server/usb_server.h"
#include "scroll_mode_thread/scroll_mode_thread.h"
#include <once_trigger_thread/once_trigger_thread.h>
#include <continuous_read_thread/continuous_read_thread.h>
#include <savemydate.h>
using DeviceAddress = uint8_t;
using DeviceMap = std::map<DeviceAddress, std::shared_ptr<libdaq::backend::LockzhinerUSBBackend> >;
using BusNumber = uint8_t;
using BusDeviceMap = std::map<BusNumber, DeviceMap>;
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void UpdatePlotData4Scroll(const std::vector<std::vector<float> > &data);
    void UpdatePlotData(int channel);
    void UpdateAutoScale();
private slots:
    void ConnectStatusSlot(bool status, ConnectPage::ConnectType type);
    void ClickConnectAction();
    void ClickStartADCCollectionAction();
    void ClickUpdateConfig();
    void ClickSoftwareTrigger();
    void ClickStopADCCollection();
    void UpdateAutoScale(bool open_scroll);
    void on_selectPath_clicked();
    void getMSG2m(QString);
private:
    void InitWaveWidget();
    void ReceiveADCData(int channel);
    void AutomaticCollection();
    void CommonCollection();
    void ScrollCollection();
    void readParameter();
    void EndCollection();
    void ContinuousCollectionUpdatePlotData();
private:
    bool usecount=false;
    int countTimeMax=0;
    int currentCountTime=0;
    int sampleNum;
    Ui::MainWindow *ui;
    saveMydate*mysaveMydate;
    QThread *mysaveMydateThread;
    std::vector<std::vector<float>> data_; // 数据
    std::vector<std::vector<float>> Mydatabuffer; // 缓存数据——保存用
    std::vector<std::vector<float>> count_data; // 累加缓存数据——保存用
    std::vector<std::vector<float>> count_data_common; // 累加缓存数据——保存用
    QVector<bool> channel_state_{true, true, true, true};
    QVector<bool> ADC_data_state_{true, true, true, true};
    QSharedPointer<ScrollModeADCThread> scroll_mode_adc_thread_; // 滚动模式采集数据线程
    QVector<QSharedPointer<OnceTriggerADCThread> > once_trigger_adc_thread_list_; // 触发采集数据线程 // 普通触发采集数据线程
    QSharedPointer<ConnectPage> connect_page_; // 连接页面
    QSharedPointer<libdaq::device::LockzhinerBaseDevice> base_device_;
    QSharedPointer<BaseWaveWidget> base_wave_widget_; // 波形显示控件
    std::shared_ptr<libdaq::backend::LockzhinerBaseBackend> base_backend_ = nullptr; // 服务端
    libdaq::backend::USBOption* usb_option_=nullptr; // USB属性
    std::unique_ptr<libdaq::backend::LockzhinerUSBServer> usb_server_; // USB服务器
    std::atomic<bool> is_start_adc_collection_ = false;  // 是否正在进行采集
    QVector<QCheckBox *> channel_checkbox_vector; // 校正通道复选框
    std::atomic<bool> is_roll_mode_ = false;  // 是否开启滚动模式
    std::atomic<bool> is_collecting = false;
    int MaxDataSaveLength=100000;
signals:
    void sendData2Save( const std::vector<std::vector<float>>& data);
};
#endif // MAINWINDOW_H
