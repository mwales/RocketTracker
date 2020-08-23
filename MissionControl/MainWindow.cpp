#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QtDebug>
#include <stdint.h>
#include <QtSerialPort>
#include <QMessageBox>
#include "qrcodegen.h"


MainWindow::MainWindow(QWidget *parent)
   : QMainWindow(parent)
   , ui(new Ui::MainWindow)
{
   ui->setupUi(this);

   theAvailPorts = QSerialPortInfo::availablePorts();
   foreach(QSerialPortInfo pi, theAvailPorts)
   {
      ui->theRadioPortList->addItem(pi.systemLocation());
   }

   connect(ui->theGenerateButton, &QPushButton::clicked,
           this, &MainWindow::generateQrCode);
   connect(ui->theOpenRadioButton, &QPushButton::clicked,
           this, &MainWindow::openRadio);
}

static void printQr(const uint8_t qrcode[]) {
        int size = qrcodegen_getSize(qrcode);
        int border = 4;
        for (int y = -border; y < size + border; y++) {
                for (int x = -border; x < size + border; x++) {
                        fputs((qrcodegen_getModule(qrcode, x, y) ? "##" : "  "), stdout);
                }
                fputs("\n", stdout);
        }
        fputs("\n", stdout);
}

void MainWindow::openRadio()
{
   qDebug() << "Open port #" << ui->theRadioPortList->currentIndex();

   QSerialPortInfo pi = theAvailPorts[ui->theRadioPortList->currentIndex()];

   if (theOpenPort)
   {
      qDebug() << "Closing serial port";
      theOpenPort->close();
      theOpenPort->deleteLater();
      theOpenPort = nullptr;
   }

   theOpenPort = new QSerialPort(pi, this);

   if(!theOpenPort->setDataBits(QSerialPort::Data8) ||
      !theOpenPort->setBaudRate(115200) ||
      !theOpenPort->setParity(QSerialPort::NoParity) ||
      !theOpenPort->setStopBits(QSerialPort::OneStop) )
   {
      qWarning() << "Error configuring the serial port";
      QMessageBox::critical(this, "Error opening serial port", "Failed to configure the serial port for 115200 8N1: " + theOpenPort->errorString());

      theOpenPort->deleteLater();
      theOpenPort = nullptr;
      return;
   }

   qDebug() << "Serial port configured for 115200 8N1";

   if(!theOpenPort->open(QIODevice::ReadWrite))
   {
      qWarning() << "Failed to open the serial port";
      QMessageBox::critical(this, "Error opening serial port", "Error opening serial port: " + theOpenPort->errorString());

      theOpenPort->deleteLater();
      theOpenPort = nullptr;
   }

   qDebug() << "Serial port opened successfully!";

   connect(theOpenPort, &QSerialPort::readyRead,
           this, &MainWindow::serialDataAvailable);
}

void MainWindow::serialDataAvailable()
{
   if (theOpenPort->canReadLine())
   {
      QByteArray data = theOpenPort->readLine(255);
      qDebug() << "DATA: " << data;
      logRawSerialData(data);

      processSerialData(data);
   }
   else
   {
      qDebug() << "Data not quite ready";
   }
}

void MainWindow::logRawSerialData(QByteArray data)
{
   QString strData(data.size());

   // Trim off line-endings and non-printable
   foreach(char curChar, data)
   {
      if ( (curChar >= ' ') && (curChar <= '~'))
      {
         strData += curChar;
      }
   }

   ui->theSerialRaw->addItem(strData);

   // Do we have too many items?
   if (ui->theSerialRaw->count() > 300)
   {
      QListWidgetItem* removeMe = ui->theSerialRaw->item(0);
      ui->theSerialRaw->removeItemWidget(removeMe);
   }

   ui->theSerialRaw->setCurrentRow(ui->theSerialRaw->count() - 1);
}

