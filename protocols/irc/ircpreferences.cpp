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

IRCPreferences::IRCPreferences(const QPixmap &pixmap,QObject *parent)
							: ConfigModule(i18n("IRC Plugin"),i18n("Internet Relay Chat Protocol"),pixmap,parent)
{
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
	QString text = KGlobal::config()->readEntry("HighlightPhrase", "");
	if (text.isEmpty() == false)
	{
		preferencesDialog->phraseHighlight->setText(text);
	} else {
		preferencesDialog->chkHighlightOthers->setChecked(false);
	}
	QObject::connect(preferencesDialog->chkHighlightNick, SIGNAL(clicked()), this, SLOT(slotHighlightNick()));
	QObject::connect(preferencesDialog->chkHighlightOthers, SIGNAL(clicked()), this, SLOT(slotHighlightOthers()));
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
	if (preferencesDialog->phraseHighlight->text().isEmpty() == false)
	{
		config->writeEntry("HighlightPhrase", preferencesDialog->phraseHighlight->text());
	}
	config->sync();
	emit saved();
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
