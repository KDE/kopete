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
#include <QApplication>

#include <kdebug.h>
#include <KLocalizedString>
#include <kio/netaccess.h>
#include <kmimetype.h>
#include <kmessagebox.h>
#include <kprogressdialog.h>


#include <KArchive/ktar.h>
#include <KArchive/kzip.h>

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
		mSelf->customName();
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
	mSelf = 0L;
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
	return createProp(QStringLiteral("FormattedName"),
		i18n("Full Name"));
}

const PropertyTmpl &Properties::idleTime() const
{
	return createProp(QStringLiteral("idleTime"),
		i18n("Idle Time"));
}

const PropertyTmpl &Properties::onlineSince() const
{
	return createProp(QStringLiteral("onlineSince"),
		i18n("Online Since"));
}

const PropertyTmpl &Properties::lastSeen() const
{
	return createProp(QStringLiteral("lastSeen"),
		i18n("Last Seen"), QString(), true);
}

const PropertyTmpl &Properties::statusTitle() const
{
	return createProp(QStringLiteral("statusTitle"),
	                  i18n("Status Title"));
}

const PropertyTmpl &Properties::statusMessage() const
{
	return createProp(QStringLiteral("statusMessage"),
		i18n("Status Message"));
}

const PropertyTmpl &Properties::firstName() const
{
	return createProp(QStringLiteral("firstName"),
		i18n("First Name"), QString(), true);
}

const PropertyTmpl &Properties::lastName() const
{
	return createProp(QStringLiteral("lastName"),
		i18n("Last Name"), QString(), true);
}

const PropertyTmpl &Properties::privatePhone() const
{
	return createProp(QStringLiteral("privatePhoneNumber"),
		i18n("Private Phone"), QString(), true);
}

const PropertyTmpl &Properties::privateMobilePhone() const
{
	return createProp(QStringLiteral("privateMobilePhoneNumber"),
		i18n("Private Mobile Phone"), QString(), true);
}

const PropertyTmpl &Properties::workPhone() const
{
	return createProp(QStringLiteral("workPhoneNumber"),
		i18n("Work Phone"), QString(), true);
}

const PropertyTmpl &Properties::workMobilePhone() const
{
	return createProp(QStringLiteral("workMobilePhoneNumber"),
		i18n("Work Mobile Phone"), QString(), true);
}

const PropertyTmpl &Properties::emailAddress() const
{
	return createProp(QStringLiteral("emailAddress"),
		i18n("Email Address"), QStringLiteral("mail"), true);
}

const PropertyTmpl &Properties::nickName() const
{
	return createProp(QStringLiteral("nickName"),
		i18n("Nick Name"), QString(), true);
}

const PropertyTmpl &Properties::customName() const
{
	return createProp(QStringLiteral("customName"),
		i18n("Custom Name"), QString(), true);
}

const PropertyTmpl &Properties::isAlwaysVisible() const
{
	return createProp(QStringLiteral("isAlwaysVisible"),
		i18n("Shown even if offline"), QString(), true);
}

const PropertyTmpl &Properties::photo() const
{
	return createProp(QStringLiteral("photo"),
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

} // END namespace Global

} // END namespace Kopete

// vim: set noet ts=4 sts=4 sw=4:
