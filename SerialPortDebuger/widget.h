#ifndef WIDGET_H
#define WIDGET_H

#include <QFile>
#include <QMessageBox>
#include <QWidget>
#include <QList>
#include<QSerialPort>
#include<QSerialPortInfo>
#include<QTimer>
#include<QDateTime>
#include <TimeThread.h>
#include<QThread>
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
    void on_pushButtonRevClear_clicked();
    void on_pushButtonRevSave_clicked();
    void on_checkBoxHexShow_clicked(bool checked);
    void on_pushButtonHidePanel_clicked(bool checked);

private:
    Ui::Widget *ui;
    QSerialPort * serialPort;
    QSerialPort * serialPortForRecv;
    int writeCntTotal;
    int recvCntTotal;
    QString preRecvMsg;
    QTimer *timer;
    void refresh_time();
    TimeThread timeThread;
};
#endif // WIDGET_H
