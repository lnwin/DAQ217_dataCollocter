#include <QEventLoop>
#include "MainWindow.h"
#include "qtimer.h"
#include "qpushbutton.h"
#include "./ui_MainWindow.h"
#include "utils/utils.h"
#include "libdaq/device/base_device.h"
#include "connect_page/connect_page.h"
#include "scroll_mode_thread/scroll_mode_thread.h"
#include <memory>
#include <QFileDialog>

using namespace libdaq::device;
QThreadPool read_data_thread_pool_;  // 线程池
std::vector<std::shared_ptr<std::vector<float>>> ADC_Data_Buffer(4); // 连续采集数据缓存

std::vector<bool> ADCDataState = {false, false, false, false};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    qRegisterMetaType<std::vector<std::vector<float> > >("std::vector<std::vector<float>>");
    // 创建设备
    base_device_ = NewDevice();
    connect(ui->Connnectbutton, &QPushButton::clicked, this, &MainWindow::ClickConnectAction);
    connect(ui->StartADCCollection, &QPushButton::clicked, this, &MainWindow::ClickStartADCCollectionAction);
    connect(ui->UpdateConfig, &QPushButton::clicked, this, &MainWindow::ClickUpdateConfig);
    connect(ui->OnceCollection, &QPushButton::clicked, this, &MainWindow::ClickSoftwareTrigger);
    connect(ui->StopADCCollection, &QPushButton::clicked, this, &MainWindow::ClickStopADCCollection);


    mysaveMydate=new saveMydate;
    mysaveMydateThread=new QThread();
    mysaveMydate->moveToThread(mysaveMydateThread);
    mysaveMydateThread->start();
     connect(this, &MainWindow::sendData2Save, mysaveMydate, &saveMydate::getData2Save);
    InitWaveWidget();

}

