#include <qlayout.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <qcheckbox.h>
#include <klineedit.h>
#include <gaduprefs.h>
#include "gadupreferences.h"

GaduPreferences::GaduPreferences( const QString& pixmap, QObject* parent )
	: ConfigModule( i18n("Gadu-Gadu Plugin"), i18n("Gadu Gadu"), pixmap, parent )
{
	uin_ = 0;
	( new QVBoxLayout( this ) )->setAutoAdd( true );
	prefDialog_ = new gaduPrefsUI( this );

	KGlobal::config()->setGroup("Gadu");
	prefDialog_->nicknameEdit_->setText( KGlobal::config()->readEntry("Nick", i18n("Your Gadu-Gadu nickname here")) );
	prefDialog_->logAll_->setChecked( KGlobal::config()->readBoolEntry( "LogAll", false ) );
}

GaduPreferences::~GaduPreferences()
{
}

void
GaduPreferences::save()
{
	KConfig *config=KGlobal::config();


	config->setGroup("Gadu");
	config->writeEntry("Nick", prefDialog_->nicknameEdit_->text());
	config->writeEntry("LogAll", prefDialog_->logAll_->isChecked());
	config->sync();

	emit saved();
}

#include "gadupreferences.moc"
