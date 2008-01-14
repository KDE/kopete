
#ifndef _APPLICATIONWIDGET_H_
#define _APPLICATIONWIDGET_H_

#include "ui_mainWindow.h"
#include <QApplication>
#include <QtGui>
#include <QTextEdit>
#include <QString>
#include <QTreeWidget>
 #include <QLineEdit>

#include "upnpKopete.h"
#include "device.h"

class ApplicationWidget : public QMainWindow, private Ui::MainWindow
{
	Q_OBJECT
 
public:
	ApplicationWidget(QWidget *parent = 0);
	UpnpKopete * upnp;

public slots:
	void openPort();
	void deletePort();
	void envoyer();
	void close();
	void statusInfos();

};

#endif
