#ifndef WIDGET_H
#define WIDGET_H

#include <QMessageBox>
#include <QWidget>
#include <QList>
#include<QSerialPort>
#include<QSerialPortInfo>
#include<QTimer>
QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    QSerialPort *getSerialPort() const;
    void setSerialPort(QSerialPort *newSerialPort);

signals:
    void serialPortChanged();

private slots:
    void on_pushButtonCloseOrOpenSerial_clicked();
    void on_pushButtonMsgSend_clicked();
    void on_msg_reach();
    void copySerialPortConfig();
    void on_checkBoxclockSend_clicked(bool checked);

private:
    Ui::Widget *ui;
    QSerialPort * serialPort;
    QSerialPort * serialPortForRecv;
    int writeCntTotal;
    int recvCntTotal;
    QString preRecvMsg;
    QTimer *timer;
};
#endif // WIDGET_H
