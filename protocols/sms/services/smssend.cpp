#include "smssend.h"
#include "smssendprefs.h"
#include "smssendprovider.h"
#include "smsglobal.h"

#include <qdir.h>
#include <qcombobox.h>
#include <qvgroupbox.h>
#include <qpushbutton.h>
#include <qinputdialog.h>
#include <qlayout.h>
#include <qlabel.h>
#include <klineedit.h>
#include <klocale.h>
#include <kfile.h>
#include <kurlrequester.h>
#include <kmessagebox.h>

SMSSend::SMSSend(SMSContact* contact)
	: SMSService(contact)
{
	prefWidget = 0L;
}

SMSSend::~SMSSend()
{
}

void SMSSend::send(const KopeteMessage& msg)
{
	QString provider = SMSGlobal::readConfig("SMSSend", "ProviderName", m_contact);

	if (provider == QString::null)
	{
		KMessageBox::error(0L, i18n("No provider configured"), i18n("Could not send message"));
		return;
	}

	QString prefix = SMSGlobal::readConfig("SMSSend", "Prefix", m_contact);
	if (prefix == QString::null)
		prefix = "/usr/";

	SMSSendProvider* s = new SMSSendProvider(provider, prefix, m_contact, this);

	connect( s, SIGNAL(messageSent(const KopeteMessage &)), this, SIGNAL(messageSent(const KopeteMessage &)));

	s->send(msg);
}

QWidget* SMSSend::configureWidget(QWidget* parent)
{
	if (prefWidget == 0L)
		prefWidget = new SMSSendPrefsUI(parent);
	
	prefWidget->program->setMode(KFile::Directory);
	QString prefix = SMSGlobal::readConfig("SMSSend", "Prefix", m_contact);
	if (prefix == QString::null)
		prefix = "/usr";

	prefWidget->program->setURL(prefix);
	
	prefWidget->provider->insertStringList(providers());

	connect(prefWidget->provider, SIGNAL(activated(const QString &)),
		this, SLOT(setOptions(const QString &)));
	
	QString pName = SMSGlobal::readConfig("SMSSend", "ProviderName", m_contact);
	for (int i=0; i < prefWidget->provider->count(); i++)
	{
		if (prefWidget->provider->text(i) == pName)
		{
			prefWidget->provider->setCurrentItem(i);
			setOptions(pName);
			break;
		}
	}
	
	return prefWidget;
}

void SMSSend::savePreferences()
{
	if (prefWidget != 0L)
	{
		SMSGlobal::writeConfig("SMSSend", "Prefix", m_contact, prefWidget->program->url());
		SMSGlobal::writeConfig("SMSSend", "ProviderName", m_contact, prefWidget->provider->currentText());
		SMSSendProvider* s = new SMSSendProvider(prefWidget->provider->currentText(),
			prefWidget->program->url(), m_contact, this);
		s->save(args);
	}
}

QStringList SMSSend::providers()
{
	QStringList p;

	QDir d = QDir::homeDirPath()+"/.smssend/";
	p = d.entryList("*.sms");
		d.setPath(QString("%1/share/smssend/").arg(prefWidget->program->url()));
	p += d.entryList("*.sms");

	for (QStringList::iterator it = p.begin(); it != p.end(); ++it)
		(*it).truncate((*it).length()-4);

	return p;
}

void SMSSend::setOptions(const QString& name)
{
	prefWidget->settingsBox->setTitle(name);

	args.setAutoDelete(true);
	args.clear();

	SMSSendProvider* s = new SMSSendProvider(name, prefWidget->program->url(), m_contact, this);

	for (int i=0; i < s->count(); i++)
	{
		if (s->name(i) != QString::null)
		{
			SMSSendArg* a = new SMSSendArg(prefWidget->settingsBox);
			a->argName->setText(s->name(i));
			a->value->setText(s->value(i));
			a->description->setText(s->description(i));
			args.append(a);
			a->show();
		}
	}
}

int SMSSend::maxSize()
{
	QString pName = SMSGlobal::readConfig("SMSSend", "ProviderName", m_contact);
	QString prefix = SMSGlobal::readConfig("SMSSend", "Prefix", m_contact);
	if (prefix == QString::null)
		prefix = "/usr";
	SMSSendProvider* s = new SMSSendProvider(pName, prefix, m_contact, this);
	return s->maxSize();
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

