#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QtDebug>
#include <stdint.h>
#include "qrcodegen.h"


MainWindow::MainWindow(QWidget *parent)
   : QMainWindow(parent)
   , ui(new Ui::MainWindow)
{
   ui->setupUi(this);

   connect(ui->theGenerateButton, &QPushButton::clicked,
           this, &MainWindow::generateQrCode);
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

