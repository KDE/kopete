/*  *************************************************************************
    *   copyright: (C) 2003 Richard Lärkäng <nouseforaname@home.se>         *
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

#include <qvaluelist.h>
#include <qlabel.h>
#include <qfile.h>

#include <kconfigbase.h>
#include <kprocess.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <klocale.h>

#include "kopeteaccount.h"
#include "kopeteuiglobal.h"

#include "smssendprovider.h"
#include "smsprotocol.h"
#include "smscontact.h"

SMSSendProvider::SMSSendProvider(const QString& providerName, const QString& prefixValue, Kopete::Account* account, QObject* parent, const char *name)
	: QObject( parent, name ), m_account(account)
{
	kdWarning( 14160 ) << k_funcinfo << "this = " << this << ", m_account = " << m_account << " (should be ok if zero!!)" << endl;

	provider = providerName;
	prefix = prefixValue;
	m_maxSize = 160;

	messagePos = -1;
	telPos = -1;

	QString file = prefix + "/share/smssend/" + provider + ".sms";
	QFile f(file);
	if (f.open(IO_ReadOnly))
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

		while( !t.eof())
		{
			QString s = t.readLine();
			if( s[0] == '%')
			{
				QStringList args = QStringList::split(':',s);
				QStringList options = QStringList::split(' ', args[0]);

				names.append(options[0].replace(0,1,""));

				bool hidden = false;
				for(unsigned i = 1; i < options.count(); i++)
					if(options[i] == "Hidden")
					{	hidden = true;
						break;
					}
				isHiddens.append(hidden);

				// Strip trailing whitespace in the end
				// and '%' in the beginning
				args[0] = args[0].simplifyWhiteSpace().mid(1);

				descriptions.append(args[1]);
				if (m_account)
					values.append(m_account->configGroup()->readEntry(QString("%1:%2").arg(group).arg(names[names.count()-1]),
					                                                  QString::null));
				else
					values.append("");

				if( args[0].contains("Message") || args[0].contains("message")
					|| args[0].contains("message") || args[0].contains("nachricht")
					|| args[0].contains("Msg") || args[0].contains("Mensagem") )
				{
					for( unsigned i = 0; i < options.count(); i++)
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
//								kdDebug(14160) << "Exact match for " << args[0] << endl;
								exactNumberMatch = true;
							}
//							kdDebug(14160) << "args[0] (" << args[0] << ") contains " << *it << endl;
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
	kdWarning( 14160 ) << k_funcinfo << "this = " << this << endl;
}

void SMSSendProvider::setAccount(Kopete::Account *account)
{
	m_account = account;
}

const QString& SMSSendProvider::name(int i)
{
	if ( telPos == i || messagePos == i)
		return QString::null;
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

const bool SMSSendProvider::isHidden(int i)
{
	return isHiddens[i];
}

void SMSSendProvider::save(QPtrList<KLineEdit>& args)
{
	kdDebug( 14160 ) << k_funcinfo << "m_account = " << m_account << " (should be non-zero!!)" << endl;
	if (!m_account) return;		// prevent crash in worst case

	QString group = QString("SMSSend-%1").arg(provider);
	int namesI=0;

	for (unsigned i=0; i < args.count(); i++)
	{
		if (telPos == namesI || messagePos == namesI)
		{
//		    kdDebug(14160) << k_funcinfo << "Skipping pos " << namesI << endl;
		    namesI++;
		    if (telPos == namesI || messagePos == namesI)
		    {
//		        kdDebug(14160) << k_funcinfo << "Skipping pos " << namesI << endl;
		        namesI++;
		    }
		}

//                kdDebug(14160) << k_funcinfo << "saving " << args.at(i) << " to " << names[namesI] << endl;
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
			KMessageBox::error(Kopete::UI::Global::mainWidget(), i18n("Could not determine which argument which should contain the message."),
				i18n("Could Not Send Message"));
			return;
		}
		if ( telPos == -1 )
		{
			canSend = false;

			KMessageBox::error(Kopete::UI::Global::mainWidget(), i18n("Could not determine which argument which should contain the number."),
				i18n("Could Not Send Message"));
			return;
		}
	}

	m_msg = msg;

	QString message = msg.plainBody();
	QString nr = dynamic_cast<SMSContact *>(msg.to().first())->qualifiedNumber();

	if (canSend = false)
		return;

	values[messagePos] = message;
	values[telPos] = nr;

	KProcess* p = new KProcess;

	kdWarning( 14160 ) << "Executing " << QString("%1/bin/smssend").arg(prefix) << " \"" << provider << "\" " << values.join("\" \"") << "\"" << endl;

	*p << QString("%1/bin/smssend").arg(prefix) << provider << values;

	output = "";
	connect( p, SIGNAL(processExited(KProcess *)), this, SLOT(slotSendFinished(KProcess *)));
	connect( p, SIGNAL(receivedStdout(KProcess *, char *, int)), this, SLOT(slotReceivedOutput(KProcess *, char *, int)));
//	connect( p, SIGNAL(receivedStderr(KProcess *, char *, int)), this, SLOT(slotReceivedOutput(KProcess *, char *, int)));

	p->start(KProcess::NotifyOnExit, KProcess::AllOutput);
}

void SMSSendProvider::slotSendFinished(KProcess *p)
{
	kdWarning( 14160 ) << k_funcinfo << "this = " << this << ", es = " << p->exitStatus() << ", p = " << p << " (should be non-zero!!)" << endl;
	if (p->exitStatus() == 0)
		emit messageSent(m_msg);
	else
		emit messageNotSent(m_msg, QString().setLatin1(output));

	p->deleteLater();
}

void SMSSendProvider::slotReceivedOutput(KProcess *, char *buffer, int buflen)
{
//	QStringList lines = QStringList::split("\n", QString::fromLocal8Bit(buffer, buflen));
//	for (QStringList::Iterator it = lines.begin(); it != lines.end(); ++it)
	for(int i = 0; i < buflen; i++)
		output += buffer[i];
	kdWarning( 14160 ) << k_funcinfo << " output now = " << output << endl;
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

