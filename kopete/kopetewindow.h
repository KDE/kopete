#ifndef KOPETEWINDOW_H
#define KOPETEWINDOW_H

#include <kmainwindow.h>

class QLabel;
class KAction;
class KToggleAction;
class KSelectAction;
class ContactList;

class KopeteWindow : public KMainWindow
{
	Q_OBJECT

	public:
		KopeteWindow ( QWidget *parent=0, const char *name=0 );
		 ~KopeteWindow();

	private slots:
		void showToolbar(void);

	public:
		ContactList *contactlist;
		/* Some Actions */
		KAction* actionAddContact;
		KAction* actionAboutPlugins;
		KAction* actionSetAway;
		KAction* actionQuit;
		KSelectAction* actionStatus;
		KAction* actionConnect;
		KAction* actionDisconnect;
		KAction* actionPrefs;
		KAction* actionHide;
		KToggleAction *toolbarAction;

	private:
		bool queryExit(void);
		void loadOptions(void);
		void saveOptions(void);
		QWidget *mainwidget;
};

#endif
