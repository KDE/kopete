#include <QtCore>
#include <QPushButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVariantList>

#include "cryptography2preferences.h"

#include <kpluginfactory.h>

K_PLUGIN_FACTORY (Cryptography2PreferencesFactory, registerPlugin<Cryptography2Preferences>();)
K_EXPORT_PLUGIN(Cryptography2PreferencesFactory ("kcm_kopete_cryptography2"))

Cryptography2Preferences::Cryptography2Preferences ( QWidget *parent, const QVariantList &args )
		: KCModule ( Cryptography2PreferencesFactory::componentData(), parent, args )
{
  setButtons( Help | Apply | Default );
  QVBoxLayout *nl = new QVBoxLayout(this);
  QLabel *testme = new QLabel("Dokimastiko label",this);
  nl->addLayout(nl);
  nl->addWidget(testme);
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
