#include <QEventLoop>
#include "MainWindow.h"
#include <QTimer>
#include <QPushButton>
#include "./ui_MainWindow.h"
#include "utils/utils.h"
#include "libdaq/device/base_device.h"
#include "connect_page/connect_page.h"
#include <QDebug>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // 创建设备
    base_device_ = NewDevice();
    connect(ui->Connnectbutton, &QPushButton::clicked, this, &MainWindow::ClickConnectAction);
    connect(ui->StartADCCollection, &QPushButton::clicked, this, &MainWindow::ClickStartADCCollectionAction);
    connect(ui->UpdateConfig, &QPushButton::clicked, this, &MainWindow::ClickUpdateConfig);
    connect(ui->OnceCollection, &QPushButton::clicked, this, &MainWindow::ClickSoftwareTrigger);
    connect(ui->StopADCCollection, &QPushButton::clicked, this, &MainWindow::ClickStopADCCollection);

    InitWaveWidget();
}

MainWindow::~MainWindow()
{
    EndCollection();
    delete ui;
}

void MainWindow::ConnectStatusSlot(bool status, ConnectPage::ConnectType type) {
    if (status) {
        if (!base_device_->InitializeDevice()) {
            qWarning(u8"初始化设备失败，请检查配置参数是否存在错误");
            return;
        }
        if (!base_device_->ConnectedDevice()) {
            qWarning(u8"发送连接设备命令失败");
            return;
        }
    }else{
        base_device_->DisconnectedDevice();
    }
}

void MainWindow::ClickConnectAction() {
    ConnectPage connect_page(base_device_, nullptr);
    // 初始化连接界面
    if (connect_page_.isNull()) {
        connect_page_.reset(new ConnectPage(base_device_));
        connect_page_->setModal(false);
        connect(connect_page_.get(), &ConnectPage::ConnectStatus, this, &MainWindow::ConnectStatusSlot);
    }
    connect_page_->InitParams(base_device_);
    connect_page_->show();
}

