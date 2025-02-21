#ifndef CLION_EVB_CONNECT_CONTROLLER_H
#define CLION_EVB_CONNECT_CONTROLLER_H

#include <QDialog>
#include <QSharedPointer>
#include "libdaq/backend/tcp_server/tcp_server.h"
#include "libdaq/backend/usb_server/usb_server.h"
#include "libdaq/device/base_device.h"

QT_BEGIN_NAMESPACE

namespace Ui {
class ConnectPage;
}

QT_END_NAMESPACE

class ConnectPage : public QDialog {
Q_OBJECT
public:
  explicit ConnectPage(QSharedPointer<libdaq::device::LockzhinerBaseDevice> base_device, QWidget *parent = nullptr);

  ~ConnectPage() override;

  enum ConnectType {
    UNCONNECTED,  // 未连接
    TCP_CONNECTED, // TCP连接
    USB_CONNECTED, // USB连接
  };

signals:
  void ConnectStatus(bool status, ConnectType connect_type);

public:
  void InitParams(QSharedPointer<libdaq::device::LockzhinerBaseDevice> base_device);

public slots:
  void Disconnect(); // 断开连接

private slots:
  void TCPServerScanButtonClicked(); // TCP扫描客户端按钮
  void TCPSocketConnectButtonClicked(); // TCP连接按钮
  void TCPSocketDisconnectButtonClicked(); // TCP断开连接按钮

  void USBServerScanButtonClicked(); // USB扫描客户端按钮
  void USBServerConnectButtonClicked(); // USB连接按钮
  void USBServerDisconnectButtonClicked(); // USB断开连接按钮

private:
  Ui::ConnectPage *ui_;
  std::unique_ptr<libdaq::backend::LockzhinerUSBServer> usb_server_; // USB服务器
  std::unique_ptr<libdaq::backend::LockzhinerTCPServer> tcp_server_; // TCP服务器
  QWeakPointer<libdaq::device::LockzhinerBaseDevice> base_device_; // 即将要连接的设备

  ConnectType connect_type_ = UNCONNECTED; // 连接类型
};

#endif //CLION_EVB_CONNECT_CONTROLLER_H
