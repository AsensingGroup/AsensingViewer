#include "difop_monitor.h"
#include "ui_difop_monitor.h"
#include <QUdpSocket>
#include <QMutex>
#include <QNetworkDatagram>

static QMutex g_mutex;

static char const *g_array[1024] = {
    "MAINBOARD_TEMPERATURE_FATAL_LOW",
    "MAINBOARD_TEMPERATURE_WARN_LOW",
    "MAINBOARD_TEMPERATURE_FATAL_HIGH",
    "MAINBOARD_TEMPERATURE_WARN_HIGH",
    "MAINBOARD_HUMIDITY_FATAL_LOW",
    "MAINBOARD_HUMIDITY_WARN_LOW",
    "MAINBOARD_HUMIDITY_FATAL_HIGH",
    "MAINBOARD_HUMIDITY_WARN_HIGH",
    "WINDOW_TEMPERATURE_FATAL_LOW",
    "WINDOW_TEMPERATURE_WARN_LOW",
    "WINDOW_TEMPERATURE_FATAL_HIGH",
    "WINDOW_TEMPERATURE_WARN_HIGH",
    "LASER_NOT_CONNECTED",
    "LASER_SYNC_POINT_ERROR",
    "LASER_TEMPERATURE_FATAL_LOW",
    "LASER_TEMPERATURE_WARN_LOW",
    "LASER_TEMPERATURE_FATAL_HIGH",
    "LASER_TEMPERATURE_WARN_HIGH",
    "LASER_VOLTAGE_FATAL_LOW",
    "LASER_VOLTAGE_WARN_LOW",
    "LASER_VOLTAGE_FATAL_HIGH",
    "LASER_VOLTAGE_WARN_HIGH",
    "SIPM_NOT_CONNECTED",
    "SIPM_TEMPERATURE_FATAL_LOW",
    "SIPM_TEMPERATURE_WARN_LOW",
    "SIPM_TEMPERATURE_FATAL_HIGH",
    "SIPM_TEMPERATURE_WARN_HIGH",
    "SIPM_VOLTAGE_FATAL_LOW",
    "SIPM_VOLTAGE_WARN_LOW",
    "SIPM_VOLTAGE_FATAL_HIGH",
    "SIPM_VOLTAGE_WARN_HIGH",
    "MEMS_TEMPERATURE_FATAL_LOW",
    "MEMS_TEMPERATURE_WARN_LOW",
    "MEMS_TEMPERATURE_FATAL_HIGH",
    "MEMS_TEMPERATURE_WARN_HIGH",
    "MEMS_VOLTAGE_FATAL_LOW",
    "MEMS_VOLTAGE_WARN_LOW",
    "MEMS_VOLTAGE_FATAL_HIGH",
    "MEMS_VOLTAGE_WARN_HIGH",
    "SCANNER_FREQ_ALERT",
    "SCANNER_EXTRE_ANGLE_ALERT",
    "SCANNER_MINUSCULE_ANGLE_ALERT",
    "SCANNER_ZERO_ALERT",
    "SCANNER_FOV_ALERT",
    "LIDAR_CONFIG_NOT_EXIST",
    "CALIBRATION_FILE_NOT_EXIST",
    "SCANNER_CALIB_FILE_NOT_EXIST",
    "SENSOR_CALIB_FILE_NOT_EXIST",
    "TRANSMIT_CALIB_FILE_NOT_EXIST",
    "PTP_CONFIG_FILE_NOT_EXIST",
    "DIRTY_WIN_ACTIVE",
    "ICE_WIN_ACTIVE",
    "PC_SEND_TIMEOUT_ACTIVE",
    "PC_RECV_TIMEOUT_ACTIVE",
    "FRAME_DELAY_WARN_ACTIVE",
    "ETH_IF_DOWN_ACTIVE",
    "ETH_IF_TX_ERR_ACTIVE",
    "ETH_IF_RX_ERR_ACTIVE",
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
