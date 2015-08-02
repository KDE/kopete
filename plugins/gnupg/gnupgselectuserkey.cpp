#include "gnupgselectuserkey.h"
#include "kopetemetacontact.h"
#include <kiconloader.h>
#include <klocalizedstring.h>
#include <klineedit.h>
#include <QLabel>
#include <QBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <kabc/addressbook.h>
#include <kabc/addressee.h>
#include <kdialog.h>

GnupgSelectUserKey::GnupgSelectUserKey(const QString &key, Kopete::MetaContact *mc): KDialog()
{
  setCaption ( i18n ( "1337" ) );
  setButtons ( KDialog::Ok | KDialog::Cancel );
  setDefaultButton ( KDialog::Ok );
  m_metaContact = mc;
  QWidget *w = new QWidget(this);
  QLabel *label = new QLabel(w);
  setMainWidget ( w );
  label->setText(mc->displayName());  
  QVBoxLayout * l = new QVBoxLayout ( w );
  l->addWidget ( label );
}

GnupgSelectUserKey::~GnupgSelectUserKey()
{
  
}

#include "gnupgselectuserkey.moc"