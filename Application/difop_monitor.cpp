#include "difop_monitor.h"
#include "ui_difop_monitor.h"
#include <QUdpSocket>
#include <QMutex>
#include <QNetworkDatagram>

static QMutex g_mutex;

static char const *g_array[1024] = {
    "mainboard temperature mboard_temper.fatal_low",
    "mainboard temperature mboard_temper.warn_low",
    "mainboard temperature mboard_temper.fatal_high",
    "mainboard temperature mboard_temper.warn_high",
    "mainboard humidity humidity.fatal_low",
    "mainboard humidity humidity.warn_low",
    "mainboard humidity humidity.fatal_high",
    "mainboard humidity humidity.warn_high",
    "window temperature temperature.fatal_low",
    "window temperature temperature.warn_low",
    "window temperature temperature.fatal_high",
    "window temperature temperature.warn_low",
    "laser not connected",
    "laser sync point error",
    "laser temperature ls_temper.fatal_low",
    "laser temperature ls_temper.warn_low",
    "laser temperature ls_temper.fatal_high",
    "laser temperature ls_temper.warn_high",
    "laser voltage ls_vol.fatal_low",
    "laser voltage ls_vol.warn_low",
    "laser voltage ls_vol.fatal_high",
    "laser voltage ls_vol.warn_high",
    "sipm not connected",
    "sipm temperature sipm_temper.fatal_low",
    "sipm temperature sipm_temper.warn_low",
    "sipm temperature sipm_temper.fatal_high",
    "sipm temperature sipm_temper.warn_high",
    "sipm voltage sipm_vol.fatal_low",
    "sipm voltage sipm_vol.warn_low",
    "sipm voltage sipm_vol.fatal_high",
    "sipm voltage sipm_vol.warn_high",
    "mems temperature mems_temper.fatal_low",
    "mems temperature mems_temper.warn_low",
    "mems temperature mems_temper.fatal_high",
    "mems temperature mems_temper.warn_high",
    "mems voltage mems_vol.fatal_low",
    "mems voltage mems_vol.warn_low",
    "mems voltage mems_vol.fatal_high",
    "mems voltage mems_vol.warn_high",
    "mems frequence run out of range",
    "mems angle run out of range",
    "SCANNER_MINUSCULE_ANGLE_ALERT",
    "mems zero point run out of range",
    "mems FoV run out of range",
    "lidar config not exist",
    "calibration file not exist",
    "scanner calib file not exist",
    "sensor calib file not exist",
    "transmit calib file not exist",
    "ptp config file not exist",
    "DIRTY_WIN_ACTIVE",
    "ICE_WIN_ACTIVE",
    "Delay of send-data from PL to PHY",
    "Delay of recv-data",
    "Delay of pc frame process",
    "Main Eth reports down",
    "Main Eth reports TX unstable",
    "Main Eth reports RX unstable",
    "PCL_NO_DATA",
    "PDATA_ADDR_NULL",
    "TIME_TICK_ERROR",
    "PDATA_DONE_ERROR",
    "SENSOR_FRAME_NOT_ALINE",
    "FPGA_ANGLE_ERROR",
    "FPGA_NOT_READY",
    "FPGA_SYNC_ERROR",
    "MEMS_SCAN_FREQ_ERROR",
    "MEMS_ANGLE_ERROR"
};

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
    ui->lidarHumidity->setText(QString("%1%").arg((int)p->funcSafeInfo.lidarHumidity));

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
            ui->listWidget->insertItem(row++, QString(g_array[i]));
        }
		temp >>= 1;
    }
	temp = p->funcSafeInfo.curAlarmWarn[1];
	for (int i = 0; i < 64; i++) {
		auto flag = temp & 0x01;
		if (flag) {
            ui->listWidget->insertItem(row++, QString(g_array[i]));
		}
		temp >>= 1;
	}
}
