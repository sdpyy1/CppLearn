#include "widget.h"
#include "./ui_widget.h"



Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    timeThread.start();
    writeCntTotal = 0;
    recvCntTotal = 0;
    preRecvMsg = "";
    timer = new QTimer(this);
    QTimer *sysTimeTimer = new QTimer(this);
    connect(sysTimeTimer,&QTimer::timeout,this,[=](){
        refresh_time();
    });
    sysTimeTimer->start(1000);
    // 串口对象创建
    this->serialPort = new QSerialPort(this);
    this->serialPortForRecv = new QSerialPort(this);
    // 获取当前存在的串口
    QList<QSerialPortInfo> list = QSerialPortInfo::availablePorts();
    for(QSerialPortInfo info: list){
        ui->comboBoxSerialName->addItem(info.portName());
    }

    // 不直接在自定义控件的事件重写方法，是为了直接在这里使用ui-> xxx
    connect(ui->comboBoxSerialName,&MyComboBox::fresh,this,[=](){
        ui->comboBoxSerialName->clear();
        // 获取当前存在的串口
        QList<QSerialPortInfo> list = QSerialPortInfo::availablePorts();
        for(QSerialPortInfo info: list){
            ui->comboBoxSerialName->addItem(info.portName());
        }
        ui->labelSend->setText("刷新串口成功");
    });

    // 设置一些下拉框的默认选择位置
    ui->comboBoxBoautrate->setCurrentIndex(6);
    ui->comboBoxDatabit->setCurrentIndex(3);

    // 绑定msg收到信号
    connect(serialPortForRecv,&QSerialPort::readyRead,this,&Widget::on_msg_reach);
    // 绑定定时器信号
    connect(timer,&QTimer::timeout,this,[=](){
        // timeout就发一条msg
        if(ui->lineEditMsg->text()== ""){
            QMessageBox msgBox;
            msgBox.setText("请输入发送内容！");
            timer->stop();
            ui->checkBoxclockSend->setChecked(false);
            msgBox.exec();
        }else{
            on_pushButtonMsgSend_clicked();
        }

    });
    connect(&timeThread,&QThread::finished,this,[](){
        qDebug() << "new Thread stop";
    });
}

Widget::~Widget()
{
    delete ui;
}
// 打开或关闭串口
void Widget::on_pushButtonCloseOrOpenSerial_clicked()
{
    if(ui->pushButtonCloseOrOpenSerial->text() == "关闭串口"){
        serialPort->close();
        ui->comboBoxSerialName->setEnabled(true);
        ui->comboBoxBoautrate->setEnabled(true);
        ui->comboBoxCheckbit->setEnabled(true);
        ui->comboBoxLiukong->setEnabled(true);
        ui->comboBoxStopbit->setEnabled(true);
        ui->comboBoxDatabit->setEnabled(true);
        ui->pushButtonMsgSend->setEnabled(false);
        ui->pushButtonCloseOrOpenSerial->setText("打开串口");
        ui->lineEditClockSend->setEnabled(false);
        ui->checkBoxclockSend->setEnabled(false);
        if(timer->isActive()){
            timer->stop();
        }

        return;
    }
    // 1. 选择端口号
    serialPort->setPortName(ui->comboBoxSerialName->currentText());
    // 2.配置波特率、数据位、校验位、停止位、流控
    serialPort->setBaudRate(ui->comboBoxBoautrate->currentText().toInt());
    serialPort->setDataBits(QSerialPort::DataBits(ui->comboBoxDatabit->currentText().toInt()));
    serialPort->setStopBits(QSerialPort::StopBits(ui->comboBoxStopbit->currentIndex()));
    if(ui->comboBoxLiukong->currentText() == "no"){
        serialPort->setFlowControl(QSerialPort::NoFlowControl);
    }
    switch(ui->comboBoxCheckbit->currentIndex()){
        case 0:
            serialPort->setParity(QSerialPort::NoParity);
            break;
        case 1:
            serialPort->setParity(QSerialPort::EvenParity);
            break;
        case 2:
            serialPort->setParity(QSerialPort::MarkParity);
            break;
        case 3:
            serialPort->setParity(QSerialPort::OddParity);
            break;
        case 4:
            serialPort->setParity(QSerialPort::SpaceParity);
            break;
        default:
            // serialPort->setParity(QSerialPort::UnknownParity);
            break;
    }
    // 打开串口
    if(!serialPort->open(QIODevice::ReadWrite)){
        qDebug() << "serialPort open error";
        qDebug() << serialPort->errorString();
    }
    qDebug() << "send port open: ok";
    // 克隆一个用于com2接收数据
    copySerialPortConfig();
    // 打开串口
    if(!serialPortForRecv->open(QIODevice::ReadWrite)){
        qDebug() << "serialPortForRecv open error";
        qDebug() << serialPortForRecv->errorString();
    }
    qDebug() << "recv port open: ok";
    ui->pushButtonCloseOrOpenSerial->setText("关闭串口");
    ui->comboBoxSerialName->setEnabled(false);
    ui->comboBoxBoautrate->setEnabled(false);
    ui->comboBoxCheckbit->setEnabled(false);
    ui->comboBoxLiukong->setEnabled(false);
    ui->comboBoxStopbit->setEnabled(false);
    ui->comboBoxDatabit->setEnabled(false);
    ui->pushButtonMsgSend->setEnabled(true);
    ui->lineEditClockSend->setEnabled(true);
    ui->checkBoxclockSend->setEnabled(true);


}


