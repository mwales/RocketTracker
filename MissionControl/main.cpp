#include "MainWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   MainWindow w;

   if ( (argc == 3) && (strcmp(argv[1], "--serial") == 0) )
   {
      w.openSerial(argv[2]);
   }
   w.show();
   return a.exec();
}