void MainWindow::processSerialData(QByteArray data)
{
   // Find the characters between > and !
   int startIndex = data.indexOf('>');

   if (startIndex == -1)
   {
      return;
   }

   int stopIndex = data.indexOf('!', startIndex + 1);
   if (stopIndex == -1)
   {
      return;
   }

   // We must have gotten a message from the rocket!
   QByteArray msgData = data.mid(startIndex + 1, stopIndex - startIndex - 1);

   qDebug() << "Rocket Msg:" << msgData;

   if ( (msgData.size() % 2) != 0)
   {
      // Invalid length of data
      qDebug() << "Received invalid amount of data: " << msgData.size() << " bytes";
      return;
   }

   QByteArray decodedData;
   for(int i = 0; i < msgData.size(); i += 2)
   {
      QByteArray singleAscii;
      singleAscii.append(msgData[i]);
      singleAscii.append(msgData[i+1]);

      bool success = true;
      uint8_t asciiChar = singleAscii.toUShort(&success, 16);

      if (!success)
      {
         qDebug() << "Invalid message text: " << singleAscii;
         return;
      }

      decodedData.append((char) asciiChar);
   }

   qDebug() << "Decoded Msg = " << decodedData;

   if (decodedData.size() <= 0)
   {
      // We got atleast a message type
      qDebug() << "Decoded Msg empty";
      return;
   }

   char msgType = decodedData.front();
   decodedData.remove(0,1);

   switch(msgType)
   {
   case 0x00:
      // Timestamp
      processTimestampReport(decodedData);
      return;

   case 0x01:
      // Position report
      processPositionReport(decodedData);
      return;

   case 0x02:
      // Altitude report
      processAltitudeReport(decodedData);
      return;

   case 0x03:
      // Battery level
      processBatteryLevelReport(decodedData);
      return;

   case 0x04:
      // GPS Fix / Precision
      processGpsFixReport(decodedData);
      return;

   case 0x05:
      // Num satellites
      processNumSatsReport(decodedData);
      return;

   case 0x06:
      // Speed
      processSpeedReport(decodedData);
      return;

   case 0x10:
      // 2D fix event
      process2dFixAcquiredEvent(decodedData);
      return;

   case 0x11:
      // 3D fix event
      process3dFixAcquiredEvent(decodedData);
      return;

   case 0x12:
      // GPS Lost
      processGpsLostEvent(decodedData);
      return;

   case 0x13:
      // Launch
      processLaunchEvent(decodedData);
      return;

   case 0x14:
      // Apogee detected
      processApogeeEvent(decodedData);
      return;

   case 0x15:
      // Landing detected
      processLandingEvent(decodedData);
      return;

   case 0x17:
      // ASCII event
      processAsciiEvent(decodedData);
      return;

   default:
      qWarning() << "Unexpected message type: " << (int) msgType << " with decoded data: " << decodedData;
      return;
   }

}

void MainWindow::processTimestampReport(QByteArray decodedData)
{

}

void MainWindow::processPositionReport(QByteArray decodedData)
{
   qDebug() << "Lat / Long data: " << decodedData;
}

void MainWindow::processAltitudeReport(QByteArray decodedData)
{

}

void MainWindow::processBatteryLevelReport(QByteArray decodedData)
{
   qDebug() << "Battery: " << decodedData;

   QStringList batteryData = QString(decodedData).split(",");

   if (batteryData.size() != 2)
   {
      qDebug() << "Battery data format error!  Num Items = " << batteryData.size();
      return;
   }

   bool success = true;
   double rawVoltage = batteryData[0].toDouble(&success) / 1000.0;
   if (!success)
   {
      qWarning() << "Invalid battery voltage: " << batteryData[0];
      return;
   }

   int batteryPercent = batteryData[1].toInt(&success);
   if (!success)
   {
      qWarning() << "Invalid batter percent:" << batteryData[1];
      return;
   }

   ui->theBatteryBar->setValue(batteryPercent);
   ui->theBatteryVoltsLabel->setText(QString("%1 V").arg(rawVoltage));
}

void MainWindow::processGpsFixReport(QByteArray decodedData)
{
   if (decodedData.size() < 3)
   {
      qWarning() << "Invalid fix report: " << decodedData;
      return;
   }

   QString fix = "Invalid";
   switch(decodedData[0])
   {
   case 0:
      fix = "None";
      break;
   case 1:
      fix = "2D";
      break;
   case 2:
      fix = "3D";
      break;
   }

   ui->theGpsFixLabel->setText(fix);

   QString dopStrTime10 = QString(decodedData.mid(2));
   bool success = true;
   double dop = dopStrTime10.toDouble(&success);
   if (!success)
   {
      qWarning() << "Error converting DOP to float: " << dopStrTime10;
      return;
   }

   qDebug() << "DOP: " << dop / 10.0;
   ui->theDopValue->setText(QString("%1").arg(dop / 10.0));
}

void MainWindow::processNumSatsReport(QByteArray decodedData)
{
   QString numSatStr(decodedData);
   bool success = true;
   int numSats = numSatStr.toInt(&success);
   if (!success)
   {
      qWarning() << "Error parsing num sats: " << numSatStr;
      return;
   }

   ui->theNumSatsValue->setText(QString("%1").arg(numSats));
}

void MainWindow::processSpeedReport(QByteArray decodedData)
{

}

void MainWindow::process2dFixAcquiredEvent(QByteArray decodedData)
{

}

void MainWindow::process3dFixAcquiredEvent(QByteArray decodedData)
{

}

void MainWindow::processGpsLostEvent(QByteArray decodedData)
{

}

void MainWindow::processLaunchEvent(QByteArray decodedData)
{

}

void MainWindow::processApogeeEvent(QByteArray decodedData)
{

}

void MainWindow::processLandingEvent(QByteArray decodedData)
{

}

void MainWindow::processAsciiEvent(QByteArray decodedData)
{

}


void MainWindow::generateQrCode()
{
   enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_LOW;  // Error correction level

   // Make and print the QR Code symbol
   uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
   uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
   bool ok = qrcodegen_encodeText(ui->theQrText->text().toLatin1(), tempBuffer, qrcode, errCorLvl,
                                 qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
   if (ok)
     printQr(qrcode);
}


MainWindow::~MainWindow()
{
   delete ui;
}

