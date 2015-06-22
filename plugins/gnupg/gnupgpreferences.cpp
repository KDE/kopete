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
#include <QtCrypto/QtCrypto>

#include <QtCrypto/QtCrypto>

#include "cryptography2preferences.h"

#include <kpluginfactory.h>

Q_DECLARE_METATYPE(QCA::KeyStoreEntry)

K_PLUGIN_FACTORY (Cryptography2PreferencesFactory, registerPlugin<Cryptography2Preferences>();)
K_EXPORT_PLUGIN(Cryptography2PreferencesFactory ("kcm_kopete_cryptography2"))

Cryptography2Preferences::Cryptography2Preferences ( QWidget *parent, const QVariantList &args )
		: KCModule ( Cryptography2PreferencesFactory::componentData(), parent, args )
{
  QCA::Initializer init;
  setButtons( Help | Apply | Default );
  QVBoxLayout *nl = new QVBoxLayout(this);
  QLabel *intro = new QLabel("This is the Cryptography2 plugin.<br>Please select your private key below:",this);
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

Cryptography2Preferences::~Cryptography2Preferences()
{

}

void Cryptography2Preferences::load()
{
    KCModule::load();
}

void Cryptography2Preferences::save()
{
    KCModule::save();
}

void Cryptography2Preferences::defaults()
{
    KCModule::defaults();
}
