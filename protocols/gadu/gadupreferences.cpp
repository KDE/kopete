#include "gaduprefs.h"
#include "gadupreferences.h"
#include "gaducommands.h"

#include <kmessagebox.h>
#include <klineedit.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include <qpushbutton.h>
#include <qlayout.h>
#include <qcheckbox.h>

GaduPreferences::GaduPreferences( const QString& pixmap, QObject* parent )
	: ConfigModule( i18n("Gadu-Gadu Plugin"), i18n("Gadu Gadu"), pixmap, parent )
{
	uin_ = 0;
	( new QVBoxLayout( this ) )->setAutoAdd( true );
	prefDialog_ = new gaduPrefsUI( this );

	KGlobal::config()->setGroup("Gadu");
	prefDialog_->nicknameEdit_->setText( KGlobal::config()->readEntry("Nick", i18n("Your Gadu-Gadu nickname here")) );
	prefDialog_->logAll_->setChecked( KGlobal::config()->readBoolEntry( "LogAll", false ) );
  connect( prefDialog_->registerButton_, SLOT(clicked()),this, SIGNAL(slotRegister()) );
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

void
GaduPreferences::slotRegister()
{
  QString email =  prefDialog_->emailEdit_->text();
  QString pass  =  prefDialog_->passEdit_->text();

  if ( email.isEmpty() || pass.isEmpty() || !email.contains( '@' ) ) {
    KMessageBox::information( this, i18n("Incorrect data"), i18n("All fields must be filled in.") );
  }
  RegisterCommand *cmd = new RegisterCommand( email, pass, this );
  connect( cmd, SIGNAL(done(const QString&, const QString&)),
           SLOT(registrationComplete(const QString&, const QString&)) );
  connect( cmd, SIGNAL(error(const QString&, const QString&)),
           SLOT(registrationError(const QString&, const QString&)) );
  cmd->execute();
}

void
GaduPreferences::registrationComplete( const QString& title, const QString& what )
{
  KMessageBox::information( this, title, what );
}

void
GaduPreferences::registrationError( const QString& title, const QString& what )
{
  KMessageBox::information( this, title, what );
}

#include "gadupreferences.moc"
