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
#include <kcolorbutton.h>
#include <qradiobutton.h>

IRCPreferences::IRCPreferences(const QString &pixmap,QObject *parent)
	: ConfigModule(i18n("IRC Plugin"),i18n("Internet Relay Chat Protocol"),pixmap,parent)
{
	(new QVBoxLayout(this))->setAutoAdd(true);
	preferencesDialog = new ircPrefsUI(this);
	KGlobal::config()->setGroup("IRC");
	preferencesDialog->mID->setText(KGlobal::config()->readEntry("Nickname", "KopeteUser"));
	preferencesDialog->mServer->setText(KGlobal::config()->readEntry("Server", "irc.openprojects.net"));
	preferencesDialog->mPort->setText(KGlobal::config()->readEntry("Port", "6667"));
	preferencesDialog->mAutoConnect->setChecked(KGlobal::config()->readBoolEntry("AutoConnect", false));
	QColor color(175, 8, 8);
	preferencesDialog->highlightColor->setColor(KGlobal::config()->readColorEntry("HighlightColor", &color));
	preferencesDialog->chkHighlightNick->setChecked(KGlobal::config()->readBoolEntry("HighlightNickname", false));
	preferencesDialog->chkHighlightOthers->setChecked(KGlobal::config()->readBoolEntry("HighlightOthers", false));
	preferencesDialog->chkSideIcons->setChecked(KGlobal::config()->readBoolEntry("UseSidePics", true));
	preferencesDialog->chkShowTimes->setChecked(KGlobal::config()->readBoolEntry("UseTimeStamps", true));
	preferencesDialog->useMDI->setChecked(KGlobal::config()->readBoolEntry("UseMDI", true));
	preferencesDialog->chkMinimizeQueries->setChecked(KGlobal::config()->readBoolEntry("MinimizeNewQueries", false));
	preferencesDialog->chkHideConsole->setChecked(KGlobal::config()->readBoolEntry("HideConsole", false));
	preferencesDialog->chkLogChans->setChecked(KGlobal::config()->readBoolEntry("LogChannels", true));
	preferencesDialog->chkLogQuery->setChecked(KGlobal::config()->readBoolEntry("LogQueries", true));
	preferencesDialog->chkLogDcc->setChecked(KGlobal::config()->readBoolEntry("LogDcc", false));
	preferencesDialog->chkLogServers->setChecked(KGlobal::config()->readBoolEntry("LogServers", false));
	if (preferencesDialog->useMDI->isChecked() == false)
	{
		preferencesDialog->useSDI->setChecked(KGlobal::config()->readBoolEntry("UseSDI", false));
		if (preferencesDialog->useSDI->isChecked() == false)
		{
			preferencesDialog->useMDI->setChecked(true);
		}
	}
	QString text = KGlobal::config()->readEntry("HighlightPhrase", "");
	if (text.isEmpty() == false)
	{
		preferencesDialog->phraseHighlight->setText(text);
	} else {
		preferencesDialog->chkHighlightOthers->setChecked(false);
	}
	QObject::connect(preferencesDialog->chkHighlightNick, SIGNAL(clicked()), this, SLOT(slotHighlightNick()));
	QObject::connect(preferencesDialog->chkHighlightOthers, SIGNAL(clicked()), this, SLOT(slotHighlightOthers()));
	QObject::connect(preferencesDialog->useMDI, SIGNAL(clicked()), this, SLOT(slotUseMDI()));
}
IRCPreferences::~IRCPreferences()
{
}

void IRCPreferences::save()
{
	KConfig *config=KGlobal::config();
	config->setGroup("IRC");
	config->writeEntry("Nickname", preferencesDialog->mID->text());
	config->writeEntry("Server", preferencesDialog->mServer->text());
	config->writeEntry("Port", preferencesDialog->mPort->text());
	config->writeEntry("AutoConnect", preferencesDialog->mAutoConnect->isChecked());
	config->writeEntry("HighlightNickname", preferencesDialog->chkHighlightNick->isChecked());
	config->writeEntry("HighlightOthers", preferencesDialog->chkHighlightOthers->isChecked());
	config->writeEntry("HighlightColor", preferencesDialog->highlightColor->color());
	config->writeEntry("UseSidePics", preferencesDialog->chkSideIcons->isChecked());
	config->writeEntry("UseTimeStamps", preferencesDialog->chkShowTimes->isChecked());
	config->writeEntry("UseMDI", preferencesDialog->useMDI->isChecked());
	config->writeEntry("UseSDI", preferencesDialog->useSDI->isChecked());
	config->writeEntry("MinimizeNewQueries", preferencesDialog->chkMinimizeQueries->isChecked());
	config->writeEntry("HideConsole", preferencesDialog->chkHideConsole->isChecked());
	config->writeEntry("LogChannels", preferencesDialog->chkLogChans->isChecked());
	config->writeEntry("LogQueries", preferencesDialog->chkLogQuery->isChecked());
	config->writeEntry("LogDcc", preferencesDialog->chkLogDcc->isChecked());
	config->writeEntry("LogServers", preferencesDialog->chkLogServers->isChecked());
	if (preferencesDialog->phraseHighlight->text().isEmpty() == false)
	{
		config->writeEntry("HighlightPhrase", preferencesDialog->phraseHighlight->text());
	}
	config->sync();
	emit saved();
}

void IRCPreferences::slotUseMDI()
{
	if (preferencesDialog->useMDI->isChecked())
	{
		preferencesDialog->chkMinimizeQueries->setEnabled(true);
	} else {
		preferencesDialog->chkMinimizeQueries->setChecked(false);
		preferencesDialog->chkMinimizeQueries->setEnabled(false);
	}
}

void IRCPreferences::slotHighlightNick()
{
	if (preferencesDialog->chkHighlightNick->isChecked() == true)
	{
		preferencesDialog->lblUseToHighlight->setEnabled(true);
		preferencesDialog->lblClick->setEnabled(true);
		preferencesDialog->highlightColor->setEnabled(true);
		if (preferencesDialog->chkHighlightOthers->isChecked() == false)
		{
			preferencesDialog->phraseHighlight->setEnabled(false);
		}
	} else {
		if (preferencesDialog->chkHighlightOthers->isChecked() == false)
		{
			preferencesDialog->lblUseToHighlight->setEnabled(false);
			preferencesDialog->lblClick->setEnabled(false);
			preferencesDialog->highlightColor->setEnabled(false);
		}
	}
}

void IRCPreferences::slotHighlightOthers()
{
	if (preferencesDialog->chkHighlightOthers->isChecked() == true)
	{
		preferencesDialog->lblUseToHighlight->setEnabled(true);
		preferencesDialog->lblClick->setEnabled(true);
		preferencesDialog->highlightColor->setEnabled(true);
		preferencesDialog->phraseHighlight->setEnabled(true);
	} else {
		if (preferencesDialog->chkHighlightNick->isChecked() == false)
		{
			preferencesDialog->lblUseToHighlight->setEnabled(false);
			preferencesDialog->lblClick->setEnabled(false);
			preferencesDialog->highlightColor->setEnabled(false);
			preferencesDialog->phraseHighlight->setEnabled(false);
		}
	}
}
#include "ircpreferences.moc"
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

