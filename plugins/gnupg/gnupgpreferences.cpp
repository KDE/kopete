#include <QtCore>
#include <QPushButton>
#include <QDebug>
#include <QComboBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QString>
#include <QVariantList>

#include <qca2/QtCrypto/QtCrypto>

#include "gnupgpreferences.h"

#include <kpluginfactory.h>

Q_DECLARE_METATYPE(QCA::KeyStoreEntry)

K_PLUGIN_FACTORY (GnupgPreferencesFactory, registerPlugin<GnupgPreferences>();)
K_EXPORT_PLUGIN(GnupgPreferencesFactory ("kcm_kopete_gnupg"))

GnupgPreferences::GnupgPreferences(QWidget* parent, const QVariantList& args)
: KCModule ( GnupgPreferencesFactory::componentData(), parent, args )
{
  QCA::Initializer init;
  setButtons( Help | Apply | Default );
  QVBoxLayout *nl = new QVBoxLayout(this);
  QLabel *intro = new QLabel("This is the GnuPG plugin.<br>Please select your private key below:",this);
  QComboBox *keysList = new QComboBox(this);
  nl->addLayout(nl);
  nl->addWidget(intro);
  nl->addWidget(keysList);
  QCA::KeyStoreManager::start();
  QCA::KeyStoreManager sman(this);
  sman.waitForBusyFinished();
  QCA::KeyStore pgpks(QString("qca-gnupg"), &sman);
  foreach(const QCA::KeyStoreEntry kse, pgpks.entryList())
  {
    QString text = kse.name()+" "+kse.id();
    QVariant v;
    v.setValue(kse);
    if(!kse.pgpSecretKey().isNull())
    {
      keysList->addItem(text,v);
    }
  }
  load();
}

void GnupgPreferences::defaults()
{
    KCModule::defaults();
}

void GnupgPreferences::load()
{
    KCModule::load();
}

void GnupgPreferences::save()
{
    KCModule::save();
}

GnupgPreferences::~GnupgPreferences()
{

}