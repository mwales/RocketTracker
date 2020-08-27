#include "QHighlightLineEdit.h"
#include <QPalette>
#include <QColor>

QVector<int> QHighlightLineEdit::theColorShades = { 0xff, 0xe6, 0xcc, 0xb3, 0x99,
                                                 0x80, 0x66, 0x4d, 0x33 };

QHighlightLineEdit::QHighlightLineEdit(QWidget *parent):
   QLineEdit(parent),
   theTimer(this)
{
   setAutoFillBackground(true);

   theHighlightLevel = 0;

   theTimer.setInterval(250);
   theTimer.start();

   connect(&theTimer, &QTimer::timeout,
           this, &QHighlightLineEdit::updateHighlight);
}

void QHighlightLineEdit::setText(QString const & str)
{
   QLineEdit::setText(str);

   theHighlightLevel = theColorShades.size() - 1;

   setBackgroundColorToHighlightLevel();
}

void QHighlightLineEdit::setBackgroundColorToHighlightLevel()
{
   QPalette p = palette();
   QColor c(theColorShades[theHighlightLevel],
            theColorShades[theHighlightLevel],
            0xff);

   p.setColor(QPalette::Base, c);
   setPalette(p);


}

void QHighlightLineEdit::updateHighlight()
{
   if (theHighlightLevel == 0)
   {
      return;
   }

   theHighlightLevel--;
   setBackgroundColorToHighlightLevel();
}
