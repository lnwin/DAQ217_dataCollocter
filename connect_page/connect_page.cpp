#include "connect_page/connect_page.h"
#include "libdaq/backend/usb_server/usb_server.h"
#include "utils/utils.h"
#include "utils/uint_validator/uint_validator.h"
#include "ui_connect_page.h"
#include <QMessageBox>
#include <QEventLoop>
#include <QTimer>
#include <QLineEdit>
#include <QNetworkInterface>
#include <QSharedPointer>

ConnectPage::ConnectPage(QSharedPointer<libdaq::device::LockzhinerBaseDevice> base_device, QWidget *parent)
  : QDialog(parent), ui_(new Ui::ConnectPage)
  , base_device_(base_device)
   {
  ui_->setupUi(this);
  this->setWindowTitle(u8"选择连接方式");

  const auto usb_option = libdaq::device::DAQ217::GetUSBOption();

  const QString product_id = QString("0x%1").arg(usb_option.product_id, 4, 16, QChar('0'));
  ui_->USBServerPID->setText(product_id.toUpper());
  const QString vendor_id = QString("0x%1").arg(usb_option.vendor_id, 4, 16, QChar('0'));
  ui_->USBServerVID->setText(vendor_id.toUpper());

  ui_->TCPServerAddress->setText(QString::fromStdString("192.168.1.100"));
  ui_->TCPServerPort->setText(QString::number(8899));



  connect(ui_->USBServerScanButton, &QPushButton::clicked, this, &ConnectPage::USBServerScanButtonClicked);
  connect(ui_->USBServerConnectButton, &QPushButton::clicked, this, &ConnectPage::USBServerConnectButtonClicked);
  connect(ui_->USBServerDisconnectButton, &QPushButton::clicked, this, &ConnectPage::USBServerDisconnectButtonClicked);

  connect(ui_->TCPServerScanButton, &QPushButton::clicked, this, &ConnectPage::TCPServerScanButtonClicked);
  connect(ui_->TCPSocketConnectButton, &QPushButton::clicked, this, &ConnectPage::TCPSocketConnectButtonClicked);
  connect(ui_->TCPSocketDisconnectButton, &QPushButton::clicked, this, &ConnectPage::TCPSocketDisconnectButtonClicked);

  ui_->TCPSocketConnectCount->setValidator(new UIntValidator());
  ui_->TCPSocketConnectCount->setText(u8"1");


}

ConnectPage::~ConnectPage() {
  delete ui_;
}

void ConnectPage::USBServerScanButtonClicked() {
  // Lock button
  ui_->USBServerScanButton->setEnabled(false);

  // Create LockzhinerUSBServer

  const auto usb_option = libdaq::device::DAQ217::GetUSBOption();

  usb_server_ = std::make_unique<libdaq::backend::LockzhinerUSBServer>(usb_option);

  // Scan
  ui_->USBServerScanButton->setText(u8"扫描设备中");
  usb_server_->StartSearch();
  const auto bus_numbers = usb_server_->GetBusNumbers();
  for (auto &bus_number: bus_numbers) {
    const QString bus_number_str = Uint8ToString(bus_number);
    if (ui_->USBDeviceBusNumberComboBox->findText(bus_number_str.toUpper()) == -1) {
      ui_->USBDeviceBusNumberComboBox->addItem(bus_number_str.toUpper());
      const auto device_addresses = usb_server_->GetDeviceAddresses(bus_number);
      for (auto &device_address: device_addresses) {
        const QString device_address_str = Uint8ToString(device_address);
        if (ui_->USBDeviceAddressComboBox->findText(device_address_str.toUpper()) == -1) {
          ui_->USBDeviceAddressComboBox->addItem(device_address_str.toUpper());
        }
      }
    }
  }

  if(ui_->USBDeviceBusNumberComboBox->count() == 0
      || ui_->USBDeviceAddressComboBox->count() == 0){
      QMessageBox::warning(nullptr, u8"提示", u8"请接入并检查USB线缆是否连接良好");
  }

  ui_->USBServerScanButton->setText(u8"扫描设备");
  ui_->USBServerScanButton->setEnabled(true);
}

void ConnectPage::USBServerConnectButtonClicked() {
  if ((ui_->USBDeviceBusNumberComboBox->count() == 0)
        || (ui_->USBDeviceAddressComboBox->count() == 0)) {
    QMessageBox::warning(nullptr, u8"提示", u8"请接入并检查USB线缆是否连接良好");
    return;
  }
  if(connect_type_ != UNCONNECTED){
    QMessageBox::warning(NULL, u8"提示", u8"设备已连接！");
    return;
  }

  auto base_device = base_device_.toStrongRef();
  auto bus_number = ui_->USBDeviceBusNumberComboBox->currentText().toShort(nullptr, 16);
  auto device_address = ui_->USBDeviceAddressComboBox->currentText().toShort(nullptr, 16);

  auto usb_client = usb_server_->GetClient(bus_number, device_address);
  if (usb_client == nullptr) {
    qDebug(u8"No USBBackend  was found in the USBServer.");
    return;
  }
  base_device->UseBackend(usb_client);
  connect_type_ = USB_CONNECTED;
  emit ConnectStatus(true, connect_type_);

  QEventLoop loop;
  QTimer::singleShot(200, &loop, SLOT(quit()));
  loop.exec();

  if (usb_client->is_initialized() && usb_client->is_opened()) {
    QMessageBox::information(NULL, u8"提示", u8"设备连接成功！");
  } else {
    QMessageBox::warning(NULL, u8"提示", u8"设备连接失败！");
    connect_type_ = UNCONNECTED;
    return;
  }

}

