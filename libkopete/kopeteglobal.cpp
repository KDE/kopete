/*
    kopeteglobal.cpp - Kopete Globals

    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>

    Kopete    (c) 2004      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteglobal.h"
#include "kopeteuiglobal.h"

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kio/netaccess.h>
#include <kmimetype.h>
#include <kmessagebox.h>
#include <kprogress.h>
#include <kstandarddirs.h>
#include <ktar.h>


namespace Kopete
{

namespace Global
{

class PropertiesPrivate
{
	public:
		ContactPropertyTmpl::Map mTemplates;
};

Properties *Properties::mSelf = 0L;

Properties *Properties::self()
{
	if(!mSelf)
	{
		//kdDebug(14000) << k_funcinfo << endl;
		mSelf = new Properties();
	}
	return mSelf;
}

Properties::Properties()
{
	kdDebug(14000) << k_funcinfo << endl;
	d = new PropertiesPrivate();
}

Properties::~Properties()
{
	kdDebug(14000) << k_funcinfo << endl;
	delete d;
}

const ContactPropertyTmpl &Properties::tmpl(const QString &key) const
{
	if(d->mTemplates.contains(key))
	{
		/*kdDebug(14000) << k_funcinfo <<
			"Found template for key = '" << key << "'" << endl;*/
		return d->mTemplates[key];
	}
	else
		return ContactPropertyTmpl::null;
}

bool Properties::registerTemplate(const QString &key,
	const ContactPropertyTmpl &tmpl)
{
	if(d->mTemplates.contains(key))
	{
		kdDebug(14000) << k_funcinfo <<
			"Called for EXISTING key = '" << key << "'" << endl;
		return false;
	}
	else
	{
		d->mTemplates.insert(key, tmpl);
		return true;
	}
}

void Properties::unregisterTemplate(const QString &key)
{
	kdDebug(14000) << k_funcinfo << "called for key: '" << key << "'" << endl;
	d->mTemplates.remove(key);
}

bool Properties::isRegistered(const QString &key)
{
	return d->mTemplates.contains(key);
}

const ContactPropertyTmpl &Properties::fullName() const
{
	return createProp(QString::fromLatin1("FormattedName"),
		QString::fromLatin1("Full Name"));
}

const ContactPropertyTmpl &Properties::idleTime() const
{
	return createProp(QString::fromLatin1("idleTime"),
		QString::fromLatin1("Idle Time"));
}

const ContactPropertyTmpl &Properties::onlineSince() const
{
	return createProp(QString::fromLatin1("onlineSince"),
		QString::fromLatin1("Online Since"));
}

const ContactPropertyTmpl &Properties::lastSeen() const
{
	return createProp(QString::fromLatin1("lastSeen"),
		QString::fromLatin1("Last Seen"), QString::null, true);
}

const ContactPropertyTmpl &Properties::awayMessage() const
{
	return createProp(QString::fromLatin1("awayMessage"),
		QString::fromLatin1("Away Message"));
}

const ContactPropertyTmpl &Properties::firstName() const
{
	return createProp(QString::fromLatin1("firstName"),
		QString::fromLatin1("First Name"), QString::null, true);
}

const ContactPropertyTmpl &Properties::lastName() const
{
	return createProp(QString::fromLatin1("lastName"),
		QString::fromLatin1("Last Name"), QString::null, true);
}

const ContactPropertyTmpl &Properties::privatePhone() const
{
	return createProp(QString::fromLatin1("privatePhoneNumber"),
		i18n("Private Phone"), QString::null, true);
}

const ContactPropertyTmpl &Properties::privateMobilePhone() const
{
	return createProp(QString::fromLatin1("privateMobilePhoneNumber"),
		i18n("Private Mobile Phone"), QString::null, true);
}

const ContactPropertyTmpl &Properties::workPhone() const
{
	return createProp(QString::fromLatin1("workPhoneNumber"),
		i18n("Work Phone"), QString::null, true);
}

const ContactPropertyTmpl &Properties::workMobilePhone() const
{
	return createProp(QString::fromLatin1("workMobilePhoneNumber"),
		i18n("Work Mobile Phone"), QString::null, true);
}

const ContactPropertyTmpl &Properties::emailAddress() const
{
	return createProp(QString::fromLatin1("emailAddress"),
		i18n("Email Address"), QString::fromLatin1("mail_generic"), true);
}

