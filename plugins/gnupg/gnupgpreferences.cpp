//=====QT Stuff here=====//
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
#include <QListView>
#include <QTableView>
#include <QPushButton>
//=======================//


//=====Kopete Stuff here=====//
#include <kopeteaccountmanager.h>
#include <kopeteaccount.h>
#include <kopeteprotocol.h>
#include <QStandardItemModel>
#include <QList>
//===========================//

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
    QVBoxLayout *globalLayout = new QVBoxLayout(this);
    QHBoxLayout *introLayout = new QHBoxLayout(this);
    QHBoxLayout *addbuttonLayout = new QHBoxLayout(this);
    QHBoxLayout *removebuttonLayout = new QHBoxLayout(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    QHBoxLayout *accountsLayout = new QHBoxLayout(this);
    QHBoxLayout *keysLayout = new QHBoxLayout(this);
    QVBoxLayout *resultsLayout = new QVBoxLayout(this);
    resultsLayout->setAlignment(Qt::AlignCenter);
    QListView *accountsList = new QListView(this);
    QListView *keysList = new QListView(this);
    resultsTable = new QTableView(this);
    QPushButton *addCombination = new QPushButton("Add Combination",this);
    QPushButton *removeCombination = new QPushButton("Remove Pair",this);
    connect(addCombination,SIGNAL(clicked()),this,SLOT(addPair()));
    connect(removeCombination,SIGNAL(clicked()),this,SLOT(remPair()));
    QLabel *resultsInfo = new QLabel("Account-Key PGP pair",this);
    QLabel *intro = new QLabel("This is the GnuPG plugin.<br>Please select your private key below:",this);
    QList<Kopete::Account*> accountList = Kopete::AccountManager::self()->accounts();
    QStandardItemModel *accountsModel = new QStandardItemModel(this);
    accountsList->setModel(accountsModel);
    if(accountList.length()==0)
    {
        QStandardItem *accountItem = new QStandardItem();
        accountItem->setData("<no account>",Qt::DisplayRole);
        accountItem->setEditable(false);
        accountsModel->appendRow(accountItem);
    }
    else
    {
        foreach( Kopete::Account *account, accountList )
        {
            QStandardItem *accountItem = new QStandardItem();
            accountItem->setData(account->accountLabel(),Qt::DisplayRole);
            accountItem->setEditable(false);
            accountsModel->appendRow(accountItem);
        }
    }

    QCA::KeyStoreManager::start();
    QCA::KeyStoreManager sman(this);
    sman.waitForBusyFinished();
    QCA::KeyStore pgpks(QString("qca-gnupg"), &sman);
    QStandardItemModel *keysModel = new QStandardItemModel(this);
    keysList->setModel(keysModel);
    if(pgpks.entryList().length() == 0 )
    {
        QStandardItem *keyItem = new QStandardItem();
        keyItem->setData("<no pgp keys>",Qt::DisplayRole);
        keyItem->setEditable(false);
        keysModel->appendRow(keyItem);
    }
    else
    {
        foreach(const QCA::KeyStoreEntry kse, pgpks.entryList())
        {
            QString text = kse.name()+" "+kse.id();
            QVariant v;
            v.setValue(kse);
            if(!kse.pgpSecretKey().isNull())
            {
                QStandardItem *keyItem = new QStandardItem();
                keyItem->setData(text,Qt::DisplayRole);
                keyItem->setEditable(false);
                keysModel->appendRow(keyItem);
            }
        }
    }
    accountsLayout->addWidget(accountsList);
    keysLayout->addWidget(keysList);
    introLayout->addWidget(intro);
    addbuttonLayout->addWidget(addCombination);
    removebuttonLayout->addWidget(removeCombination);
    resultsLayout->addWidget(resultsInfo);
    resultsLayout->addWidget(resultsTable);
    mainLayout->addLayout(accountsLayout);
    mainLayout->addLayout(keysLayout);
    globalLayout->addLayout(introLayout);
    globalLayout->addLayout(mainLayout);
    globalLayout->addLayout(addbuttonLayout);
    globalLayout->addLayout(resultsLayout);
    globalLayout->addLayout(removebuttonLayout);
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

void GnupgPreferences::addPair()
{

}

void GnupgPreferences::remPair()
{

}

GnupgPreferences::~GnupgPreferences()
{

}