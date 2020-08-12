#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
   : QMainWindow(parent)
   , ui(new Ui::MainWindow)
{
   ui->setupUi(this);

   connect(ui->theInput, &QLineEdit::textChanged,
           this, &MainWindow::textChanged);
}

MainWindow::~MainWindow()
{
   delete ui;
}

void MainWindow::textChanged(QString text)
{
   ui->theOutput->setText(nmeaEncodeString(text.toLatin1().data()));
}

QString MainWindow::nmeaEncodeString(char* text)
{
   QString retVal;
   uint8_t checksum = 0;
   int cmdLen = strlen(text);
   for(int i = 0; i < cmdLen; i++)
   {
     checksum ^= text[i];
   }

   retVal += "$";
   retVal += text;
   retVal += "*";
   retVal += QString::number(checksum, 16);
   return retVal;
}