MainWindow::~MainWindow() {
    delete ui;
     EndCollection();
    if (scroll_mode_adc_thread_ != nullptr) {
        scroll_mode_adc_thread_->quit();
        scroll_mode_adc_thread_->deleteLater();
        scroll_mode_adc_thread_ = nullptr;
    }
    currentCountTime=0;
}
std::vector<std::vector<float>> divideAndSum(const std::vector<std::vector<float>>&Mydatabuffer_count,int TT)
{
    std::vector<std::vector<float>> result;
    result.resize(4);


    for (int i = 0; i < Mydatabuffer_count.size(); ++i)
    {
        if(Mydatabuffer_count[i].empty())
        {
            continue;
        }
        // 计算每段的长度

        int partSize = Mydatabuffer_count[i].size()/TT;
        std::vector<float> FD(partSize);
        for (int var = 0; var < partSize; ++var)
        {
            for (size_t j = 0; j < TT; ++j)
            {
                FD[var]+=Mydatabuffer_count[i][var+partSize*j];
            }
        }
        result[i]=FD;
    }
    return result;
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
    is_start_adc_collection_ = true;
    if(ui->triggerType->currentIndex()==0)
    {
        ScrollCollection();
        qWarning(u8"开启AD采集成功！");

    }
    else
    {
        if(ui->triggerMode->currentIndex()==0)
        {
            AutomaticCollection();
            qWarning(u8"开启AD采集成功！");
        }
        else
        {
            is_collecting=true;

//            if(!base_device_->ConfigADCTriggerSource
//                 (static_cast<uint8_t>(libdaq::device::DAQADCTriggerSource::TriggerSourceSoftware))){
//                qWarning(u8"软件触发失败");
//                return;
//            }

            CommonCollection();
            qWarning(u8"开启CommonCollectionAD采集成功！");
        }


    }

}
void MainWindow::on_selectPath_clicked()
{
   QString mupath= QFileDialog::getExistingDirectory(this, "Select Directory");
    ui->saveFilePath->setText(mupath);

};
void MainWindow::readParameter()
{
    sampleNum=ui->sampleNum->text().toInt();
    countTimeMax=ui->countTime->text().toInt();
    Mydatabuffer.resize(4);
    count_data.resize(4);
    count_data_common.resize(4);
    MaxDataSaveLength=ui->maxFileLineNum->text().toInt();
    mysaveMydate->filePath=ui->saveFilePath->text();
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
    std::vector<libdaq::device::LockzhinerADCVoltage> myADvoltageRanges;
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
    libdaq::device::LockzhinerADCTriggerType type;
    libdaq::device::LockzhinerADCTriggerMode mode;
    if(ui->triggerMode->currentIndex()==0)
    {
        mode=libdaq::device::LockzhinerADCTriggerMode::Automatic;
    }
    else
    {
        mode=libdaq::device::LockzhinerADCTriggerMode::Common;
    }
    libdaq::device::LockzhinerADCChannel channel;
    switch (ui->triggerType->currentIndex())
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
       // channel_state_[0]=false;
        break;
    case 1:
        channel=static_cast<libdaq::device::LockzhinerADCChannel>(libdaq::device::LockzhinerADCChannel::ADCChannelAIN2);
        //channel_state_[1]=false;
        break;
    case 2:
        channel=static_cast<libdaq::device::LockzhinerADCChannel>(libdaq::device::LockzhinerADCChannel::ADCChannelAIN3);
       // channel_state_[2]=false;
        break;
    case 3:
        channel=static_cast<libdaq::device::LockzhinerADCChannel>(libdaq::device::LockzhinerADCChannel::ADCChannelAIN4);
        //channel_state_[3]=false;
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
    int CHAC_DC =   ((ui->CH1_AC_DC->currentIndex()==1) ? 1 : 0) |
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
    qInfo(u8"配置成功");

};
void MainWindow::ClickUpdateConfig()
{
    readParameter();
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

void clearMyBuff(std::vector<std::vector<float>>MB )
{
    for (int var = 0; var < 4; ++var)
    {
        const auto &data = MB[var];
        if (data.empty()) {
            continue;
        }
        MB[var].clear();
    }
};


void MainWindow::ClickStopADCCollection() {
    is_start_adc_collection_ = false;
    is_collecting=false;
    if (!base_device_->StopADCCollection()) {
        qWarning(u8"停止ADC采集失败");
        return;
    }
    if (scroll_mode_adc_thread_ != nullptr) {
        scroll_mode_adc_thread_->quit();
        scroll_mode_adc_thread_->deleteLater();
        scroll_mode_adc_thread_ = nullptr;
    }
    read_data_thread_pool_.waitForDone();
    qInfo(u8"停止ADC采集成功");  
    clearMyBuff(Mydatabuffer);
    clearMyBuff(count_data);
    clearMyBuff(count_data_common);

}

void MainWindow::InitWaveWidget() {
    QVector<Qt::GlobalColor> channel_color_ = { // 通道颜色
        Qt::red, Qt::green, Qt::cyan, Qt::blue
    };
    base_wave_widget_ = QSharedPointer<BaseWaveWidget>::create();
    base_wave_widget_->InitChannel(4, channel_color_); // 初始化通道数


    QVector<bool> state = {true, true, true, true};
    base_wave_widget_->InitChannelState(state); // 初始化各通道数据显示状态
    ui->stackedWidget->addWidget(base_wave_widget_.get());
    ui->stackedWidget->setCurrentWidget(base_wave_widget_.get());
}
void MainWindow::CommonCollection()
{

    if(!is_collecting){
        return;
    }
    is_collecting=true;

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
        if(!channel_state_.at(i)){
            continue;
        }

        auto thread = QSharedPointer<OnceTriggerADCThread>::create(base_device_, i, data_[i]);
        connect(thread.get(), &OnceTriggerADCThread::complete, this, &MainWindow::UpdatePlotData);
        connect(thread.get(), &OnceTriggerADCThread::sendMSG2m, this, &MainWindow::getMSG2m);
        once_trigger_adc_thread_list_.push_back(thread);
        thread->start();
    }

};