void MainWindow::ClickStartADCCollectionAction() {

    is_collecting = true;

    CommonTrigger();
    qInfo(u8"软件触发成功");
}
void MainWindow:: on_pushButton_clicked()
{
    readParameter();
    std::vector<LockzhinerADCVoltage> myADvoltageRanges;
    myADvoltageRanges =base_device_->GetADCCurrentVoltage();
    for (const auto& voltage : myADvoltageRanges)
    {
        std::cout << static_cast<int>(voltage) << std::endl;

    }
};
void MainWindow::readParameter()
{

    auto legal_sample_rate = base_device_->GetADCLegalSampleRate();
    //set sample rate========================================================
    switch (ui->sampleRate->currentIndex()) {
    case 0:
        base_device_->ConfigADCSampleRate(libdaq::device::LockzhinerADCSampleRate::ADCSampleRate_125_M);
        break;
    case 1:
        base_device_->ConfigADCSampleRate(libdaq::device::LockzhinerADCSampleRate::ADCSampleRate_100_M);
        break;
    case 2:
        base_device_->ConfigADCSampleRate(libdaq::device::LockzhinerADCSampleRate::ADCSampleRate_50_M);
        break;
    case 3:
        base_device_->ConfigADCSampleRate(libdaq::device::LockzhinerADCSampleRate::ADCSampleRate_20_M);
        break;
    case 4:
        base_device_->ConfigADCSampleRate(libdaq::device::LockzhinerADCSampleRate::ADCSampleRate_10_M);
        break;
    case 5:
        base_device_->ConfigADCSampleRate(libdaq::device::LockzhinerADCSampleRate::ADCSampleRate_5_M);
        break;
    case 6:
        base_device_->ConfigADCSampleRate(libdaq::device::LockzhinerADCSampleRate::ADCSampleRate_2_M);
        break;
    case 7:
        base_device_->ConfigADCSampleRate(libdaq::device::LockzhinerADCSampleRate::ADCSampleRate_1_M);
        break;
    case 8:
        base_device_->ConfigADCSampleRate(libdaq::device::LockzhinerADCSampleRate::ADCSampleRate_500_K);
        break;
    case 9:
        base_device_->ConfigADCSampleRate(libdaq::device::LockzhinerADCSampleRate::ADCSampleRate_200_K);
        break;
    case 10:
        base_device_->ConfigADCSampleRate(libdaq::device::LockzhinerADCSampleRate::ADCSampleRate_100_K);
        break;
    case 11:
        base_device_->ConfigADCSampleRate(libdaq::device::LockzhinerADCSampleRate::ADCSampleRate_50_K);
        break;
    case 12:
        base_device_->ConfigADCSampleRate(libdaq::device::LockzhinerADCSampleRate::ADCSampleRate_20_K);
        break;
    case 13:
        base_device_->ConfigADCSampleRate(libdaq::device::LockzhinerADCSampleRate::ADCSampleRate_10_K);
        break;
    case 14:
        base_device_->ConfigADCSampleRate(libdaq::device::LockzhinerADCSampleRate::ADCSampleRate_5_K);
        break;
    case 15:
        base_device_->ConfigADCSampleRate(libdaq::device::LockzhinerADCSampleRate::ADCSampleRate_2_K);
        break;
    case 16:
        base_device_->ConfigADCSampleRate(libdaq::device::LockzhinerADCSampleRate::ADCSampleRate_1_K);
        break;
    case 17:
        base_device_->ConfigADCSampleRate(libdaq::device::LockzhinerADCSampleRate::ADCSampleRate_500);
        break;
    case 18:
        base_device_->ConfigADCSampleRate(libdaq::device::LockzhinerADCSampleRate::ADCSampleRate_200);
        break;
    case 19:
        base_device_->ConfigADCSampleRate(libdaq::device::LockzhinerADCSampleRate::ADCSampleRate_100);
        break;
    default:
        break;
    }
    //set sample MaxValue====================================================
    std::vector<LockzhinerADCVoltage> myADvoltageRanges;
    myADvoltageRanges =base_device_->GetADCCurrentVoltage();
    switch (ui->CH1_MaxV->currentIndex()) {
    case 0:
        myADvoltageRanges[0]=libdaq::device::LockzhinerADCVoltage::ADCVoltage_1_V;
        break;
    case 1:
        myADvoltageRanges[0]=libdaq::device::LockzhinerADCVoltage::ADCVoltage_10_V;
        break;
    }
    switch (ui->CH2_MaxV->currentIndex()) {
    case 0:
        myADvoltageRanges[1]=libdaq::device::LockzhinerADCVoltage::ADCVoltage_1_V;
        break;
    case 1:
        myADvoltageRanges[1]=libdaq::device::LockzhinerADCVoltage::ADCVoltage_10_V;
        break;
    }
    switch (ui->CH3_MaxV->currentIndex()) {
    case 0:
        myADvoltageRanges[2]=libdaq::device::LockzhinerADCVoltage::ADCVoltage_1_V;
        break;
    case 1:
        myADvoltageRanges[2]=libdaq::device::LockzhinerADCVoltage::ADCVoltage_10_V;
        break;
    }
    switch (ui->CH4_MaxV->currentIndex()) {
    case 0:
        myADvoltageRanges[3]=libdaq::device::LockzhinerADCVoltage::ADCVoltage_1_V;
        break;
    case 1:
        myADvoltageRanges[0]=libdaq::device::LockzhinerADCVoltage::ADCVoltage_10_V;
        break;
    }
    base_device_->ConfigADCVoltage(myADvoltageRanges);
    //set chanel enable=====================================================
    int combination = (ui->enable_CH1->isChecked() ? 1 : 0) |
                      (ui->enable_CH2->isChecked() ? 2 : 0) |
                      (ui->enable_CH3->isChecked() ? 4 : 0) |
                      (ui->enable_CH4->isChecked() ? 8 : 0);
    switch (combination) {
    case 0:
       base_device_->ConfigADCChannel(0x00);
        break;
    case 1:
        base_device_->ConfigADCChannel(0x01);
        break;
    case 2:
       base_device_->ConfigADCChannel(0x02);
        break;
    case 3:
       base_device_->ConfigADCChannel(0x03);
        break;
    case 4:
       base_device_->ConfigADCChannel(0x04);
       break;
    case 5:
       base_device_->ConfigADCChannel(0x05);
       break;
    case 6:
       base_device_->ConfigADCChannel(0x06);
       break;
    case 7:
       base_device_->ConfigADCChannel(0x07);
       break;
    case 8:
       base_device_->ConfigADCChannel(0x08);
       break;
    case 9:
       base_device_->ConfigADCChannel(0x09);
       break;
    case 10:
       base_device_->ConfigADCChannel(0x0a);
       break;
    case 11:
       base_device_->ConfigADCChannel(0x0b);
       break;
    case 12:
       base_device_->ConfigADCChannel(0x0c);
       break;
    case 13:
       base_device_->ConfigADCChannel(0x0d);
       break;
    case 14:
       base_device_->ConfigADCChannel(0x0e);
       break;
    case 15:
       base_device_->ConfigADCChannel(0x0f);
        break;
    default:

        break;
    }
    //set trigger para======================================================
    LockzhinerADCTriggerType type;
    auto mode = static_cast<libdaq::device::LockzhinerADCTriggerMode>(libdaq::device::LockzhinerADCTriggerMode::Common);
    LockzhinerADCChannel channel;
    switch (ui->triggerMode->currentIndex())
    {
    case 0:
        type=static_cast<libdaq::device::LockzhinerADCTriggerType>(libdaq::device::LockzhinerADCTriggerType::Normal);
        break;
    case 1:
        type=static_cast<libdaq::device::LockzhinerADCTriggerType>(libdaq::device::LockzhinerADCTriggerType::RisingEdge);
        break;
    case 2:
        type=static_cast<libdaq::device::LockzhinerADCTriggerType>(libdaq::device::LockzhinerADCTriggerType::FallingEdge);
        break;
    case 3:
        type=static_cast<libdaq::device::LockzhinerADCTriggerType>(libdaq::device::LockzhinerADCTriggerType::DoubleEdge);
        break;
    }
    switch (ui->triggerCH->currentIndex())
    {
    case 0:
        channel=static_cast<libdaq::device::LockzhinerADCChannel>(libdaq::device::LockzhinerADCChannel::ADCChannelAIN1);
        channel_state_[0]=false;
        break;
    case 1:
        channel=static_cast<libdaq::device::LockzhinerADCChannel>(libdaq::device::LockzhinerADCChannel::ADCChannelAIN2);
        channel_state_[1]=false;
        break;
    case 2:
        channel=static_cast<libdaq::device::LockzhinerADCChannel>(libdaq::device::LockzhinerADCChannel::ADCChannelAIN3);
        channel_state_[2]=false;
        break;
    case 3:
        channel=static_cast<libdaq::device::LockzhinerADCChannel>(libdaq::device::LockzhinerADCChannel::ADCChannelAIN4);
        channel_state_[3]=false;
        break;
    }
    // 触发电平 0mv
    uint32_t level=ui->triggerlevel->text().toInt();
    if (!base_device_->ConfigADCTrigger(type, mode, channel, level))
    {
        qWarning(u8"DAQ控制器配置ADC触发失败");
    }
    int triggerLength=ui->triggerLength->text().toInt();
    if (!base_device_->ConfigADCTriggerLength(triggerLength)) {
        qWarning(u8"触发长度配置失败");
    }
    // 超时 100ms
    int triggerTimeOUT=ui->timeout->text().toInt();
    if (!base_device_->ConfigTriggerTimeout(triggerTimeOUT)) {
        qWarning(u8"超时时间配置失败");
    }
    // AC/DC设置为===========================================================================================
    int CHAC_DC =     ((ui->CH1_AC_DC->currentIndex()==1) ? 1 : 0) |
                      ((ui->CH2_AC_DC->currentIndex()==1) ? 2 : 0) |
                      ((ui->CH3_AC_DC->currentIndex()==1) ? 4 : 0) |
                      ((ui->CH4_AC_DC->currentIndex()==1) ? 8 : 0);
    switch (CHAC_DC) {
    case 0:
        base_device_->ConfigACDC(0x00);
        break;
    case 1:
        base_device_->ConfigACDC(0x01);
        break;
    case 2:
        base_device_->ConfigACDC(0x02);
        break;
    case 3:
        base_device_->ConfigACDC(0x03);
        break;
    case 4:
        base_device_->ConfigACDC(0x04);
        break;
    case 5:
        base_device_->ConfigACDC(0x05);
        break;
    case 6:
        base_device_->ConfigACDC(0x06);
        break;
    case 7:
        base_device_->ConfigACDC(0x07);
        break;
    case 8:
        base_device_->ConfigACDC(0x08);
        break;
    case 9:
        base_device_->ConfigACDC(0x09);
        break;
    case 10:
        base_device_->ConfigACDC(0x0a);
        break;
    case 11:
        base_device_->ConfigACDC(0x0b);
        break;
    case 12:
        base_device_->ConfigACDC(0x0c);
        break;
    case 13:
        base_device_->ConfigACDC(0x0d);
        break;
    case 14:
        base_device_->ConfigACDC(0x0e);
        break;
    case 15:
        base_device_->ConfigACDC(0x0f);
        break;
    default:

        break;
    }



};
void MainWindow::ClickUpdateConfig() {
    // 采样率 1M
    auto legal_sample_rate = base_device_->GetADCLegalSampleRate();
    if (!base_device_->ConfigADCSampleRate(libdaq::device::LockzhinerADCSampleRate::ADCSampleRate_1_M)) {
        qWarning(u8"采样率配置失败");
    }
    // 电压量程 10V
    if (!base_device_->ConfigADCVoltage(libdaq::device::LockzhinerADCVoltage::ADCVoltage_10_V)) {
        qWarning(u8"电压量程配置失败");
    }
    // 4通道使能
    if (!base_device_->ConfigADCChannel(0x0F)) {
        qWarning(u8"ADC4通道配置失败");
    }
    // 触发电平 0mv
    uint32_t level=0;
    // 双边缘触发 普通采集 普通触发
    auto type = static_cast<libdaq::device::LockzhinerADCTriggerType>(libdaq::device::LockzhinerADCTriggerType::DoubleEdge);
    auto mode = static_cast<libdaq::device::LockzhinerADCTriggerMode>(libdaq::device::LockzhinerADCTriggerMode::Common);
    auto channel = static_cast<libdaq::device::LockzhinerADCChannel>(libdaq::device::LockzhinerADCChannel::ADCChannelAINNone);
    if (!base_device_->ConfigADCTrigger(type, mode, channel, level)) {
        qWarning(u8"DAQ控制器配置ADC触发失败");
    }

    // 长度 50000个点
    if (!base_device_->ConfigADCTriggerLength(50000)) {
        qWarning(u8"触发长度配置失败");
    }
    // 超时 100ms
    if (!base_device_->ConfigTriggerTimeout(100)) {
        qWarning(u8"超时时间配置失败");
    }

    // AC/DC 设置为AC
    uint8_t enable = 0x00;
    if (!base_device_->ConfigACDC(enable)) {
        qWarning("配置AC/DC失败");
        return;
    }
    qInfo(u8"下发配置成功");
}

