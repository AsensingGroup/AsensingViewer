#include "difop_monitor.h"
#include "ui_difop_monitor.h"
#include <QUdpSocket>
#include <QMutex>
#include <QNetworkDatagram>

static QMutex g_mutex;

DifopMonitor::DifopMonitor(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::DifopMonitor)
{
    ui->setupUi(this);

    m_udpSocket = new QUdpSocket(this);
    m_udpSocket->bind(9988);
    connect(m_udpSocket, &QUdpSocket::readyRead, this, [=]() {
        auto checkValid = [=](const QByteArray &data) -> bool {
            return data.size() == 102;
        };
        auto data = m_udpSocket->receiveDatagram().data();
        if(checkValid(data)) {
            g_mutex.lock();
            auto p = reinterpret_cast<DeviceInfoPackage *>(data.data());
            this->updateUI(p);
            g_mutex.unlock();
        }
    });
}

DifopMonitor::~DifopMonitor()
{
    m_udpSocket->close();
    delete ui;
}

void DifopMonitor::updateUI(const DeviceInfoPackage *p)
{
    // func safe
    ui->laserTemperature->setText(QString("%1°C").arg((int)p->funcSafeInfo.lidarTmp));
    ui->memsTemperature->setText(QString("%1°C").arg((int)p->funcSafeInfo.MEMSTmp));
    ui->sensorTemperature->setText(QString("%1°C").arg((int)p->funcSafeInfo.sensorTmp));
    ui->windowTemperature->setText(QString("%1°C").arg((int)p->funcSafeInfo.windowTmp));
    ui->lidarHumidity->setText(QString("%1°C").arg((int)p->funcSafeInfo.lidarHumidity));

    // header
    ui->timestamp->setText(QString("%1").arg((int)p->difPackageHead.timeSyncInfo.Timestamp));
    ui->fps->setText(QString("%1").arg((int)p->difPackageHead.FPS));
    ui->firmwareVersion->setText(QString("%1").arg(QString((char *)p->difPackageHead.firmwareVersion)));
    ui->snSequence->setText(QString("%1").arg(QString((char *)p->difPackageHead.SNSequnce)));
    ui->horizontalStartAngle->setText(QString("%1°").arg((int)p->difPackageHead.fovCfgInfo.startAngleH));
    ui->horizontalEndAngle->setText(QString("%1°").arg((int)p->difPackageHead.fovCfgInfo.endAngleH));
    ui->verticalStartAngle->setText(QString("%1°").arg((int)p->difPackageHead.fovCfgInfo.startAngleV));
    ui->verticalEndAngle->setText(QString("%1°").arg((int)p->difPackageHead.fovCfgInfo.endAngleV));
    ui->echoFlag->setText(QString("%1").arg((int)p->difPackageHead.flags));

    // warnings
    ui->alarmCount->setText(QString("%1").arg((int)p->funcSafeInfo.curFaultNum));
    ui->listWidget->clear();
    int row = 0;
	auto temp = p->funcSafeInfo.curAlarmWarn[0];
    for(int i = 0; i < 64; i++) {
		auto flag = temp & 0x01;
        if(flag) {
            ui->listWidget->insertItem(row++, QString("Error code : %1").arg(i));
        }
		temp >>= 1;
    }
	temp = p->funcSafeInfo.curAlarmWarn[1];
	for (int i = 0; i < 64; i++) {
		auto flag = temp & 0x01;
		if (flag) {
			ui->listWidget->insertItem(row++, QString("Error code : %1").arg(i + 64));
		}
		temp >>= 1;
	}
}
