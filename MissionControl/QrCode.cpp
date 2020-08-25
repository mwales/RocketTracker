#include "QrCode.h"
#include <QPainter>

QrCode::QrCode(QWidget *parent) :
   QWidget(parent),
   theBorder(4)
{
   //setText("http://mwales.net");
}

void QrCode::setText(QString qrText)
{
   theText = qrText;


   // I want label to be about 600 x 600
   enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_LOW;  // Error correction level


   qrcodegen_encodeText(theText.toLatin1().data(), tempBuffer, qrcode, errCorLvl,
                                    qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);

   theQrSize = qrcodegen_getSize(qrcode);


   thePixelsPerPixel = 450 / (theQrSize + theBorder * 2);
   resize(thePixelsPerPixel * (theQrSize + theBorder * 2),
          thePixelsPerPixel * (theQrSize + theBorder * 2));

}


void QrCode::paintEvent(QPaintEvent*)
{
   QPainter painter(this);

   QBrush whiteBrush(Qt::white);
   QBrush blackBrush(Qt::black);

   for (int y = -theBorder; y < theQrSize + theBorder; y++)
   {
      for (int x = -theBorder; x < theQrSize + theBorder; x++)
      {
         if (qrcodegen_getModule(qrcode, x, y))
         {
            painter.fillRect((theBorder + x) * thePixelsPerPixel,
                             (theBorder + y) * thePixelsPerPixel,
                             thePixelsPerPixel,
                             thePixelsPerPixel,
                             whiteBrush);
         }
         else
         {
            painter.fillRect((theBorder + x) * thePixelsPerPixel,
                             (theBorder + y) * thePixelsPerPixel,
                             thePixelsPerPixel,
                             thePixelsPerPixel,
                             blackBrush);
         }

      }

   }

}

