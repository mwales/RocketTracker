#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort>
#include <QList>
#include <QByteArray>
#include <QTimer>
#include <QTime>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
   Q_OBJECT

public:
   MainWindow(QWidget *parent = nullptr);
   ~MainWindow();

protected slots:

   void generateQrCode();

   void openRadio();

   void serialDataAvailable();

   void heartbeat();

protected:

   void logRawSerialData(QByteArray data);

   void processSerialData(QByteArray data);

   void processTimestampReport(QByteArray decodedData);
   void processPositionReport(QByteArray decodedData);
   void processAltitudeReport(QByteArray decodedData);
   void processBatteryLevelReport(QByteArray decodedData);
   void processGpsFixReport(QByteArray decodedData);
   void processNumSatsReport(QByteArray decodedData);
   void processSpeedReport(QByteArray decodedData);
   void processRssiReport(QByteArray decodedData);
   void process2dFixAcquiredEvent(QByteArray decodedData);
   void process3dFixAcquiredEvent(QByteArray decodedData);
   void processGpsLostEvent(QByteArray decodedData);
   void processLaunchEvent(QByteArray decodedData);
   void processApogeeEvent(QByteArray decodedData);
   void processLandingEvent(QByteArray decodedData);
   void processAsciiEvent(QByteArray decodedData);

   double convertGpsDegressMinutesToDecimal(char* number, char* direction);

private:
   Ui::MainWindow *ui;

   QList<QSerialPortInfo> theAvailPorts;

   QSerialPort* theOpenPort;

   QTimer theHeartbeatTimer;
   QTime theTimeSinceLastMsg;
};
#endif // MAINWINDOW_H
