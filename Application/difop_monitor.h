#ifndef DIFOP_MONITOR_H
#define DIFOP_MONITOR_H

#include <QDockWidget>

#include "difop.h"

namespace Ui {
class DifopMonitor;
}

class QUdpSocket;

class DifopMonitor : public QDockWidget
{
    Q_OBJECT

public:
    explicit DifopMonitor(QWidget *parent = nullptr);
    ~DifopMonitor();


protected:
    void updateUI(const DeviceInfoPackage *p);

private:
    Ui::DifopMonitor *ui;
    QUdpSocket *m_udpSocket;
};

#endif // DIFOP_MONITOR_H
