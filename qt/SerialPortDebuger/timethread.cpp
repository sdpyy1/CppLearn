#include "timethread.h"

TimeThread::TimeThread(QObject *parent)
    : QThread{parent}
{}

void TimeThread::run(){
    int i = 0;
    while(1){
        qDebug() << QString("%1: loop in new thread").arg(++i);;
        QThread::sleep(1);
        if(i==3){
            break;
        }
    }
}
