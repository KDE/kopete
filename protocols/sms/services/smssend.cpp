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

	if (provider.length() < 1)
	{
		KMessageBox::error(0L, i18n("No provider configured"), i18n("Could Not Send Message"));
		return;
	}

	QString prefix = SMSGlobal::readConfig("SMSSend", "Prefix", m_contact);
	if (prefix == QString::null)
	{
		KMessageBox::error(0L, i18n("No prefix set for SMSSend, please change it in the configuration dialog"), i18n("No Prefix"));
		return;
	}

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
	
	connect (prefWidget->program, SIGNAL(textChanged(const QString &)),
		this, SLOT(loadProviders(const QString&)));

	prefWidget->program->setURL(prefix);
	
	loadProviders(prefix);

	connect(prefWidget->provider, SIGNAL(activated(const QString &)),
		this, SLOT(setOptions(const QString &)));
	
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

void SMSSend::loadProviders(const QString &prefix)
{
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

	QString pName = SMSGlobal::readConfig("SMSSend", "ProviderName", m_contact);
	bool found=false;
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
	if (!found)
		setOptions(prefWidget->provider->currentText());
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
	if (pName.length() < 1)
		return 160;
	QString prefix = SMSGlobal::readConfig("SMSSend", "Prefix", m_contact);
	if (prefix == QString::null)
		prefix = "/usr";
	SMSSendProvider* s = new SMSSendProvider(pName, prefix, m_contact, this);
	return s->maxSize();
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

