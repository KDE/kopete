#include "smssendprovider.h"
#include "smsglobal.h"
#include "smscontact.h"

#include <kdeversion.h>
#if KDE_VERSION > 305
#include <kprocio.h>
#else
#include <kprocess.h>
#endif
#include <qregexp.h>
#include <qlabel.h>
#include <qfile.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <klocale.h>

SMSSendProvider::SMSSendProvider(QString providerName, QString prefixValue, SMSContact* contact, QObject* parent, const char *name)
	: QObject( parent, name )
{

	provider = providerName;
	prefix = prefixValue;
	m_contact = contact;

	messagePos = -1;
	telPos = -1;

	QString file = prefix + "/share/smssend/" + provider + ".sms";
	QFile f(file);
	if (f.open(IO_ReadOnly))
	{
		QTextStream t(&f);
		QString group = QString("SMSSend-%1").arg(provider);
		while( !t.eof())
		{
			QString s = t.readLine();
			if( s[0] == '%')
			{
				QStringList args = QStringList::split(':',s);
				QStringList options = QStringList::split(' ', args[0]);

				names.append(options[0].replace(0,1,""));
				descriptions.append(args[1]);
				values.append(SMSGlobal::readConfig(group, names[names.count()-1], m_contact));

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
				else if ( args[0].contains("Tel") || args[0].contains("Number")
					|| args[0].contains("number") || args[0].contains("TelNum")
					|| args[0].contains("Recipient") || args[0].contains("Tel1")
					|| args[0].contains("To") || args[0].contains("nummer")
					|| args[0].contains("telefone") || args[0].contains("ToPhone") )
				{
					telPos = names.count() - 1;
				}
			}
		}
	}
	f.close();

	if ( messagePos == -1 )
	{
		canSend = false;
		KMessageBox::error(0L, i18n("Could not determine which argument which should contain the message"),
			i18n("Could not send message"));
		return;
	}
	if ( telPos == -1 )
	{
		canSend = false;

		KMessageBox::error(0L, i18n("Could not determine which argument which should contain the number"),
			i18n("Could not send message"));
		return;
	}

	canSend = true;
}

SMSSendProvider::~SMSSendProvider()
{

}

QString SMSSendProvider::name(int i)
{
	if ( telPos == i || messagePos == i)
		return QString::null;
	else
		return names[i];
}

QString SMSSendProvider::value(int i)
{
	return values[i];
}

QString SMSSendProvider::description(int i)
{
	return descriptions[i];
}

void SMSSendProvider::save(QPtrList<SMSSendArg> args)
{
	QString group = QString("SMSSend-%1").arg(provider);

	for (unsigned i=0; i < args.count(); i++)
	{
		if (args.at(i)->value->text() == "")
			SMSGlobal::deleteConfig(group, args.at(i)->argName->text(), m_contact);
		else
			SMSGlobal::writeConfig(group, args.at(i)->argName->text(), m_contact, args.at(i)->value->text());
	}
}

int SMSSendProvider::count()
{
	return names.count();
}

void SMSSendProvider::send(const KopeteMessage& msg)
{
	m_msg = msg;

	QString message = msg.plainBody();
	QString nr = msg.to().first()->id();

	if (canSend = false)
		return;

	values[messagePos] = message;
	values[telPos] = nr;

#if KDE_VERSION > 305
	KProcIO* p = new KProcIO;
#else
	KProcess* p = new KProcess;
#endif

	*p << QString("%1/bin/smssend").arg(prefix) << provider;
    for( unsigned int i = 0; i < values.count(); ++i)
      *p << values[i];
    
	output.clear();
	connect( p, SIGNAL(processExited(KProcess *)), this, SLOT(slotSendFinished(KProcess*)));
	connect( p, SIGNAL(receivedStdout(KProcess*, char*, int)),
		this, SLOT(slotReceivedOutput(KProcess*, char*, int)));
	connect( p, SIGNAL(receivedStderr(KProcess*, char*, int)),
		this, SLOT(slotReceivedOutput(KProcess*, char*, int)));

	p->start(KProcess::Block);
}

void SMSSendProvider::slotSendFinished(KProcess* p)
{
	if (p->exitStatus() == 0)
	{
		KMessageBox::information(0L, i18n("Message sent"), output.join("\n"), i18n("Message sent"));

		emit messageSent(m_msg);
	}
	else
	{
		KMessageBox::detailedError(0L, i18n("Something went wrong when sending message"), output.join("\n"),
				i18n("Could not send message"));
	}
}

void SMSSendProvider::slotReceivedOutput(KProcess*, char  *buffer, int  buflen)
{
	QStringList lines = QStringList::split("\n", QString::fromLocal8Bit(buffer, buflen));
	for (QStringList::Iterator it = lines.begin(); it != lines.end(); ++it)
		output.append(*it);
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