void MainWindow::getMSG2m(QString msg)
{
   // ui->textEdit->append(msg);
}
void MainWindow::ScrollCollection()
{
    if (!is_start_adc_collection_) {
        return;
    }

    is_start_adc_collection_ = true;
    is_roll_mode_ = true;

    if (!base_device_->StopADCCollection()) {
        qWarning(u8"尝试停止ADC采集失败");
    }

    if (!base_device_->StartADCCollection()) {
        qWarning(u8"开始ADC采集失败");
    }

    if (scroll_mode_adc_thread_ != nullptr) {
        scroll_mode_adc_thread_->quit();
        scroll_mode_adc_thread_->deleteLater();
        scroll_mode_adc_thread_ = nullptr;
    }
    scroll_mode_adc_thread_ = QSharedPointer<ScrollModeADCThread>::create(base_device_);
    connect(scroll_mode_adc_thread_.get(), &ScrollModeADCThread::SendData, this, &MainWindow::UpdatePlotData4Scroll);
    scroll_mode_adc_thread_->start();

}
void MainWindow::AutomaticCollection() {
    // 如果已经被停止，则不要再继续采集
    if (!is_start_adc_collection_) {
        return;
    }

    if (!base_device_->StopADCCollection()) {
        qWarning(u8"启动ADC采集失败");
        return;
    }

    if (!base_device_->StartADCCollection()) {
        qWarning(u8"启动ADC采集失败");
        return;
    }


    // 读取数据
    auto channel_number = base_device_->GetADCLegalChannelSize();

    // 确认上一次采集完成后再开始采集
    while (read_data_thread_pool_.activeThreadCount()) {
        QEventLoop loop;
        QTimer::singleShot(20, &loop, SLOT(quit()));
        loop.exec();
        QApplication::processEvents();
    }

    for (int channel_index = 0; channel_index < channel_number; ++channel_index) {
        // 获取需要读取数据的长度并初始化读取缓冲区
        ADC_Data_Buffer[channel_index] = std::make_shared<std::vector<float>>();
       // auto buffer_size = ((4920 / 492) + 10) * 492;
        auto buffer_size = sampleNum;
        ADC_Data_Buffer[channel_index]->resize(buffer_size);
        //ADC_Data_Buffer[channel_index]->resize(2 * 50000);
        auto read_thread = new ContinuousReadADCThread(base_device_, channel_index, sampleNum,
                                                       ADC_Data_Buffer[channel_index]);
        connect(read_thread, &ContinuousReadADCThread::complete, this, &MainWindow::ReceiveADCData);
        read_thread->setAutoDelete(true);
        read_data_thread_pool_.start(read_thread);
    }
}
void findMinMax(const std::vector<std::vector<float>> &data, float &minValue, float &maxValue) {
    // 初始化最小值和最大值
    minValue = std::numeric_limits<float>::max();
    maxValue = std::numeric_limits<float>::lowest();

    // 遍历二维向量，寻找最小值和最大值
    for (const auto &row : data) {
        // 找到当前子向量中的最小值和最大值
        if(row.empty())
        {
            continue;
        }
        for (const auto &value : row) {
            if (value < minValue) {
                minValue = value;
            }
            if (value > maxValue) {
                maxValue = value;
            }
        }
    }
}
void getMinMax(const std::vector<std::shared_ptr<std::vector<float>>>& ADC_Data_Buffer,float &minValue, float &maxValue)
{
    if (ADC_Data_Buffer.empty())
    {
        return;
    }

    float minVal = std::numeric_limits<float>::max();
    float maxVal = std::numeric_limits<float>::lowest();

    // 遍历 ADC_Data_Buffer 中的每个共享指针
    for (const auto& bufferPtr : ADC_Data_Buffer) {
        if (bufferPtr && !bufferPtr->empty())
        {

            auto [localMin, localMax] = std::minmax_element(bufferPtr->begin(), bufferPtr->end());
            // 更新全局最小值
            minVal = std::min(minVal, *localMin);
            // 更新全局最大值
            maxVal = std::max(maxVal, *localMax);
        }
    }
    minValue=minVal;
    maxValue= maxVal;
}
void getMinMax(const std::vector<std::vector<float>>& ADC_Data_Buffer,float &minValue, float &maxValue)
{
    if (ADC_Data_Buffer.empty())
    {
        return;
    }

    float minVal = std::numeric_limits<float>::max();
    float maxVal = std::numeric_limits<float>::lowest();

    // 遍历 ADC_Data_Buffer 中的每个共享指针
    for (const auto &bufferPtr : ADC_Data_Buffer) {
        if (!bufferPtr.empty())
        {

            auto [localMin, localMax] = std::minmax_element(bufferPtr.begin(), bufferPtr.end());
            // 更新全局最小值
            minVal = std::min(minVal, *localMin);
            // 更新全局最大值
            maxVal = std::max(maxVal, *localMax);
        }
    }
    minValue=minVal;
    maxValue= maxVal;
}
void MainWindow::UpdatePlotData4Scroll(const std::vector<std::vector<float> > &data) {

    // 判断数据是否为空
    bool is_empty = true;
    for (const auto &i: data) {
        if (!i.empty()) {
            is_empty = false;
            break;
        }
    }

    if (is_empty) {
        return;
    }

    auto trigger_type = base_device_->GetCurrentTriggerType();
    if (trigger_type != LockzhinerADCTriggerType::Normal) {
        // 触发类型不为滚动模式 清空之前的数据
        base_wave_widget_->Clear();
    }

    for (int i = 0; i < data.size(); ++i) {
        for (int j = 0; j < data.at(i).size(); ++j) {
            base_wave_widget_->AddData(i, data[i][j]);
        }
    }

    // 更新图表
    float y_min,y_max;
    findMinMax(data,y_min,y_max);
    UpdateAutoScale(trigger_type == LockzhinerADCTriggerType::Normal);
    base_wave_widget_->yAxis->setRange(y_min,y_max);
    base_wave_widget_->replot(QCustomPlot::rpQueuedReplot);


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


    if(currentCountTime<countTimeMax)
    {
        currentCountTime+=1;
    }

    else
    {
        std::vector<std::vector<float>> Transit_cache=divideAndSum (count_data_common,countTimeMax);

        for (int var = 0; var < 4; ++var)
        {
            const auto &data = count_data_common[var];
            if (data.empty()) {
                continue;
            }
            count_data_common[var].clear();
        }
        for (int channel = 0; channel < Transit_cache.size(); ++channel)
        {
            const auto &data = Transit_cache[channel];
            if (data.empty()) {
                continue;
            }

            for (int index = 0; index < data.size(); ++index)
            {
                base_wave_widget_->AddData(channel, data.at(index));
            }
            Mydatabuffer[channel].insert(
                Mydatabuffer[channel].end(),
                Transit_cache[channel].begin(),
                Transit_cache[channel].end()
                );
        }
        // 更新图表
        float y_min,y_max;
        getMinMax(Transit_cache,y_min,y_max);
        for (int var = 0; var < 4; ++var) {
            if(!Transit_cache[var].empty())
            {
                 base_wave_widget_->xAxis->setRange(0, Transit_cache[var].size());
            }

        }
        // 定义一个扩展比例
        const float paddingRatio = 0.1;

        // 计算数据的范围
        float range = y_max - y_min;

        // 计算扩展的数值
        float paddingValue = range * paddingRatio;

        // 计算扩展后的最小值和最大值
        float new_y_min = y_min - paddingValue;
        float new_y_max = y_max + paddingValue;

        // 设置 Y 轴的范围
        base_wave_widget_->yAxis->setRange(new_y_min, new_y_max);
        base_wave_widget_->replot(QCustomPlot::rpQueuedReplot);
    }

    // 添加数据到图表
    for (int channel = 0; channel < data_.size(); ++channel)
    {
        const auto &channel_data = data_.at(channel);
        if (channel_data.empty() || !channel_state_.at(channel)) {
            continue;
        }

        for (auto &value : channel_data) {
           // base_wave_widget_->AddData(channel, value);
            count_data_common[channel].emplace_back(value);
        }
    }
    if(Mydatabuffer[0].size()>=MaxDataSaveLength)
    {
        std::vector<std::vector<float>>linshi; // 缓存数据——保存用
        linshi.resize(4);
        for (int var = 0; var < linshi.size(); ++var)
        {
            if (Mydatabuffer[var].empty())
            {
                continue;
            }
            linshi[var].assign(Mydatabuffer[var].begin(), Mydatabuffer[var].begin() +MaxDataSaveLength);
            Mydatabuffer[var].erase(Mydatabuffer[var].begin(),Mydatabuffer[var].begin() +MaxDataSaveLength);
        }
        qInfo(u8"Mydatabuffer[0].size()>=MaxDataSaveLength=================");
        emit sendData2Save(linshi);

    }
    CommonCollection();

}
void MainWindow::ReceiveADCData(int channel) {
    auto channel_size = base_device_->GetADCLegalChannelSize();
    ADCDataState.resize(channel_size);
    ADCDataState[channel] = true;

    // 等待所有通道数据接收完成再显示
    for (int var = 0; var < ADCDataState.size(); ++var) {
        if (!ADCDataState[var]) {
            return;
        }
    }
    for (int var = 0; var < ADCDataState.size(); ++var) {
        ADCDataState[var] = false;
    }

    ContinuousCollectionUpdatePlotData();
}