void MainWindow::ClickSoftwareTrigger()
{
    if(!base_device_->ConfigADCTriggerSource
         (static_cast<uint8_t>(libdaq::device::DAQADCTriggerSource::TriggerSourceSoftware))){
        qWarning(u8"软件触发失败");
        return;
    }
    qInfo(u8"软件触发成功");
}

void MainWindow::ClickStopADCCollection()
{
    is_collecting = false;
    if(!base_device_->StopADCCollection()){
        qWarning(u8"停止ADC采集失败");
        return;
    }

    EndCollection();

    qInfo(u8"停止ADC采集成功");
    UpdatePlotData(1);
}

void MainWindow::InitWaveWidget() {
    QVector<Qt::GlobalColor> channel_color_ = { // 通道颜色
        Qt::red, Qt::green, Qt::cyan, Qt::blue
    };
    base_wave_widget_ = QSharedPointer<BaseWaveWidget>::create();
    base_wave_widget_->InitChannel(4, channel_color_);
    ui->stackedWidget->addWidget(base_wave_widget_.get());
    ui->stackedWidget->setCurrentWidget(base_wave_widget_.get());
}

void MainWindow::CommonTrigger()
{
    if(!is_collecting){
        return;
    }

    EndCollection();

    if(!base_device_->StopADCCollection()){
        qWarning(u8"启动ADC采集失败");
        return;
    }

    QEventLoop loop;
    QTimer::singleShot(50, &loop, SLOT(quit()));
    loop.exec();

    if(!base_device_->StartADCCollection()){
        qWarning(u8"启动ADC采集失败");
        return;
    }

    data_.resize(4);

    for(int i = 0; i < channel_state_.size(); i++)
    {
        if(!channel_state_.at(i))
        {
            continue;
        }

        auto thread = QSharedPointer<OnceTriggerADCThread>::create(base_device_, i, data_[i]);
        connect(thread.get(), &OnceTriggerADCThread::complete, this, &MainWindow::UpdatePlotData);
        once_trigger_adc_thread_list_.push_back(thread);
        thread->start();
    }

}

