#ifndef KOPETEWINDOW_H
#define KOPETEWINDOW_H

#include <qwidget.h>
#include <kmainwindow.h>
#include <kaction.h>
#include "contactlist.h"
#include "connectionlabel.h"

class QLabel;

class KopeteWindow : public KMainWindow
{
Q_OBJECT
public:
	KopeteWindow(QWidget *parent=0, const char *name=0);
	 ~KopeteWindow();
	/** No descriptions */
  void initActions();
  QWidget *mainwidget;
	
	ContactList *contactlist;
	
	ConnectionLabel *statuslabel;

	/* Some Actions */
  KAction* actionAddContact;
	KAction* actionAboutPlugins;
	KAction* actionSetAway;
	KAction* actionQuit;
	KSelectAction* actionStatus;
  KAction* actionConnect;
  KAction* actionPrefs;
  KAction* actionHide;
};

#endif