const ContactPropertyTmpl &Properties::createProp(const QString &key,
	const QString &label, const QString &icon, bool persistent) const
{
	/*kdDebug(14000) << k_funcinfo <<
		"key = " << key  << ", label = " << label << endl;*/

	if(!d->mTemplates.contains(key))
	{
		kdDebug(14000) << k_funcinfo <<
			"CREATING NEW ContactPropertyTmpl WITH key = " << key  <<
			", label = " << label << ", persisten = " << persistent << endl;
		new ContactPropertyTmpl(key, label, icon, persistent);
	}
	return tmpl(key);
}

const ContactPropertyTmpl::Map &Properties::templateMap() const
{
	return d->mTemplates;
}


// -----------------------------------------------------------------------------


void installEmoticonTheme(const QString &archiveName)
{
	QStringList foundThemes;
	KArchiveEntry *currentEntry = 0L;
	KArchiveDirectory* currentDir = 0L;
	KProgressDialog *progressDlg = 0L;
	KTar *archive = 0L;

	QString localThemesDir(locateLocal("data",
		QString::fromLatin1("kopete/pics/emoticons/")));

	if(localThemesDir.isEmpty())
	{
		KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget(),
			KMessageBox::Error, i18n("Could not find suitable place " \
			"to install emoticon themes into."));
		return;
	}

	progressDlg = new KProgressDialog(0 , "emoticonInstProgress",
	 	i18n("Installing Emoticon Themes..."), QString::null, true);
	progressDlg->progressBar()->setTotalSteps(foundThemes.count());
	progressDlg->show();
	kapp->processEvents();

	archive = new KTar(archiveName);
	if ( !archive->open(IO_ReadOnly) )
	{
		KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget(),
			KMessageBox::Error,
			i18n("Could not open \"%1\" for unpacking.").arg(archiveName));
		delete archive;
		delete progressDlg;
		return;
	}

	const KArchiveDirectory* rootDir = archive->directory();

	// iterate all the dirs looking for an emoticons.xml file
	QStringList entries = rootDir->entries();
	for (QStringList::Iterator it = entries.begin(); it != entries.end(); ++it)
	{
		currentEntry = const_cast<KArchiveEntry*>(rootDir->entry(*it));
		if (currentEntry->isDirectory())
		{
			currentDir = dynamic_cast<KArchiveDirectory*>( currentEntry );
			if (currentDir && (currentDir->entry(QString::fromLatin1("emoticons.xml")) != NULL))
				foundThemes.append(currentDir->name());
		}
	}

	if (foundThemes.isEmpty())
	{
		KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget(),
			KMessageBox::Error, i18n("<qt>The file \"%1\" is not a valid" \
				" emoticon theme archive.</qt>").arg(archiveName));
		archive->close();
		delete archive;
		delete progressDlg;
		return;
	}

	for (QStringList::ConstIterator it = foundThemes.begin(); it != foundThemes.end(); ++it)
	{
		progressDlg->setLabel(
			i18n("<qt>Installing <strong>%1</strong> emoticon theme</qt>")
			.arg(*it));
		progressDlg->resize(progressDlg->sizeHint());
		kapp->processEvents();

		if (progressDlg->wasCancelled())
			break;

		currentEntry = const_cast<KArchiveEntry *>(rootDir->entry(*it));
		if (currentEntry == 0)
		{
			kdDebug(14010) << k_funcinfo << "couldn't get next archive entry" << endl;
			continue;
		}

		if(currentEntry->isDirectory())
		{
			currentDir = dynamic_cast<KArchiveDirectory*>(currentEntry);
			if (currentDir == 0)
			{
				kdDebug(14010) << k_funcinfo <<
					"couldn't cast archive entry to KArchiveDirectory" << endl;
				continue;
			}
			currentDir->copyTo(localThemesDir + *it);
			progressDlg->progressBar()->advance(1);
		}
	}

	archive->close();
	delete archive;

	// check if all steps were done, if there are skipped ones then we didn't
	// succeed copying all dirs from the tarball
	if (progressDlg->progressBar()->totalSteps() !=
		progressDlg->progressBar()->progress())
	{
		KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget(),
			KMessageBox::Error,
			i18n("<qt>A problem occurred during the installation process. "
			"However, some of the emoticon themes in the archive may have been "
			"installed.</qt>"));
	}

	delete progressDlg;
}

} // END namespace Global

} // END namespace Kopete

// vim: set noet ts=4 sts=4 sw=4:
