/*
  smssendprovider.cpp  -  SMS Plugin

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

#include "smssendprovider.h"

#include <QFile>
#include <QTextStream>
#include <QList>

#include <kconfigbase.h>
#include <k3process.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <klocale.h>

#include "kopeteaccount.h"
#include "kopeteuiglobal.h"

#include "smsprotocol.h"
#include "smscontact.h"

SMSSendProvider::SMSSendProvider(const QString& providerName, const QString& prefixValue, Kopete::Account* account, QObject* parent)
	: QObject( parent ), m_account(account)
{
	kWarning( 14160 ) << "this = " << this << ", m_account = " << m_account << " (should be ok if zero!!)";

	provider = providerName;
	prefix = prefixValue;
	m_maxSize = 160;

	messagePos = -1;
	telPos = -1;

	QString file = prefix + "/share/smssend/" + provider + ".sms";
	QFile f(file);
	if (f.open(QIODevice::ReadOnly))
	{
		QTextStream t(&f);
		QString group = QString("SMSSend-%1").arg(provider);
		bool exactNumberMatch = false;
		QStringList numberWords;
		numberWords.append("Tel");
                numberWords.append("Number");
		numberWords.append("number");
		numberWords.append("TelNum");
		numberWords.append("Recipient");
		numberWords.append("Tel1");
		numberWords.append("To");
		numberWords.append("nummer");
		numberWords.append("telefone");
		numberWords.append("ToPhone");

		while( !t.atEnd())
		{
			QString s = t.readLine();
			if( s[0] == '%')
			{
				QStringList args = s.split(':');
				QStringList options = args[0].split(' ');

				names.append(options[0].replace(0,1,""));

				bool hidden = false;
				for(int i = 1; i < options.count(); i++)
					if(options[i] == "Hidden")
					{	hidden = true;
						break;
					}
				isHiddens.append(hidden);

				// Strip trailing whitespace in the end
				// and '%' in the beginning
				args[0] = args[0].simplified().mid(1);

				descriptions.append(args[1]);
				if (m_account)
					values.append(m_account->configGroup()->readEntry(QString("%1:%2").arg(group).arg(names[names.count()-1]),
					                                                  QString()));
				else
					values.append("");

				if( args[0].contains("Message") || args[0].contains("message")
					|| args[0].contains("message") || args[0].contains("nachricht")
					|| args[0].contains("Msg") || args[0].contains("Mensagem") )
				{
					for( int i = 0; i < options.count(); i++)
					{
						if (options[i].contains("Size="))
						{
							QString option = options[i];
							option.replace(0,5,"");
							m_maxSize = option.toInt();
						}
					}
					messagePos = names.count()-1;
				}
				else if (!exactNumberMatch)
				{
					for (QStringList::Iterator it=numberWords.begin(); it != numberWords.end(); ++it)
					{
						if (args[0].contains(*it))
						{
							telPos = names.count() - 1;
							if (args[0] == *it)
							{
//								kDebug(14160) << "Exact match for " << args[0];
								exactNumberMatch = true;
							}
//							kDebug(14160) << "args[0] (" << args[0] << ") contains " << *it;
						}
					}
				}
			}
		}
	}
	f.close();

	if ( messagePos == -1 || telPos == -1 )
	{
		canSend = false;
		return;
	}

	canSend = true;
}

SMSSendProvider::~SMSSendProvider()
{
	kWarning( 14160 ) << "this = " << this;
}

void SMSSendProvider::setAccount(Kopete::Account *account)
{
	m_account = account;
}

QString SMSSendProvider::name(int i)
{
	if ( telPos == i || messagePos == i)
		return QString();
	else
		return names[i];
}

const QString& SMSSendProvider::value(int i)
{
	return values[i];
}

const QString& SMSSendProvider::description(int i)
{
	return descriptions[i];
}

bool SMSSendProvider::isHidden(int i) const
{
	return isHiddens[i];
}

void SMSSendProvider::save(const QList<KLineEdit*>& args)
{
	kDebug( 14160 ) << "m_account = " << m_account << " (should be non-zero!!)";
	if (!m_account) return;		// prevent crash in worst case

	QString group = QString("SMSSend-%1").arg(provider);
	int namesI=0;

	for (int i=0; i < args.count(); i++)
	{
		if (telPos == namesI || messagePos == namesI)
		{
//		    kDebug(14160) << "Skipping pos " << namesI;
		    namesI++;
		    if (telPos == namesI || messagePos == namesI)
		    {
//		        kDebug(14160) << "Skipping pos " << namesI;
		        namesI++;
		    }
		}

//                kDebug(14160) << "saving " << args.at(i) << " to " << names[namesI];
		if (!args.at(i)->text().isEmpty())
		{	values[namesI] = args.at(i)->text();
			m_account->configGroup()->writeEntry(QString("%1:%2").arg(group).arg(names[namesI]), values[namesI]);
		}
	        namesI++;
	}
}

int SMSSendProvider::count()
{
	return names.count();
}

void SMSSendProvider::send(const Kopete::Message& msg)
{
	if ( canSend == false )
	{
		if ( messagePos == -1 )
		{
			canSend = false;
			KMessageBox::error(Kopete::UI::Global::mainWidget(), i18n("Could not determine which argument should contain the message."),
				i18n("Could Not Send Message"));
			return;
		}
		if ( telPos == -1 )
		{
			canSend = false;

			KMessageBox::error(Kopete::UI::Global::mainWidget(), i18n("Could not determine which argument should contain the number."),
				i18n("Could Not Send Message"));
			return;
		}
	}

	m_msg = msg;

	QString message = msg.plainBody();
	QString nr = dynamic_cast<SMSContact *>(msg.to().first())->qualifiedNumber();

	if (canSend == false)
		return;

	values[messagePos] = message;
	values[telPos] = nr;

	K3Process* p = new K3Process;

	kWarning( 14160 ) << "Executing " << QString("%1/bin/smssend").arg(prefix) << " \"" << provider << "\" " << values.join("\" \"") << "\"";

	*p << QString("%1/bin/smssend").arg(prefix) << provider << values;

	output = "";
	connect( p, SIGNAL(processExited(K3Process*)), this, SLOT(slotSendFinished(K3Process*)));
	connect( p, SIGNAL(receivedStdout(K3Process*,char*,int)), this, SLOT(slotReceivedOutput(K3Process*,char*,int)));
//	connect( p, SIGNAL(receivedStderr(K3Process*,char*,int)), this, SLOT(slotReceivedOutput(K3Process*,char*,int)));

	p->start(K3Process::NotifyOnExit, K3Process::AllOutput);
}

void SMSSendProvider::slotSendFinished(K3Process *p)
{
	kWarning( 14160 ) << "this = " << this << ", es = " << p->exitStatus() << ", p = " << p << " (should be non-zero!!)";
	if (p->exitStatus() == 0)
		emit messageSent(m_msg);
	else
		emit messageNotSent(m_msg, QString::fromLatin1(output));

	p->deleteLater();
}

void SMSSendProvider::slotReceivedOutput(K3Process *, char *buffer, int buflen)
{
//	QStringList lines = QStringList::split("\n", QString::fromLocal8Bit(buffer, buflen));
//	for (QStringList::Iterator it = lines.begin(); it != lines.end(); ++it)
	for(int i = 0; i < buflen; i++)
		output += buffer[i];
	kWarning( 14160 ) << " output now = " << output;
}

int SMSSendProvider::maxSize()
{
	return m_maxSize;
}

#include "smssendprovider.moc"
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

