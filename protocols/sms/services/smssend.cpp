#include "smssend.h"
#include "smssendprefs.h"
#include "smssendprovider.h"
#include <qdir.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qinputdialog.h>
#include <klistview.h>
#include <klocale.h>
#include <kfile.h>
#include <kurlrequester.h>
#include <kconfig.h>
#include <kmessagebox.h>

SMSSend::SMSSend()
	: SMSService()
{
	prefWidget = 0L;
}

SMSSend::~SMSSend()
{
	if (prefWidget != 0L)
		delete prefWidget;
}

bool SMSSend::send(QString nr, QString message)
{
	KGlobal::config()->setGroup("SMSSend");
	QString provider = KGlobal::config()->readEntry("ProviderName", QString::null);
	if (provider == QString::null)
	{
		KMessageBox::error(0L, i18n("No provider configured"), i18n("Could not send message"));
		return false;
	}
	QString prefix = KGlobal::config()->readEntry("Prefix", "/usr");
	SMSSendProvider s(provider, prefix);
	return s.send(nr, message);
}

QWidget* SMSSend::configureWidget(QWidget* parent)
{
	if (prefWidget == 0L)
		prefWidget = new SMSSendPrefsUI(parent);
	
	prefWidget->program->setMode(KFile::Directory);
	KGlobal::config()->setGroup("SMSSend");
	prefWidget->program->setURL(KGlobal::config()->readEntry("Prefix", "/usr/"));
	
	prefWidget->provider->insertStringList(providers());

	connect(prefWidget->provider, SIGNAL(activated(const QString &)),
		this, SLOT(setOptions(const QString &)));
	
	QString pName = KGlobal::config()->readEntry( "ProviderName", QString::null );
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
		KGlobal::config()->setGroup("SMSSend");
		KGlobal::config()->writeEntry("Prefix", prefWidget->program->url());
		KGlobal::config()->writeEntry("ProviderName", prefWidget->provider->currentText());
		if (prefWidget->providerSettings->childCount() > 0)
			saveProviderPreferences();
	}
}

void SMSSend::saveProviderPreferences()
{
	SMSSendProvider s(prefWidget->provider->currentText(), prefWidget->program->url());
	s.save(prefWidget->providerSettings);
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

	SMSSendProvider s(name, prefWidget->program->url());

	for (int i=0; i < s.count(); i++)
		prefWidget->providerSettings->insertItem(s.listItem(prefWidget->providerSettings, i));
}

void SMSSend::showDescription()
{
	SMSSendProvider s(prefWidget->provider->currentText(), prefWidget->program->url());
	s.showDescription(prefWidget->providerSettings->currentItem()->text(0));
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