void ConnectPage::TCPServerScanButtonClicked() {

  // Create LockzhinerTCPServer
  auto ip_address = ui_->TCPServerAddress->text().toUtf8().toStdString();
  auto port = ui_->TCPServerPort->text().toUtf8().toShort();

  // 获取当前所有网卡IP
  bool found_ip = false;
  QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    foreach (const QNetworkInterface &interface, interfaces) {
      if (!interface.flags().testFlag(QNetworkInterface::IsUp)
          || interface.flags().testFlag(QNetworkInterface::IsLoopBack))
        continue;

      // 遍历网络接口的所有地址条目
      QList<QNetworkAddressEntry> entryList = interface.addressEntries();
        foreach (const QNetworkAddressEntry &entry, entryList) {
          // 只打印IPv4地址
          if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol
              && entry.ip().toString().toStdString() == ip_address) {
            found_ip = true;
            break;
          }
        }
      if (found_ip) {
        break;
      }
    }

  if (!found_ip) {
    QMessageBox::warning(NULL, u8"提示", u8"IP地址不正确！"
                                         "\n如果您是第一次连接，请将电脑IP修改为192.168.1.100后，重启软件进行首次连接");
    return;
  }


  ui_->TCPServerScanButton->setEnabled(false);
  tcp_server_ = std::make_unique<libdaq::backend::LockzhinerTCPServer>(ip_address, port);

  ui_->TCPServerSocketAddress->clear();

  ui_->TCPServerScanButton->setText(u8"扫描设备中");

  auto connect_count = ui_->TCPSocketConnectCount->text().toInt();
  tcp_server_->StartAccept(10000, true, connect_count);
  while (!tcp_server_->IsAcceptComplete()) {
    QApplication::processEvents();
  }
  ui_->TCPServerScanButton->setText(u8"扫描设备");

  auto ip_list = tcp_server_->GetClientsIP();
  for (const auto &ip: ip_list) {
    qDebug(u8"TCPSocketBackend(%s) has successfully connected to the TCP server.", ip.c_str());
    ui_->TCPServerSocketAddress->addItem(QString::fromStdString(ip));
  }

  ui_->TCPServerScanButton->setEnabled(true);
}

void ConnectPage::TCPSocketConnectButtonClicked() {
  if (ui_->TCPServerSocketAddress->count() == 0) {
    qWarning(u8"The number of TCPServerSocketAddress is 0. Please check if the server has detected the device.");
    return;
  }
  if(connect_type_ != UNCONNECTED){
    QMessageBox::warning(NULL, u8"提示", u8"设备已连接！");
    return;
  }

  auto base_device = base_device_.toStrongRef();
  auto ip_address = ui_->TCPServerSocketAddress->currentText().toUtf8().toStdString();

  auto socket = tcp_server_->GetClient(ip_address);
  if (socket == nullptr) {
    qWarning(u8"No TCPSocket(%s) was found in the TCP server.", ip_address.c_str());
    return;
  }

  base_device->UseBackend(socket);
  connect_type_ = TCP_CONNECTED;
  emit ConnectStatus(true, connect_type_);

  QEventLoop loop;
  QTimer::singleShot(200, &loop, SLOT(quit()));
  loop.exec();

  if (socket->is_initialized() && socket->is_opened()) {
    QMessageBox::information(NULL, u8"提示", u8"设备连接成功！");

  } else {
    QMessageBox::warning(NULL, u8"提示", u8"设备连接失败！");
    connect_type_ = UNCONNECTED;
    return;
  }

}

void ConnectPage::TCPSocketDisconnectButtonClicked() {
  auto ip_address = ui_->TCPServerSocketAddress->currentText().toUtf8().toStdString();
  if (ip_address.empty()){
    qWarning(u8"No TCPSocket(%s) was found in the TCP server.", ip_address.c_str());
    return;
  }
  if(connect_type_ != TCP_CONNECTED){
    QMessageBox::warning(NULL, u8"提示", u8"请先连接以太网设备");
    return;
  }

  auto base_device = base_device_.toStrongRef();
  tcp_server_->DisconnectClient(ip_address);

  emit ConnectStatus(false, connect_type_);
  connect_type_ = UNCONNECTED;
  ui_->TCPServerSocketAddress->clear();
  QMessageBox::information(NULL, u8"提示", u8"设备断开成功！");
}

void ConnectPage::USBServerDisconnectButtonClicked() {
  auto base_device = base_device_.toStrongRef();
  auto bus_number = ui_->USBDeviceBusNumberComboBox->currentText().toShort(nullptr, 16);
  auto device_address = ui_->USBDeviceAddressComboBox->currentText().toShort(nullptr, 16);

  if(bus_number == 0 || device_address == 0){
    qWarning(u8"No USBBackend() was found in the TCP server.");
    return;
  }

  if(connect_type_ != USB_CONNECTED){
    QMessageBox::warning(NULL, u8"提示", u8"请先连接USB设备");
    return;
  }

  // usb_server_->Disconnect(bus_number, device_address);
  emit ConnectStatus(false, connect_type_);
  connect_type_ = UNCONNECTED;

  ui_->USBDeviceBusNumberComboBox->clear();
  ui_->USBDeviceAddressComboBox->clear();
  QMessageBox::information(NULL, u8"提示", u8"设备断开成功！");
}

void ConnectPage::InitParams(QSharedPointer<libdaq::device::LockzhinerBaseDevice> base_device) {
  base_device_ = base_device;
}

void ConnectPage::Disconnect() {
  if(connect_type_ == USB_CONNECTED){
    USBServerDisconnectButtonClicked();
  }else if(connect_type_ == TCP_CONNECTED){
    TCPSocketDisconnectButtonClicked();
  }
}
