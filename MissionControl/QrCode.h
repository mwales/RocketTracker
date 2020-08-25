#ifndef QRCODE_H
#define QRCODE_H

#include <QWidget>
#include <QString>
#include <stdint.h>

#include "qrcodegen.h"

class QrCode : public QWidget
{
   Q_OBJECT
public:
   explicit QrCode(QWidget *parent = nullptr);

   void setText(QString qrText);

   void paintEvent(QPaintEvent*);

signals:

protected:

   QString theText;

   uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
   uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];

   int theQrSize;

   int thePixelsPerPixel;

   int theBorder;

};

#endif // QRCODE_H
