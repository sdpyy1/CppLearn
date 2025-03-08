#ifndef TIMETHREAD_H
#define TIMETHREAD_H

#include <QThread>
#include <QWidget>

class TimeThread : public QThread
{
        Q_OBJECT
public:
    explicit TimeThread(QObject *parent = nullptr);
    void run() override;
};

#endif // TIMETHREAD_H