void MainWindow::ContinuousCollectionUpdatePlotData()
{
    if (!is_start_adc_collection_) {
        return;
    }
    base_wave_widget_->Clear();

    if(currentCountTime<countTimeMax)
    {
        currentCountTime+=1;
    }
    else
    {
      //  qDebug()<<" count_data========================"<<count_data[0].size();
        std::vector<std::vector<float>> Transit_cache=divideAndSum (count_data,countTimeMax);

        for (int var = 0; var < 4; ++var)
        {
            const auto &data = count_data[var];
            if (data.empty()) {
               continue;
            }
            count_data[var].clear();
        }
     //  qDebug()<<" count_data=======afterclear================="<<count_data[0].size();
        for (int channel = 0; channel < Transit_cache.size(); ++channel)
        {
            const auto &data = Transit_cache[channel];
            if (data.empty()) {
               continue;
            }

            for (int index = 0; index < data.size(); ++index)
            {
               base_wave_widget_->AddData(channel, data.at(index));
            }

            Mydatabuffer[channel].insert(
                Mydatabuffer[channel].end(),
                Transit_cache[channel].begin(),
                Transit_cache[channel].end()
                );

        }
      //  qDebug()<<" Mydatabuffer.insert==============================="<<Mydatabuffer[0].size();
        float y_min,y_max;
        getMinMax(Transit_cache,y_min,y_max);
        for (int var = 0; var < 4; ++var) {
            if(!Transit_cache[var].empty())
            {
               base_wave_widget_->xAxis->setRange(0, Transit_cache[var].size());
            }

        }
        // 定义一个扩展比例
        const float paddingRatio = 0.1;

        // 计算数据的范围
        float range = y_max - y_min;

        // 计算扩展的数值
        float paddingValue = range * paddingRatio;

        // 计算扩展后的最小值和最大值
        float new_y_min = y_min - paddingValue;
        float new_y_max = y_max + paddingValue;

        // 设置 Y 轴的范围
        base_wave_widget_->yAxis->setRange(new_y_min, new_y_max);
        base_wave_widget_->replot(QCustomPlot::rpQueuedReplot);
        currentCountTime=1;
    }

    // 添加图表数据
    for (int channel = 0; channel < ADC_Data_Buffer.size(); ++channel)
    {
        const auto &data = ADC_Data_Buffer[channel];
        if (data->empty()) {
            continue;
        }

        for (int index = 0; index < data->size(); ++index)
        {
          //base_wave_widget_->AddData(channel, data->at(index));
          //Mydatabuffer[channel].emplace_back( data->at(index));
            count_data[channel].emplace_back( data->at(index));
        }

    }

   // qDebug()<<"Mydatabuffer[0].size()=================================================="<<Mydatabuffer[0].size();
    if((Mydatabuffer[0].size()>=MaxDataSaveLength)||(Mydatabuffer[1].size()>=MaxDataSaveLength)||(Mydatabuffer[2].size()>=MaxDataSaveLength)||(Mydatabuffer[3].size()>=MaxDataSaveLength))
    {
        std::vector<std::vector<float>>linshi; // 缓存数据——保存用
        linshi.resize(4);
        for (int var = 0; var < linshi.size(); ++var)
        {
            if (Mydatabuffer[var].empty())
            {
                continue;
            }
            linshi[var].assign(Mydatabuffer[var].begin(), Mydatabuffer[var].begin() +MaxDataSaveLength);
            Mydatabuffer[var].erase(Mydatabuffer[var].begin(),Mydatabuffer[var].begin() +MaxDataSaveLength);
        }
       //  qInfo(u8"Mydatabuffer[0].size()>=MaxDataSaveLength");
        emit sendData2Save(linshi);

    }
    AutomaticCollection();


}

void MainWindow::UpdateAutoScale(bool open_scroll) {


    // 寻找数据目前最长长度
    uint32_t data_length_max = 0;
    for (int i = 0; i < 4; ++i) {
        data_length_max = std::max(base_wave_widget_->graph(i)->data()->size(), int(data_length_max));
    }
    auto trigger_data_length = base_device_->GetCurrentDataLength();
    if (data_length_max > trigger_data_length) {
        base_wave_widget_->xAxis->setRange(data_length_max - trigger_data_length, data_length_max);
    } else {
        base_wave_widget_->xAxis->setRange(0, trigger_data_length);
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
