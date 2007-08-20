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

#include <QtCore/QLatin1String>
#include <QtGui/QApplication>

#include <kdebug.h>
#include <klocale.h>
#include <kio/netaccess.h>
#include <kmimetype.h>
#include <kmessagebox.h>
#include <kprogressdialog.h>

#include <kstandarddirs.h>
#include <ktar.h>
#include <kzip.h>
#include <kmimetype.h>


namespace Kopete
{

namespace Global
{

class PropertiesPrivate
{
	public:
		PropertyTmpl::Map mTemplates;
};

Properties *Properties::mSelf = 0L;

Properties *Properties::self()
{
	if(!mSelf)
	{
		//kDebug(14000) ;
		mSelf = new Properties();
		// create the templates
		mSelf->fullName();
		mSelf->idleTime();
		mSelf->onlineSince();
		mSelf->lastSeen();
		mSelf->statusMessage();
		mSelf->firstName();
		mSelf->lastName();
		mSelf->emailAddress();
		mSelf->privatePhone();
		mSelf->privateMobilePhone();
		mSelf->workPhone();
		mSelf->workMobilePhone();
		mSelf->nickName();
		mSelf->photo();

	}
	return mSelf;
}

Properties::Properties()
{
	kDebug(14000) ;
	d = new PropertiesPrivate();
}

Properties::~Properties()
{
	kDebug(14000) ;
	delete d;
}

const PropertyTmpl &Properties::tmpl(const QString &key) const
{
	if(d->mTemplates.contains(key))
	{
		/*kDebug(14000) <<
			"Found template for key = '" << key << "'" << endl;*/
		return d->mTemplates[key];
	}
	else
		return PropertyTmpl::null;
}

bool Properties::registerTemplate(const QString &key,
	const PropertyTmpl &tmpl)
{
	if(d->mTemplates.contains(key))
	{
		kDebug(14000) <<
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
	kDebug(14000) << "called for key: '" << key << "'";
	d->mTemplates.remove(key);
}

bool Properties::isRegistered(const QString &key)
{
	return d->mTemplates.contains(key);
}

const PropertyTmpl &Properties::fullName() const
{
	return createProp(QLatin1String("FormattedName"),
		i18n("Full Name"));
}

const PropertyTmpl &Properties::idleTime() const
{
	return createProp(QLatin1String("idleTime"),
		i18n("Idle Time"));
}

const PropertyTmpl &Properties::onlineSince() const
{
	return createProp(QLatin1String("onlineSince"),
		i18n("Online Since"));
}

const PropertyTmpl &Properties::lastSeen() const
{
	return createProp(QLatin1String("lastSeen"),
		i18n("Last Seen"), QString(), true);
}

const PropertyTmpl &Properties::statusMessage() const
{
	return createProp(QLatin1String("statusMessage"),
		i18n("Status Message"));
}

const PropertyTmpl &Properties::firstName() const
{
	return createProp(QLatin1String("firstName"),
		i18n("First Name"), QString(), true);
}

const PropertyTmpl &Properties::lastName() const
{
	return createProp(QLatin1String("lastName"),
		i18n("Last Name"), QString(), true);
}

const PropertyTmpl &Properties::privatePhone() const
{
	return createProp(QLatin1String("privatePhoneNumber"),
		i18n("Private Phone"), QString(), true);
}

const PropertyTmpl &Properties::privateMobilePhone() const
{
	return createProp(QLatin1String("privateMobilePhoneNumber"),
		i18n("Private Mobile Phone"), QString(), true);
}

const PropertyTmpl &Properties::workPhone() const
{
	return createProp(QLatin1String("workPhoneNumber"),
		i18n("Work Phone"), QString(), true);
}

const PropertyTmpl &Properties::workMobilePhone() const
{
	return createProp(QLatin1String("workMobilePhoneNumber"),
		i18n("Work Mobile Phone"), QString(), true);
}

const PropertyTmpl &Properties::emailAddress() const
{
	return createProp(QLatin1String("emailAddress"),
		i18n("Email Address"), QLatin1String("mail"), true);
}

const PropertyTmpl &Properties::nickName() const
{
	return createProp(QLatin1String("nickName"),
		i18n("Nick Name"), QString(), true);
}

const PropertyTmpl &Properties::photo() const
{
	return createProp(QLatin1String("photo"),
					  i18n("Photo"), QString(), true);
}


const PropertyTmpl &Properties::createProp(const QString &key,
	const QString &label, const QString &icon, bool persistent) const
{
	/*kDebug(14000) <<
		"key = " << key  << ", label = " << label << endl;*/

	if(!d->mTemplates.contains(key))
	{
/*		kDebug(14000) <<
			"CREATING NEW PropertyTmpl WITH key = " << key  <<
			", label = " << label << ", persisten = " << persistent << endl;*/
		d->mTemplates.insert(key,  PropertyTmpl(key, label, icon, persistent ? PropertyTmpl::PersistentProperty : PropertyTmpl::NoProperty));
	}
	return tmpl(key);
}

const PropertyTmpl::Map &Properties::templateMap() const
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
	KArchive *archive = 0L;

	QString localThemesDir(KStandardDirs::locateLocal("emoticons", QString()) );

	if(localThemesDir.isEmpty())
	{
		KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget(),
			KMessageBox::Error, i18n("Could not find suitable place " \
			"to install emoticon themes into."));
		return;
	}

