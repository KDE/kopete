#include <qlayout.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <qlineedit.h>
#include <kdebug.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <knuminput.h>
#include <klineedit.h>
#include <gaduprefs.h>
#include "gadupreferences.h"

GaduPreferences::GaduPreferences( const QString& pixmap, QObject* parent )
    : ConfigModule( i18n("Gadu Plugin"), i18n("Gadu Gadu"), pixmap, parent )
{
    uin_ = 0;
    ( new QVBoxLayout( this ) )->setAutoAdd( true );
    prefDialog_ = new gaduPrefsUI( this );

    KGlobal::config()->setGroup("Gadu");
    prefDialog_->uinEdit_->setText( KGlobal::config()->readEntry("Uin", "Your Gadu-Gadu uin here") );
    prefDialog_->passwordEdit_->setText( KGlobal::config()->readEntry("Password", "") );
    prefDialog_->nicknameEdit_->setText( KGlobal::config()->readEntry("Nick", "Your Gadu-Gadu nickname here") );
    prefDialog_->autoConnect_->setChecked( KGlobal::config()->readBoolEntry( "AutoConnect", false ) );
    prefDialog_->logAll_->setChecked( KGlobal::config()->readBoolEntry( "LogAll", false ) );
}

GaduPreferences::~GaduPreferences()
{
}

void
GaduPreferences::save()
{
    KConfig *config=KGlobal::config();

    password_ = prefDialog_->passwordEdit_->text();
    uin_ = prefDialog_->uinEdit_->text().toUInt();

    config->setGroup("Gadu");
    config->writeEntry("Uin", prefDialog_->uinEdit_->text());
    config->writeEntry("Password", prefDialog_->passwordEdit_->text());
    config->writeEntry("Nick", prefDialog_->nicknameEdit_->text());
    config->writeEntry("AutoConnect", prefDialog_->autoConnect_->isChecked() );
    config->writeEntry("LogAll", prefDialog_->logAll_->isChecked());
    config->sync();

    uin_ = prefDialog_->uinEdit_->text().toUInt();
    password_ = prefDialog_->passwordEdit_->text();

    emit saved();
}

#include "gadupreferences.moc"

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 *
 * vim: set et ts=4 sts=4 sw=4:
 */
