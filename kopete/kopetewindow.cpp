#include <qwidget.h>
#include <kopetewindow.h>
#include <qlayout.h>
#include <kopete.h>

KopeteWindow::KopeteWindow(QWidget *parent, const char *name ): KMainWindow(parent,name)
{

	mainwidget = new QWidget(this);
	this->statusBar()->show();
	QBoxLayout *layout = new QBoxLayout(mainwidget,QBoxLayout::TopToBottom);

	contactlist = new ContactList(mainwidget);
	//statuslabel = new ConnectionLabel(mainwidget);
	//statuslabel->setText("offline");

	layout->insertWidget(-1,contactlist);
	//layout->insertWidget(-1,statuslabel);

	setCentralWidget(mainwidget);
	this->show();
	mainwidget->show();
  resize(200,400);

	initActions();
	createGUI("kopeteui.rc");
}

KopeteWindow::~KopeteWindow()
{
}

/** No descriptions */
void KopeteWindow::initActions()
{
	/* CTRL+SHIFT+Key_F */
	actionAddContact = 	new KAction( i18n("&Add contact"),"bookmark_add",0 ,
                          kopeteapp, SLOT(slotAddContact()),
                          actionCollection(), "AddContact" );

	actionConnect = new KAction( i18n("&Connect"),"connect_no",0 ,
                          kopeteapp, SLOT(slotConnect()),
                          actionCollection(), "Connect" );
	
	actionDisconnect = new KAction( i18n("&Disconnect"),"connect_established",0 ,
                          kopeteapp, SLOT(slotDisconnect()),
                          actionCollection(), "Disconnect" );
    actionDisconnect->setEnabled(false);

	actionAboutPlugins = new KAction( i18n("&Plugins"),"input_devices_settings", 0,
                          kopeteapp, SLOT(slotAboutPlugins()),
                          actionCollection(), "AboutPlugins" );

	actionPrefs = new KAction( i18n("&Configure Kopete"),"configure", 0,
                          kopeteapp, SLOT(slotPreferences()),
                          actionCollection(), "Preferences" );

	actionQuit = new KAction( i18n("&Quit"),"exit", 0,
                          kopeteapp, SLOT(slotExit()),
                          actionCollection(), "Quit" );
}