void MainWindow::EndCollection()
{
    for(int i = 0; i < once_trigger_adc_thread_list_.size(); ++ i){
        auto& thread = once_trigger_adc_thread_list_[i];
        if(thread != nullptr){
            thread->quit();
            thread->wait();
            thread->deleteLater();
            thread = nullptr;
        }
    }
    once_trigger_adc_thread_list_.clear();
}

void MainWindow::UpdatePlotData(int channel)
{

    ADC_data_state_[channel] = true;

    // 等待所有通道数据接收完成再显示
    for (int var = 0; var < ADC_data_state_.size(); ++var) {
        // 当前通道有读取数据 但是数据还没返回 则等待
        if (!ADC_data_state_[var] && channel_state_[var]) {
            return;
        }
    }

    // 所有通道数据接收完成
    for (int var = 0; var < ADC_data_state_.size(); ++var) {
        ADC_data_state_[var] = false;
    }

    base_wave_widget_->Clear();
    double max_val = 0;
    double min_val = 0;
    // 添加数据到图表
    for (int channel = 0; channel < data_.size(); ++channel) {
        const auto &channel_data = data_.at(channel);
        if (channel_data.empty() || !channel_state_.at(channel)) {
            continue;
        }

        for (auto &value : channel_data) {
            base_wave_widget_->AddData(channel, value);
            if (value > max_val) {
                max_val = value;
            }
            if (value < min_val) {
                min_val = value;
            }
        }
    }
    // 更新图表
    base_wave_widget_->yAxis->setRange(min_val, max_val);
    base_wave_widget_->xAxis->setRange(0, 50000);
    base_wave_widget_->replot(QCustomPlot::rpQueuedReplot);
 ;
}
 void MainWindow::on_selectPath_clicked()
{

};
