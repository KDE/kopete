//=====QT Stuff=====//
#include <QtCore>
#include <QDebug>
#include <QComboBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QString>
#include <QItemSelectionModel>
#include <QStringList>
#include <QList>
#include <QMessageBox>
#include <QTest>
//=======================//


//=====Kopete Stuff=====//
#include <kopeteaccountmanager.h>
#include <kopeteaccount.h>
#include <kopeteprotocol.h>
//===========================//

//=======KDE Stuff=======//
#include <kconfig.h>
#include <kconfiggroup.h>
//=======================//

//=======Other Stuff=======//
#include <QtCrypto>
#include "gnupgpreferences.h"
#include <kpluginfactory.h>
#include <gnupgsettings.h>
//=========================//

Q_DECLARE_METATYPE(QCA::KeyStoreEntry)

K_PLUGIN_FACTORY (GnupgPreferencesFactory, registerPlugin<GnupgPreferences>();)
K_EXPORT_PLUGIN(GnupgPreferencesFactory ("kcm_kopete_gnupg"))

GnupgPreferences::GnupgPreferences(QWidget* parent, const QVariantList& args)
    : KCModule ( GnupgPreferencesFactory::componentData(), parent, args )
{
    QCA::Initializer init;
    QCA::KeyStoreManager::start();
    QCA::KeyStoreManager sman(this);
    sman.waitForBusyFinished();
    QCA::KeyStore pgpks(QString("qca-gnupg"), &sman);
    setButtons( KCModule::Help | KCModule::Apply | KCModule::Default );
    QVBoxLayout *globalLayout = new QVBoxLayout(this);
    QHBoxLayout *introLayout = new QHBoxLayout(0);
    QHBoxLayout *addbuttonLayout = new QHBoxLayout(0);
    QHBoxLayout *removebuttonLayout = new QHBoxLayout(0);
    QHBoxLayout *mainLayout = new QHBoxLayout(0);
    QHBoxLayout *accountsLayout = new QHBoxLayout(0);
    QHBoxLayout *keysLayout = new QHBoxLayout(0);
    QVBoxLayout *resultsLayout = new QVBoxLayout(0);
    resultsLayout->setAlignment(Qt::AlignCenter);
    accountsList = new QListView(this);
    keysList = new QListView(this);
    resultsTable = new QTableView(this);
    resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    addCombination = new QPushButton("Add Pair",this);
    remCombination = new QPushButton("Remove Pair",this);
    connect(addCombination,SIGNAL(clicked()),this,SLOT(addPair()));
    connect(remCombination,SIGNAL(clicked()),this,SLOT(remPair()));
    QLabel *resultsInfo = new QLabel("Account-Key PGP pair",this);
    QLabel *intro = new QLabel("This is the GnuPG plugin.<br>Please select your private key below:",this);
    QList<Kopete::Account*> accountList = Kopete::AccountManager::self()->accounts();
    accountsModel = new QStandardItemModel(this);
    resultsModel = new QStandardItemModel(this);
    QStringList headersList;
    headersList << "Account" << "Private Key";
    resultsModel->setHorizontalHeaderLabels(headersList);
    resultsTable->setModel(resultsModel);
    resultsTable->resizeColumnsToContents();
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
    keysModel = new QStandardItemModel(this);
    keysList->setModel(keysModel);
    kDebug ( 14303 ) << QCA::KeyStoreManager::diagnosticText();
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
    removebuttonLayout->addWidget(remCombination);
    resultsLayout->addWidget(resultsInfo);
    resultsLayout->addWidget(resultsTable);
    mainLayout->addLayout(accountsLayout);
    mainLayout->addLayout(keysLayout);
    globalLayout->addLayout(introLayout);
    globalLayout->addLayout(mainLayout);
    globalLayout->addLayout(addbuttonLayout);
    globalLayout->addLayout(resultsLayout);
    globalLayout->addLayout(removebuttonLayout);
    addCombination->setEnabled(true);
    remCombination->setEnabled(true);
    buttonsStatus();
}

void GnupgPreferences::addPair()
{
    if(accountsList->currentIndex().data().toString()=="" || accountsList->currentIndex().data().toString() == "<no account>" || keysList->currentIndex().data().toString()=="" || keysList->currentIndex().data().toString() == "<no pgp keys>")
    {
        QMessageBox::information(this,"Kopete GnuPG","Please select both account and key.");
    }
    else
    {
        QString account = accountsList->currentIndex().data().toString();
        QString key = keysList->currentIndex().data().toString();
        QStandardItem *pairItem = new QStandardItem();
        QStandardItem *pairItem2 = new QStandardItem();
        pairItem2->setData(key,Qt::DisplayRole);
        pairItem2->setEditable(false);
        pairItem->setData(account,Qt::DisplayRole);
        pairItem->setEditable(false);
        QList<QStandardItem *> myList;
        myList << pairItem << pairItem2;
        resultsModel->appendRow(myList);
        int index = accountsList->currentIndex().row();
        accountsModel->removeRow(index);
        accountsList->clearSelection();
    }
    emit KCModule::changed(true);
    buttonsStatus();
}

void GnupgPreferences::remPair()
{
    int index = resultsTable->currentIndex().row();
    if(index<0)
    {
        QMessageBox::information(this,"Kopete GnuPG","Please select a pair to delete.");
    }
    else
    {
        //QString temp = resultsTable->currentIndex().data().toString();
        QString temp = resultsTable->model()->data(resultsTable->model()->index(index,0)).toString();
        resultsModel->removeRow(index);
        //qDebug() << "Removed INDEX: " << index << endl;
        QStandardItem *accountItem = new QStandardItem();
        accountItem->setData(temp,Qt::DisplayRole);
        accountItem->setEditable(false);
        accountsModel->appendRow(accountItem);
    }
    buttonsStatus();
}

void GnupgPreferences::buttonsStatus()
{
    if(accountsModel->rowCount() == 0 || keysModel->rowCount()==0)
        addCombination->setEnabled(false);
    else
        addCombination->setEnabled(true);
    if(resultsModel->rowCount() == 0)
        remCombination->setEnabled(false);
    else
        remCombination->setEnabled(true);
}

GnupgPreferences::~GnupgPreferences()
{

}

void GnupgPreferences::defaults()
{
    kDebug ( 14303 ) << "DEFAULTS";
    KCModule::defaults();
}

void GnupgPreferences::load()
{
    kDebug ( 14303 ) << "LOAD";
    KCModule::load();
}

void GnupgPreferences::save()
{
    kDebug ( 14303 ) <<"SAVE";
    KConfig config("/home/nikhatzi/testrc"); //test file
    KConfigGroup firstGroup(&config,"General");
    
    for(int i = 0;i<10;i++)
    {
      KConfigGroup keysGroup = config.group(QString::number(i));
      keysGroup.writeEntry("Test1","xixixi");
      keysGroup.writeEntry("Test2","xouxouxou");
    }
    firstGroup.config()->sync();
    //KCModule::save();
}
