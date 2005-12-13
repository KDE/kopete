/*  *************************************************************************
    *   copyright: (C) 2003 Richard L�k�g <nouseforaname@home.se>         *
    *   copyright: (C) 2003 Gav Wood <gav@kde.org>                          *
    *************************************************************************
*/

/*  *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qcombobox.h>
#include <qvgroupbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qtooltip.h>

#include <kconfigbase.h>
#include <klineedit.h>
#include <klocale.h>
#include <kurlrequester.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include "kopeteaccount.h"
#include "kopeteuiglobal.h"

#include "smssend.h"
#include "smssendprefs.h"
#include "smssendprovider.h"
#include "smsprotocol.h"

SMSSend::SMSSend(Kopete::Account* account)
	: SMSService(account)
{
	kdWarning( 14160 ) << k_funcinfo << " this = " << this << endl;
	prefWidget = 0L;
	m_provider = 0L;
}

SMSSend::~SMSSend()
{
}

void SMSSend::send(const Kopete::Message& msg)
{
	kdWarning( 14160 ) << k_funcinfo << "m_account = " << m_account << " (should be non-zero!!)" << endl;
	QString provider = m_account->configGroup()->readEntry("SMSSend:ProviderName", QString::null);

	if (provider.length() < 1)
	{
		KMessageBox::error(Kopete::UI::Global::mainWidget(), i18n("No provider configured."), i18n("Could Not Send Message"));
		return;
	}

	QString prefix = m_account->configGroup()->readEntry("SMSSend:Prefix", QString::null);
	if (prefix.isNull())
	{
		KMessageBox::error(Kopete::UI::Global::mainWidget(), i18n("No prefix set for SMSSend, please change it in the configuration dialog."), i18n("No Prefix"));
		return;
	}

	m_provider = new SMSSendProvider(provider, prefix, m_account, this);

	QObject::connect( m_provider, SIGNAL(messageSent(const Kopete::Message &)), this, SIGNAL(messageSent(const Kopete::Message &)));
	QObject::connect( m_provider, SIGNAL(messageNotSent(const Kopete::Message &, const QString &)), this, SIGNAL(messageNotSent(const Kopete::Message &, const QString &)));

	m_provider->send(msg);
}

void SMSSend::setWidgetContainer(QWidget* parent, QGridLayout* layout)
{
	kdWarning( 14160 ) << k_funcinfo << "ml: " << layout << ", " << "mp: " << parent << endl;
	m_parent = parent;
	m_layout = layout;

	// could end up being deleted twice??
	delete prefWidget;
	prefWidget = new SMSSendPrefsUI(parent);
	layout->addMultiCellWidget(prefWidget, 0, 1, 0, 1);

	prefWidget->program->setMode(KFile::Directory);

	QString prefix = QString::null;

	if (m_account)
		prefix = m_account->configGroup()->readEntry("SMSSend:Prefix", QString::null);
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

	QObject::connect (prefWidget->program, SIGNAL(textChanged(const QString &)),
		this, SLOT(loadProviders(const QString&)));

	prefWidget->program->setURL(prefix);

	QObject::connect(prefWidget->provider, SIGNAL(activated(const QString &)),
		this, SLOT(setOptions(const QString &)));

	prefWidget->show();
}

void SMSSend::savePreferences()
{
	if (prefWidget != 0L && m_account != 0L && m_provider != 0L )
	{
		m_account->configGroup()->writeEntry("SMSSend:Prefix", prefWidget->program->url());
		m_account->configGroup()->writeEntry("SMSSend:ProviderName", prefWidget->provider->currentText());
		m_provider->save(args);
	}
}

void SMSSend::loadProviders(const QString &prefix)
{
	kdWarning( 14160 ) << k_funcinfo << "m_account = " << m_account << " (should be ok if zero)" << endl;

	QStringList p;

	prefWidget->provider->clear();

	QDir d(prefix + "/share/smssend");
	if (!d.exists())
	{
		setOptions(QString::null);
		return;
	}

	p = d.entryList("*.sms");

	d = QDir::homeDirPath()+"/.smssend/";

	QStringList tmp(d.entryList("*.sms"));

	for (QStringList::Iterator it = tmp.begin(); it != tmp.end(); ++it)
		p.prepend(*it);

	for (QStringList::iterator it = p.begin(); it != p.end(); ++it)
		(*it).truncate((*it).length()-4);

	prefWidget->provider->insertStringList(p);

	bool found = false;
	if (m_account)
	{	QString pName = m_account->configGroup()->readEntry("SMSSend:ProviderName", QString::null);
		for (int i=0; i < prefWidget->provider->count(); i++)
		{
			if (prefWidget->provider->text(i) == pName)
			{
				found=true;
				prefWidget->provider->setCurrentItem(i);
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
	kdWarning( 14160 ) << k_funcinfo << "m_account = " << m_account << " (should be ok if zero!!)" << endl;
	if(!prefWidget) return;			// sanity check

	prefWidget->providerLabel->setText(i18n("%1 Settings").arg(name));

	labels.setAutoDelete(true);
	labels.clear();
	args.setAutoDelete(true);
	args.clear();

	if (m_provider) delete m_provider;
	m_provider = new SMSSendProvider(name, prefWidget->program->url(), m_account, this);

	for (int i=0; i < m_provider->count(); i++)
	{
		if (!m_provider->name(i).isNull())
		{
			QLabel *l = new QLabel(m_parent);
			l->setText("&" + m_provider->name(i) + ":");
			QToolTip::add(l, m_provider->description(i));
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
	kdWarning( 14160 ) << k_funcinfo << "m_account = " << m_account << " (should be non-zero!!)" << endl;

	QString pName = m_account->configGroup()->readEntry("SMSSend:ProviderName", QString::null);
	if (pName.length() < 1)
		return 160;
	QString prefix = m_account->configGroup()->readEntry("SMSSend:Prefix", QString::null);
	if (prefix.isNull())
		prefix = "/usr";
	// quick sanity check
	if (m_provider) delete m_provider;
	m_provider = new SMSSendProvider(pName, prefix, m_account, this);
	return m_provider->maxSize();
}

const QString& SMSSend::description()
{
	QString url = "http://zekiller.skytech.org/smssend_en.php";
	m_description = i18n("<qt>SMSSend is a program for sending SMS through gateways on the web. It can be found on <a href=\"%1\">%2</a></qt>").arg(url).arg(url);
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

