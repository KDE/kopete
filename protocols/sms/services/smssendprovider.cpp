#include <qvaluelist.h>
#include <qlabel.h>
#include <qfile.h>

#include <kprocess.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <klocale.h>

#include "kopeteaccount.h"

#include "smssendprovider.h"
#include "smsprotocol.h"

SMSSendProvider::SMSSendProvider(const QString& providerName, const QString& prefixValue, KopeteAccount* account, QObject* parent, const char *name)
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

				descriptions.append(args[1]);
				if (m_account)
					values.append(m_account->pluginData(SMSProtocol::protocol(), QString("%1:%2").arg(group).arg(names[names.count()-1])));
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

void SMSSendProvider::setAccount(KopeteAccount *account)
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
	kdWarning( 14160 ) << k_funcinfo << "m_account = " << m_account << " (should be non-zero!!)" << endl;
	if (!m_account) return;		// prevent crash in worst case

	QString group = QString("SMSSend-%1").arg(provider);

	for (unsigned i=0; i < args.count(); i++)
	{
		if (!args.at(i)->text().isEmpty())
		{	values[i] = args.at(i)->text();
			m_account->setPluginData(SMSProtocol::protocol(), QString("%1:%2").arg(group).arg(names[i]), values[i]);
		}
	}
}

int SMSSendProvider::count()
{
	return names.count();
}

void SMSSendProvider::send(const KopeteMessage& msg)
{
	if ( canSend == false )
	{
		if ( messagePos == -1 )
		{
			canSend = false;
			KMessageBox::error(0L, i18n("Could not determine which argument which should contain the message."),
				i18n("Could Not Send Message"));
			return;
		}
		if ( telPos == -1 )
		{
			canSend = false;

			KMessageBox::error(0L, i18n("Could not determine which argument which should contain the number."),
				i18n("Could Not Send Message"));
			return;
		}
	}

	m_msg = msg;

	QString message = msg.plainBody();
	QString nr = msg.to().first()->contactId();

	if (canSend = false)
		return;

	values[messagePos] = message;
	values[telPos] = nr;

	KProcess* p = new KProcess;

	*p << QString("%1/bin/smssend").arg(prefix) << provider << values;

	output.clear();
	connect( p, SIGNAL(processExited(KProcess *)), this, SLOT(slotSendFinished(KProcess *)));
	connect( p, SIGNAL(receivedStdout(KProcess*, char*, int)), this, SLOT(slotReceivedOutput(KProcess*, char*, int)));
	connect( p, SIGNAL(receivedStderr(KProcess*, char*, int)), this, SLOT(slotReceivedOutput(KProcess*, char*, int)));

	bool ps = p->start(KProcess::NotifyOnExit, KProcess::AllOutput);

	kdWarning( 14160 ) << k_funcinfo << "this = " << this << ", ps = " << ps << ", p = " << p << " (should be non-zero!!)" << endl;

}

void SMSSendProvider::slotSendFinished(KProcess* p)
{
	kdWarning( 14160 ) << k_funcinfo << "this = " << this << ", es = " << p->exitStatus() << ", p = " << p << " (should be non-zero!!)" << endl;
	if (p->exitStatus() == 0)
		emit messageSent(m_msg);
	else
		emit messageNotSent(m_msg, output.join("\n"));

	// TODO: is there a cleaner way of doing this?
	delete p;
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

