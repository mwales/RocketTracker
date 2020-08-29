#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QtDebug>
#include <stdint.h>
#include <QtSerialPort>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent)
   : QMainWindow(parent)
   , ui(new Ui::MainWindow),
     theOpenPort(nullptr),
     theHeartbeatTimer(this),
     theTimeSinceLastMsg(0,0,0)
{
   ui->setupUi(this);

   theAvailPorts = QSerialPortInfo::availablePorts();
   foreach(QSerialPortInfo pi, theAvailPorts)
   {
      ui->theRadioPortList->addItem(pi.systemLocation());
   }

   theHeartbeatTimer.setInterval(200);
   theHeartbeatTimer.start();
   theTimeSinceLastMsg.start();

   connect(ui->theGenerateButton, &QPushButton::clicked,
           this, &MainWindow::generateQrCode);
   connect(ui->theOpenRadioButton, &QPushButton::clicked,
           this, &MainWindow::openRadio);
   connect(&theHeartbeatTimer, &QTimer::timeout,
           this, &MainWindow::heartbeat);
   connect(ui->theSendCmdButton, &QPushButton::pressed,
           this, &MainWindow::sendCmd);
}

void MainWindow::openSerial(QString port)
{
   if (theOpenPort)
   {
      qDebug() << "Closing serial port";
      theOpenPort->close();
      theOpenPort->deleteLater();
      theOpenPort = nullptr;
   }

   theOpenPort = new QSerialPort(port, this);

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

   theTimeSinceLastMsg.restart();
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

   theTimeSinceLastMsg.restart();
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

void MainWindow::heartbeat()
{
   if (theOpenPort == nullptr)
   {
      ui->theLastRxValue->setText("NO COMMS");
      return;
   }

   int et = theTimeSinceLastMsg.elapsed();
   int ms = et % 1000;
   int numSecs = (et / 1000);
   QString msString = QString("%1").arg(ms);
   while(msString.length() < 3)
   {
      msString = "0" + msString;
   }

   QString sString = QString("%1").arg(numSecs);


   QString timeString = sString + "." + msString;
   ui->theLastRxValue->setText(timeString);

}

void MainWindow::sendCmd()
{
   if (theOpenPort == nullptr)
   {
      QMessageBox::critical(this, "Failed to send message",
                            "Serial port not open");
      return;
   }

   if (theOpenPort != nullptr)
   {
      QString msg = ui->theCmdEdit->text() + "\n";

      if (-1 == theOpenPort->write(msg.toLatin1()))
      {
         QMessageBox::critical(this, "Failed to send message",
                               QString("Error %1").arg(theOpenPort->errorString()));
      }
   }
}

void MainWindow::logRawSerialData(QByteArray data)
{
   QString strData;

   // Trim off line-endings and non-printable
   foreach(char curChar, data)
   {
      uint8_t tempVal = curChar;
      if ( (tempVal >= 0x20) && (tempVal <= 0x7e) )
      {
         strData += curChar;
      }
   }

   ui->theSerialRaw->addItem(strData);

   // Do we have too many items?
   if (ui->theSerialRaw->count() > 150)
   {
      QListWidgetItem* removeMe = ui->theSerialRaw->takeItem(0);
      delete removeMe;
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

   theTimeSinceLastMsg.restart();

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

   case 0x07:
      // Peak altitude
      processPeakAltitudeReport(decodedData);
      return;

   case 0x0f:
      // RSSI
      processRssiReport(decodedData);
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
   if (decodedData.size() < 6)
   {
      qDebug() << "Timestamp too short";
      return;
   }
void openSerial(QString port);
   QString timeDisplay = QString("%1:%2:%3")
                         .arg(QString(decodedData.mid(0,2)))
                         .arg(QString(decodedData.mid(2,2)))
                         .arg(QString(decodedData.mid(4,2)));
   ui->theTimestampValue->setText(timeDisplay);
}

void MainWindow::processPositionReport(QByteArray decodedData)
{
   qDebug() << "Lat / Long data: " << decodedData;

   // Lat ends with either N/S
   int indexOfN = decodedData.indexOf('N');
   int indexOfS = decodedData.indexOf('S');

   int indexOfLatDir = indexOfN;
   if (indexOfLatDir == -1)
   {
      indexOfLatDir = indexOfS;
   }

   if ( (indexOfN == -1) && (indexOfS == -1))
   {
      qWarning() << "Can't process position report.  No N or S found: " << decodedData;
      return;
   }

   // Last thing in the report should be E/W
   int indexOfE = decodedData.indexOf('E');
   int indexOfW = decodedData.indexOf('W');

   if ( (indexOfE == -1) && (indexOfW == -1))
   {
      qWarning() << "Can't process position report.  No E or W found: " << decodedData;
      return;
   }

   int indexOfLongDir = indexOfE;
   if (indexOfLongDir == -1)
   {
      indexOfLongDir = indexOfW;
   }

   // Lat in hhmmss
   QString latInHhmmss = QString(decodedData.mid(0, indexOfLatDir));
   QString longInHhmmss = QString(decodedData.mid(indexOfLatDir + 1, indexOfLongDir - indexOfLatDir - 1));

   char latDir = decodedData.at(indexOfLatDir);
   char longDir = decodedData.at(indexOfLongDir);

   double latDegrees = convertGpsDegressMinutesToDecimal(latInHhmmss.toLatin1().data(), &latDir);
   double longDegrees = convertGpsDegressMinutesToDecimal(longInHhmmss.toLatin1().data(), &longDir);

   ui->theLatitude2Value->setText(QString("%1").arg(latDegrees));
   ui->theLongitude2Value->setText(QString("%1").arg(longDegrees));

   ui->theLatitude1Value->setText(QString("%1").arg(latInHhmmss));
   ui->theLongitude1Value->setText(QString("%1").arg(longInHhmmss));

   ui->theQrText->setText(QString("geo:%1,%2").arg(latDegrees).arg(longDegrees));
   ui->theQrImage->setText(QString("geo:%1,%2").arg(latDegrees).arg(longDegrees));

}

void MainWindow::processAltitudeReport(QByteArray decodedData)
{
   QString data = QString(decodedData);
   bool success;
   double altMetersTimes10 = data.toDouble(&success);
   if (!success)
   {
      qWarning() << "Error processing alititude data, cant convert to double" << decodedData;
      return;
   }

   double altMeters = altMetersTimes10 / 10.0;
   double altFeet = altMeters * 3.28084;
   ui->theAltitude1Value->setText(QString("%1").arg(altMeters));
   ui->theAltitude2Value->setText(QString("%1").arg(altFeet));
}

void MainWindow::processPeakAltitudeReport(QByteArray decodedData)
{
   QString data = QString(decodedData);
   bool success;
   double altMetersTimes10 = data.toDouble(&success);
   if (!success)
   {
      qWarning() << "Error processing alititude data, cant convert to double" << decodedData;
      return;
   }

   double altMeters = altMetersTimes10 / 10.0;
   double altFeet = altMeters * 3.28084;
   ui->theApogee1Value->setText(QString("%1").arg(altMeters));
   ui->theApogee2Value->setText(QString("%1").arg(altFeet));
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
   case '1':
      fix = "None";
      break;
   case '2':
      fix = "2D";
      break;
   case '3':
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
   QString data = QString(decodedData);
   bool success;
   double speedKmhTime10 = data.toDouble(&success);
   if(!success)
   {
      qWarning() << "Error processing speed report, cant convert to double" << decodedData;
      return;
   }

   double speedKmh = speedKmhTime10 / 10.0;
   double speedMph = speedKmh * 0.621371;
   ui->theSpeed1Value->setText(QString("%1").arg(speedKmh));
   ui->theSpeed2Value->setText(QString("%1").arg(speedMph));
}

void MainWindow::processRssiReport(QByteArray decodedData)
{
   QString rssiStr = decodedData;
   ui->theRssiValue->setText(rssiStr);
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
   ui->theQrImage->setText(ui->theQrText->text());
}

double MainWindow::convertGpsDegressMinutesToDecimal(char* number, char* direction)
{
   uint8_t indexOfDecimal = 0xff;
   for(int i = 0; i < strlen(number); i++)
   {
      if (number[i] == '.')
      {
         indexOfDecimal = i;
         break;
      }
   }

   if (indexOfDecimal == 0xff)
   {
      // No decimal found
      printf("Error");
      return 0.0;
   }

   char wholeBuf[8];
   strncpy(wholeBuf, number, 8);
   if (indexOfDecimal < 2)
   {
      printf("Error");
      return 0.0;
   }

   wholeBuf[indexOfDecimal - 2] = 0;

   double minutes = strtod(number + indexOfDecimal - 2, NULL);

   double degrees = strtod(wholeBuf, NULL);

   double decimalVal = degrees + minutes / 60.0;

   if ( (direction[0] == 'S') ||
        (direction[0] == 's') ||
        (direction[0] == 'W') ||
        (direction[0] == 'w') )
   {
      decimalVal *= -1.0;
   }

   return decimalVal;
}


MainWindow::~MainWindow()
{
   delete ui;
}

