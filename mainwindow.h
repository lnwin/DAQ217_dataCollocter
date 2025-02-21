#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include "QWidget"
#include "QTreeWidget"
#include "connect_page/connect_page.h"
#include "wave/base_wave_widget.h"
#include "libdaq/backend/usb/usb_backend.h"
#include "once_trigger_thread/once_trigger_thread.h"
#include "libdaq/backend/usb_server/usb_server.h"

using DeviceAddress = uint8_t;
using DeviceMap = std::map<DeviceAddress, std::shared_ptr<libdaq::backend::LockzhinerUSBBackend> >;
using BusNumber = uint8_t;
using BusDeviceMap = std::map<BusNumber, DeviceMap>;
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void readParameter();
private slots:
    void UpdatePlotData(int channel);
    void ConnectStatusSlot(bool status, ConnectPage::ConnectType type);
    void ClickConnectAction();
    void ClickStartADCCollectionAction();
    void ClickUpdateConfig();
    void ClickSoftwareTrigger();
    void ClickStopADCCollection();
    void on_pushButton_clicked();
    void on_selectPath_clicked();


private:
    void InitWaveWidget();
    void CommonTrigger(); // 普通采集
    void EndCollection();
private:
    Ui::MainWindow *ui;
    std::vector<std::vector<float>> data_; // 数据
    QVector<bool> channel_state_{true, true, true, true};
    QVector<bool> ADC_data_state_{true, true, true, true};
    QVector<QSharedPointer<OnceTriggerADCThread> > once_trigger_adc_thread_list_; // 触发采集数据线程
    QSharedPointer<ConnectPage> connect_page_; // 连接页面
    QSharedPointer<libdaq::device::LockzhinerBaseDevice> base_device_;
    QSharedPointer<BaseWaveWidget> base_wave_widget_; // 波形显示控件
    std::shared_ptr<libdaq::backend::LockzhinerBaseBackend> base_backend_ = nullptr; // 服务端
    libdaq::backend::USBOption* usb_option_=nullptr; // USB属性
    std::unique_ptr<libdaq::backend::LockzhinerUSBServer> usb_server_; // USB服务器
    std::atomic<bool> is_collecting = false;
};
#endif // MAINWINDOW_H
