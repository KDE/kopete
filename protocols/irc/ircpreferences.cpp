#include "ircpreferences.h"
#include <qlayout.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <klineedit.h>
#include <kdebug.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qcheckbox.h>

IRCPreferences::IRCPreferences(const QPixmap &pixmap,QObject *parent)
							: ConfigModule(i18n("IRC Plugin"),i18n("Internet Relay Chat Protocol"),pixmap,parent)
{
	/*
	//(new QVBoxLayout(this))->setAutoAdd(true);
	//QGroupBox  *box1 = new QGroupBox( i18n("Connection"), this);
	preferencesDialog = new msnPrefsUI(this);
	//QBoxLayout *layout1 = new QBoxLayout(this,QBoxLayout::TopToBottom);
	//QBoxLayout *layout2 = new QBoxLayout(this,QBoxLayout::LeftToRight);
	//QBoxLayout *layout3 = new QBoxLayout(this,QBoxLayout::LeftToRight);
	
	KGlobal::config()->setGroup("MSN");
	preferencesDialog->mID->setText(KGlobal::config()->readEntry("UserID", "Your MSN id here"));
	preferencesDialog->mPass->setText(KGlobal::config()->readEntry("Password", "Your MSN pass here"));
	preferencesDialog->mNick->setText(KGlobal::config()->readEntry("Nick", "Your nick"));
	preferencesDialog->mServer->setText(KGlobal::config()->readEntry("Server", "messenger.hotmail.com"));
	preferencesDialog->mPort->setText(KGlobal::config()->readEntry("Port", "1863"));
    preferencesDialog->mAutoConnect->setChecked(KGlobal::config()->readBoolEntry("AutoConnect", false));
	*/
}
IRCPreferences::~IRCPreferences()
{
}

void IRCPreferences::save()
{
	/*
	KConfig *config=KGlobal::config();
	config->setGroup("MSN");
	config->writeEntry("UserID", preferencesDialog->mID->text());
	config->writeEntry("Password", preferencesDialog->mPass->text());	
    config->writeEntry("Nick", preferencesDialog->mNick->text());	
	config->writeEntry("Server", preferencesDialog->mServer->text());	
	config->writeEntry("Port", preferencesDialog->mPort->text());	
    config->writeEntry("AutoConnect", preferencesDialog->mAutoConnect->isChecked());
	config->sync();
	emit saved();

	*/
}