	progressDlg = new KProgressDialog(0,
	 	i18n("Installing Emoticon Themes..."));
	progressDlg->setModal(true);
	progressDlg->progressBar()->setMaximum(foundThemes.count());
	progressDlg->show();
	qApp->processEvents();

	QString currentBundleMimeType = KMimeType::findByPath(archiveName, 0, false)->name();
	if( currentBundleMimeType == QLatin1String("application/zip") ||
        currentBundleMimeType == QLatin1String("application/x-zip") ||
        currentBundleMimeType ==  QLatin1String("application/x-zip-compressed"))
		archive = new KZip(archiveName);
	else if( currentBundleMimeType == QLatin1String("application/x-compressed-tar") || 
				currentBundleMimeType == QLatin1String("application/x-bzip-compressed-tar") ||
				currentBundleMimeType == QLatin1String("application/x-gzip") ||
				currentBundleMimeType == QLatin1String("application/x-bzip") )
		archive = new KTar(archiveName);
	else if(archiveName.endsWith(QLatin1String("jisp")) || archiveName.endsWith(QLatin1String("zip")) )
		archive = new KZip(archiveName);
	else
		archive = new KTar(archiveName);

	if ( !archive || !archive->open(QIODevice::ReadOnly) )
	{
		KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget(),
			KMessageBox::Error,
			i18n("Could not open \"%1\" for unpacking.", archiveName));
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
			if (currentDir && ( currentDir->entry(QLatin1String("emoticons.xml")) != NULL ||
						 		currentDir->entry(QLatin1String("icondef.xml")) != NULL ) )
				foundThemes.append(currentDir->name());
		}
	}

	if (foundThemes.isEmpty())
	{
		KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget(),
			KMessageBox::Error, i18n("<qt>The file \"%1\" is not a valid" \
				" emoticon theme archive.</qt>", archiveName));
		archive->close();
		delete archive;
		delete progressDlg;
		return;
	}

	for (int themeIndex = 0; themeIndex < foundThemes.size(); ++themeIndex)
	{
		const QString &theme = foundThemes[themeIndex];

		progressDlg->setLabelText(
			i18n("<qt>Installing <strong>%1</strong> emoticon theme</qt>",
			theme));
		progressDlg->progressBar()->setValue(themeIndex);
		progressDlg->resize(progressDlg->sizeHint());
		qApp->processEvents();

		if (progressDlg->wasCancelled())
			break;

		currentEntry = const_cast<KArchiveEntry *>(rootDir->entry(theme));
		if (currentEntry == 0)
		{
			kDebug(14010) << "couldn't get next archive entry";
			continue;
		}

		if(currentEntry->isDirectory())
		{
			currentDir = dynamic_cast<KArchiveDirectory*>(currentEntry);
			if (currentDir == 0)
			{
				kDebug(14010) <<
					"couldn't cast archive entry to KArchiveDirectory" << endl;
				continue;
			}
			currentDir->copyTo(localThemesDir + theme);
		}
	}

	archive->close();
	delete archive;

	// check if all steps were done, if there are skipped ones then we didn't
	// succeed copying all dirs from the tarball
	if (progressDlg->progressBar()->maximum() > progressDlg->progressBar()->value())
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
