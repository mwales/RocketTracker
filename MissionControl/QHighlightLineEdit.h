#ifndef QHIGHLIGHTLINEEDIT_H
#define QHIGHLIGHTLINEEDIT_H

#include <QLineEdit>
#include <QTimer>
#include <QVector>
#include <QStringList>

class QHighlightLineEdit : public QLineEdit
{
   Q_OBJECT

public:
   QHighlightLineEdit(QWidget *parent = nullptr);

   void setText(QString const & str);

protected slots:

   void updateHighlight();

protected:

   void setBackgroundColorToHighlightLevel();

   QTimer theTimer;

   int theHighlightLevel;

   // Common list to all instances
   static QVector<int> theColorShades;

};

#endif // QHIGHLIGHTLINEEDIT_H
