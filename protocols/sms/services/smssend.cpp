#include "smssend.h"
#include "smssendprefs.h"
#include "smssendprovider.h"
#include "smsglobal.h"

#include <qdir.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qinputdialog.h>
#include <klistview.h>
#include <klocale.h>
#include <kfile.h>
#include <kurlrequester.h>
#include <kmessagebox.h>
#include <kdebug.h>

SMSSend::SMSSend(QString userName)
	: SMSService(userName)
{
	prefWidget = 0L;
}

SMSSend::~SMSSend()
{
}

void SMSSend::send(const KopeteMessage& msg)
{
	kdDebug() << "SMSSend::send()" << endl;

	QString provider = SMSGlobal::readConfig("SMSSend", "ProviderName", uName);

	if (provider == QString::null)
	{
		KMessageBox::error(0L, i18n("No provider configured"), i18n("Could not send message"));
		return;
	}

	QString prefix = SMSGlobal::readConfig("SMSSend", "Prefix", uName);
	if (prefix == QString::null)
		prefix = "/usr/";

	SMSSendProvider* s = new SMSSendProvider(provider, prefix, uName, this);

	connect( s, SIGNAL(messageSent(const KopeteMessage &)), this, SIGNAL(messageSent(const KopeteMessage &)));

	s->send(msg);
}

QWidget* SMSSend::configureWidget(QWidget* parent)
{
	if (prefWidget == 0L)
		prefWidget = new SMSSendPrefsUI(parent);
	
	prefWidget->program->setMode(KFile::Directory);
	QString prefix = SMSGlobal::readConfig("SMSSend", "Prefix", uName);
	if (prefix == QString::null)
		prefix = "/usr";

	prefWidget->program->setURL(prefix);
	
	prefWidget->provider->insertStringList(providers());

	connect(prefWidget->provider, SIGNAL(activated(const QString &)),
		this, SLOT(setOptions(const QString &)));
	
	QString pName = SMSGlobal::readConfig("SMSSend", "ProviderName", uName);
	for (int i=0; i < prefWidget->provider->count(); i++)
	{
		if (prefWidget->provider->text(i) == pName)
		{
			prefWidget->provider->setCurrentItem(i);
			setOptions(pName);
			break;
		}
	}
	
	connect(prefWidget->descButton, SIGNAL(clicked()), this, SLOT(showDescription()));
	connect(prefWidget->saveButton, SIGNAL(clicked()), this, SLOT(saveProviderPreferences()));
	connect(prefWidget->providerSettings, SIGNAL(executed(QListViewItem*)),
		this, SLOT(changeOption(QListViewItem*)));

	return prefWidget;
}

void SMSSend::savePreferences()
{
	if (prefWidget != 0L)
	{
		SMSGlobal::writeConfig("SMSSend", "Prefix", uName, prefWidget->program->url());
		SMSGlobal::writeConfig("SMSSend", "ProviderName", uName, prefWidget->provider->currentText());
		if (prefWidget->providerSettings->childCount() > 0)
			saveProviderPreferences();
	}
}

void SMSSend::saveProviderPreferences()
{
	SMSSendProvider* s = new SMSSendProvider(prefWidget->provider->currentText(), prefWidget->program->url(), uName, this);
	s->save(prefWidget->providerSettings);
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
	prefWidget->providerSettings->clear();

	SMSSendProvider* s = new SMSSendProvider(name, prefWidget->program->url(), uName, this);

	for (int i=0; i < s->count(); i++)
		prefWidget->providerSettings->insertItem(s->listItem(prefWidget->providerSettings, i));
}

void SMSSend::showDescription()
{
	if (prefWidget->providerSettings->currentItem() != 0L)
	{
		SMSSendProvider* s = new SMSSendProvider(prefWidget->provider->currentText(), prefWidget->program->url(), uName, this);
		s->showDescription(prefWidget->providerSettings->currentItem()->text(0));
	}
}

void SMSSend::changeOption(QListViewItem* i)
{
	bool ok;
	QString text = QInputDialog::getText(
		i->text(0), i18n("Enter a value for %1:").arg(i->text(0)),
		QLineEdit::Normal, QString::null, &ok);

	if ( ok )
		i->setText(1, text);
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

