/*
    translatorprefs.cpp

    Kopete Translator plugin

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett   <duncan@kde.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/


#include "translatorprefs.moc"
#include "translatorprefsbase.h"
#include "translatorplugin.h"

#include <qlayout.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <klineedit.h>
#include <kdebug.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qmap.h>

#include <knuminput.h>

TranslatorPreferences::TranslatorPreferences(const QString &pixmap,QObject *parent)
							: ConfigModule(i18n("Translator"),i18n("Translator Plugin"),pixmap,parent)
{
	( new QVBoxLayout( this ) )->setAutoAdd( true );

	preferencesDialog = new TranslatorPrefsUI(this);

    QMap<QString,QString>::ConstIterator i;
	QMap<QString,QString> m;

	m = TranslatorPlugin::plugin()->languagesMap();

	for ( i = m.begin(); i != m.end() ; ++i )
		preferencesDialog->m_LangBox->insertItem( i.data(), TranslatorPlugin::plugin()->languageIndex(i.key()) );

	m = TranslatorPlugin::plugin()->servicesMap();

	for ( i = m.begin(); i != m.end() ; ++i )
		preferencesDialog->m_ServiceBox->insertItem( i.data(), TranslatorPlugin::plugin()->serviceIndex(i.key()) );

	reopen();

}

TranslatorPreferences::~TranslatorPreferences()
{
}


const QString& TranslatorPreferences::myLang()
{
	return TranslatorPlugin::plugin()->languageKey(preferencesDialog->m_LangBox->currentItem());
}

const QString& TranslatorPreferences::service()
{
	return TranslatorPlugin::plugin()->serviceKey(preferencesDialog->m_ServiceBox->currentItem());
}

const unsigned int TranslatorPreferences::outgoingMode()
{
	return preferencesDialog->m_outgoing->id(preferencesDialog->m_outgoing->selected() ) ;
}

const unsigned int TranslatorPreferences::incommingMode()
{
	return preferencesDialog->m_incomming->id(preferencesDialog->m_incomming->selected() );
}


void TranslatorPreferences::reopen()
{
	KGlobal::config()->setGroup("Translator Plugin");

	preferencesDialog->m_LangBox->setCurrentItem( TranslatorPlugin::plugin()->languageIndex(KGlobal::config()->readEntry("myLang", "null")));
	preferencesDialog->m_ServiceBox->setCurrentItem( TranslatorPlugin::plugin()->serviceIndex(KGlobal::config()->readEntry("Service", "babelfish")));

	preferencesDialog->m_incomming->setButton(KGlobal::config()->readNumEntry( "Incomming Mode", TranslatorPlugin::ShowOriginal) );
	preferencesDialog->m_outgoing->setButton(KGlobal::config()->readNumEntry( "Outgoing Mode", TranslatorPlugin::ShowOriginal) );
}

void TranslatorPreferences::save()
{
	KConfig *config = KGlobal::config();
	config->setGroup("Translator Plugin");
	config->writeEntry("myLang", myLang() );
	config->writeEntry("Service", service() );

	config->writeEntry("Incomming Mode", preferencesDialog->m_incomming->id(preferencesDialog->m_incomming->selected() ) );
	config->writeEntry("Outgoing Mode",  preferencesDialog->m_outgoing->id(preferencesDialog->m_outgoing->selected() ) );


	config->sync();
	emit saved();
}

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

