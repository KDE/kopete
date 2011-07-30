/*
  smssend.cpp  -  SMS Plugin

  Copyright (c) 2003      by Richard Lärkäng        <nouseforaname@home.se>
  Copyright (c) 2003      by Gav Wood               <gav@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
*/

#include "smssend.h"

#include <qcombobox.h>
#include <qlayout.h>
#include <qlabel.h>


#include <kconfigbase.h>
#include <klineedit.h>
#include <klocale.h>
#include <kurlrequester.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include "kopeteaccount.h"
#include "kopeteuiglobal.h"

#include "smssendprefs.h"
#include "smssendprovider.h"
#include "smsprotocol.h"

SMSSend::SMSSend(Kopete::Account* account)
	: SMSService(account)
{
	kWarning( 14160 ) << " this = " << this;
	prefWidget = 0L;
	m_provider = 0L;
}

SMSSend::~SMSSend()
{
	qDeleteAll(labels);
	qDeleteAll(args);
}

void SMSSend::send(const Kopete::Message& msg)
{
	kWarning( 14160 ) << "m_account = " << m_account << " (should be non-zero!!)";
	QString provider = m_account->configGroup()->readEntry("SMSSend:ProviderName", QString());

	if (provider.length() < 1)
	{
		KMessageBox::error(Kopete::UI::Global::mainWidget(), i18n("No provider configured."), i18n("Could Not Send Message"));
		return;
	}

	QString prefix = m_account->configGroup()->readEntry("SMSSend:Prefix", QString());
	if (prefix.isNull())
	{
		KMessageBox::error(Kopete::UI::Global::mainWidget(), i18n("No prefix set for SMSSend, please change it in the configuration dialog."), i18n("No Prefix"));
		return;
	}

	m_provider = new SMSSendProvider(provider, prefix, m_account, this);

	QObject::connect( m_provider, SIGNAL(messageSent(Kopete::Message)), this, SIGNAL(messageSent(Kopete::Message)));
	QObject::connect( m_provider, SIGNAL(messageNotSent(Kopete::Message,QString)), this, SIGNAL(messageNotSent(Kopete::Message,QString)));

	m_provider->send(msg);
}

void SMSSend::setWidgetContainer(QWidget* parent, QGridLayout* layout)
{
	kWarning( 14160 ) << "ml: " << layout << ", " << "mp: " << parent;
	m_parent = parent;
	m_layout = layout;

	// could end up being deleted twice??
	delete prefWidget;
	prefWidget = new SMSSendPrefsUI(parent);
	layout->addWidget(prefWidget, 0, 0, 1, 1);

	prefWidget->program->setMode(KFile::Directory);

	QString prefix;

	if (m_account)
		prefix = m_account->configGroup()->readEntry("SMSSend:Prefix", QString());
	if (prefix.isNull())
	{
		QDir d("/usr/share/smssend");
		if (d.exists())
		{
			prefix = "/usr";
		}
		d = "/usr/local/share/smssend";
		if (d.exists())
		{
			prefix="/usr/local";
		}
		else
		{
			prefix="/usr";
		}
	}

	QObject::connect (prefWidget->program, SIGNAL(textChanged(QString)),
		this, SLOT(loadProviders(QString)));

	prefWidget->program->setUrl(prefix);

	QObject::connect(prefWidget->provider, SIGNAL(activated(QString)),
		this, SLOT(setOptions(QString)));

	prefWidget->show();
}

void SMSSend::savePreferences()
{
	if (prefWidget != 0L && m_account != 0L && m_provider != 0L )
	{
		m_account->configGroup()->writeEntry("SMSSend:Prefix", prefWidget->program->url().url());
		m_account->configGroup()->writeEntry("SMSSend:ProviderName", prefWidget->provider->currentText());
		m_provider->save(args);
	}
}

void SMSSend::loadProviders(const QString &prefix)
{
	kWarning( 14160 ) << "m_account = " << m_account << " (should be ok if zero)";

	QStringList p;

	prefWidget->provider->clear();

	QDir d(prefix + "/share/smssend");
	if (!d.exists())
	{
		setOptions(QString());
		return;
	}

	p = d.entryList(QStringList(QLatin1String("*.sms")));

	d = QDir::homePath()+"/.smssend/";

	QStringList tmp(d.entryList(QStringList(QLatin1String("*.sms"))));

	for (QStringList::Iterator it = tmp.begin(); it != tmp.end(); ++it)
		p.prepend(*it);

	for (QStringList::iterator it = p.begin(); it != p.end(); ++it)
		(*it).truncate((*it).length()-4);

	prefWidget->provider->addItems(p);

	bool found = false;
	if (m_account)
	{	QString pName = m_account->configGroup()->readEntry("SMSSend:ProviderName", QString());
		for (int i=0; i < prefWidget->provider->count(); i++)
		{
			if (prefWidget->provider->itemText(i) == pName)
			{
				found=true;
				prefWidget->provider->setCurrentIndex(i);
				setOptions(pName);
				break;
			}
		}
	}
	if (!found)
		setOptions(prefWidget->provider->currentText());
}

void SMSSend::setOptions(const QString& name)
{
	kWarning( 14160 ) << "m_account = " << m_account << " (should be ok if zero!!)";
	if(!prefWidget) return;			// sanity check

	prefWidget->providerLabel->setText(i18n("%1 Settings", name));

	qDeleteAll(labels);
	labels.clear();

	qDeleteAll(args);
	args.clear();

	delete m_provider;
	m_provider = new SMSSendProvider(name, prefWidget->program->url().url(), m_account, this);

	for (int i=0; i < m_provider->count(); i++)
	{
		if (!m_provider->name(i).isNull())
		{
			QLabel *l = new QLabel(m_parent);
			l->setText('&' + m_provider->name(i) + ':');
			l->setToolTip( m_provider->description(i));
			m_layout->addWidget(l, i+2, 0);
			KLineEdit *e = new KLineEdit(m_parent);
			e->setText(m_provider->value(i));
			m_layout->addWidget(e, i+2, 1);
			args.append(e);
			labels.append(l);
			l->setBuddy(e);
			if(m_provider->isHidden(i))
				e->setEchoMode(QLineEdit::Password);
			e->show();
			l->show();
		}
	}
}
void SMSSend::setAccount(Kopete::Account* account)
{
	m_provider->setAccount(account);
	SMSService::setAccount(account);
}

int SMSSend::maxSize()
{
	kWarning( 14160 ) << "m_account = " << m_account << " (should be non-zero!!)";

	QString pName = m_account->configGroup()->readEntry("SMSSend:ProviderName", QString());
	if (pName.length() < 1)
		return 160;
	QString prefix = m_account->configGroup()->readEntry("SMSSend:Prefix", QString());
	if (prefix.isNull())
		prefix = "/usr";
	// quick sanity check
	delete m_provider;
	m_provider = new SMSSendProvider(pName, prefix, m_account, this);
	return m_provider->maxSize();
}

const QString& SMSSend::description()
{
	QString url = "http://zekiller.skytech.org/smssend_en.php";
	m_description = i18n("<qt>SMSSend is a program for sending SMS through gateways on the web. It can be found on <a href=\"%1\">%2</a></qt>", url, url);
	return m_description;
}


#include "smssend.moc"
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

