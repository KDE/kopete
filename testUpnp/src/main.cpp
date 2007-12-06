#include "applicationWidget.h"
#include <QApplication>
#include <QtGui>


int main(int argc, char *argv[])
{
  	
	QApplication app(argc, argv);
  	ApplicationWidget fenetre;
  	fenetre.show();

  	return app.exec();
};