void Widget::on_pushButtonMsgSend_clicked()
{
    int writeCnt = 0;
    QByteArray sendData = ui->lineEditMsg->text().toUtf8().data();
    writeCnt = serialPort->write(sendData);
    if(writeCnt == -1){
            ui->labelSend->setText("send error");
    }else{
        writeCntTotal += writeCnt;
        ui->labelSend->setText("send ok");
        qDebug() << QString("send msg: %1").arg(sendData);
        ui->labelSent->setText(QString("sent: %1").arg(writeCntTotal));
    }
}

void Widget::on_msg_reach()
{
    // 因为QString(const QByteArray &ba); 有这样的构造函数，所以可以直接这样写
    QString msg = serialPortForRecv->readAll();
    if(!msg.isEmpty()){
        if(msg != preRecvMsg){
            ui->textEditRecord->append(msg);
            preRecvMsg = msg;
        }
        recvCntTotal += msg.toUtf8().size();
        if(ui->checkBoxHexShow->isChecked()){
            msg = QString::fromUtf8(msg.toUtf8().toHex());
        }
        if(ui->checkBoxRevTime->isChecked()){
            QDateTime curTime = QDateTime::currentDateTime();
            msg.prepend(curTime.toString("[hh:mm:ss]"));
        }
        ui->textEditRev->append(msg);
        ui->textEditRev->moveCursor(QTextCursor::MoveOperation::End);
        qDebug() << QString("recv msg: %1").arg(msg);

        ui->labelRecv->setText(QString("recv: %1").arg(recvCntTotal));
    }

}

void Widget::copySerialPortConfig()
{
    // 复制串口配置信息
    this->serialPortForRecv->setPortName("com2");
    this->serialPortForRecv->setBaudRate(this->serialPort->baudRate());
    this->serialPortForRecv->setDataBits(this->serialPort->dataBits());
    this->serialPortForRecv->setParity(this->serialPort->parity());
    this->serialPortForRecv->setStopBits(this->serialPort->stopBits());
    this->serialPortForRecv->setFlowControl(this->serialPort->flowControl());
}



void Widget::on_checkBoxclockSend_clicked(bool checked)
{
    if(checked){
        timer->start(ui->lineEditClockSend->text().toInt());
    }else{
        timer->stop();
    }
}


void Widget::on_pushButtonRevClear_clicked()
{
    ui->textEditRev->clear();
}


void Widget::on_pushButtonRevSave_clicked()
{
    QMessageBox box;
    box.setText("懒得做了");
    box.exec();
}

void Widget::refresh_time()
{
    QDateTime curTime = QDateTime::currentDateTime();
    ui->labelCurTime->setText(curTime.toString("yyyy-MM-dd hh:mm:ss"));
}

void Widget::on_checkBoxHexShow_clicked(bool checked)
{
    if (checked) {
        // 转 HEX
        QString content = ui->textEditRev->toPlainText();
        QStringList lines = content.split('\n');
        QString newContent;
        for (int i = 0; i < lines.size(); ++i) {
            const QString& line = lines[i];
            QString processedLine;
            int pos = 0;
            QRegularExpression re("\\[(.*?)\\]");
            QRegularExpressionMatchIterator id = re.globalMatch(line);
            while (id.hasNext()) {
                QRegularExpressionMatch match = id.next();
                int start = match.capturedStart();
                int end = match.capturedEnd();
                // 处理 [] 之前的部分
                if (start > pos) {
                    QString part = line.mid(pos, start - pos);
                    QByteArray array = part.toUtf8();
                    QByteArray arrayHex = array.toHex();
                    processedLine += QString::fromUtf8(arrayHex);
                }
                // 保留 [] 内的内容
                processedLine += match.captured(0);
                pos = end;
            }
            // 处理最后一个 [] 之后的部分
            if (pos < line.length()) {
                QString part = line.mid(pos);
                QByteArray array = part.toUtf8();
                QByteArray arrayHex = array.toHex();
                processedLine += QString::fromUtf8(arrayHex);
            }
            newContent += processedLine;
            if (i < lines.size() - 1) {
                newContent += '\n';
            }
        }
        ui->textEditRev->setText(newContent);
    } else {
        // 回去
        QString content = ui->textEditRev->toPlainText();
        QStringList lines = content.split('\n');
        QString newContent;
        for (int i = 0; i < lines.size(); ++i) {
            const QString& line = lines[i];
            QString processedLine;
            int pos = 0;
            QRegularExpression re("\\[(.*?)\\]");
            QRegularExpressionMatchIterator id = re.globalMatch(line);
            while (id.hasNext()) {
                QRegularExpressionMatch match = id.next();
                int start = match.capturedStart();
                int end = match.capturedEnd();

                // 处理 [] 之前的部分
                if (start > pos) {
                    QString part = line.mid(pos, start - pos);
                    QByteArray array = part.toUtf8();
                    array = QByteArray::fromHex(array);
                    processedLine += QString::fromUtf8(array);
                }
                // 保留 [] 内的内容
                processedLine += match.captured(0);
                pos = end;
            }
            // 处理最后一个 [] 之后的部分
            if (pos < line.length()) {
                QString part = line.mid(pos);
                QByteArray array = part.toUtf8();
                array = QByteArray::fromHex(array);
                processedLine += QString::fromUtf8(array);
            }
            newContent += processedLine;
            if (i < lines.size() - 1) {
                newContent += '\n';
            }
        }
        ui->textEditRev->setText(newContent);
    }
}

void Widget::on_pushButtonHidePanel_clicked(bool checked)
{
    if(checked){
        // 隐藏面板
        ui->groupBoxTexts->hide();
        ui->pushButtonHidePanel->setText("打开面板");
    }else{
        ui->groupBoxTexts->show();
        ui->pushButtonHidePanel->setText("隐藏面板");
    }
}

