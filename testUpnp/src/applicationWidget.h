
#ifndef _APPLICATIONWIDGET_H_
#define _APPLICATIONWIDGET_H_

#include "ui_mainWindow.h"
#include <QApplication>
#include <QtGui>
#include <QTextEdit>
#include <QString>

#include "upnpKopete.h"

class ApplicationWidget : public QMainWindow, private Ui::MainWindow
{
	Q_OBJECT
 
public:
	ApplicationWidget(QWidget *parent = 0);
	UpnpKopete * upnp;

public slots:
	void afficher();
	void envoyer();
	void close();

};

#endif
