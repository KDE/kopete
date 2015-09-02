#include "gnupgselectuserkey.h"
#include "kopetemetacontact.h"
#include <kiconloader.h>
#include <QFileDialog>
#include <klocalizedstring.h>
#include <klineedit.h>
#include <QLabel>
#include <QBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <kdialog.h>
#include <QTextEdit>
#include <QPushButton>
#include <kdebug.h>
#include <qca2/QtCrypto/qca.h>

GnupgSelectUserKey::GnupgSelectUserKey(Kopete::MetaContact *mc): KDialog()
{
    QCA::Initializer init;
    QCA::KeyStoreManager::start();
    m_metaContact = mc;
    setCaption(mc->displayName());
    setButtons(KDialog::Ok | KDialog::Cancel);
    setDefaultButton(KDialog::Ok);
    QWidget *w = new QWidget(this);
    setMainWidget(w);
    pathKey = new QLineEdit();
    QPushButton *loadKey = new QPushButton("Select public key.");
    connect(loadKey,SIGNAL(clicked()),this,SLOT(loadFile()));
    connect(this,SIGNAL(okClicked()),this,SLOT(save()));
    QVBoxLayout * l = new QVBoxLayout ( w );
    l->addWidget(loadKey);
    l->addWidget(pathKey);
}

void GnupgSelectUserKey::loadFile()
{
    QString keyPath = QFileDialog::getOpenFileName(this, "Open PGP File 1", "/home/nikhatzi", "PGP Files (*.asc)");
    pathKey->setText(keyPath);
}

void GnupgSelectUserKey::save()
{
    QCA::KeyStoreManager sman(this);
    sman.waitForBusyFinished();
    QCA::KeyStore pgpks(QString("qca-gnupg"), &sman);
    QCA::PGPKey pubKey = QCA::PGPKey::fromFile(pathKey->text());
    if(!pgpks.isValid())
    {
      kDebug( 14303 ) << "keystore NOT valid" << endl;
      kDebug( 14303 ) << "KEYSTORE LENGTH: " << pgpks.entryList().length() << endl;
    }
    if(!pubKey.isNull())
    {
      if(!pgpks.writeEntry(pubKey).isEmpty())
      {
	kDebug( 14303 ) << "Key written" << endl;
      }
      else
      {
	kDebug (14303 ) << "Key problem, not written" << endl;
      }
      kDebug ( 14303 ) << "1337 " << pubKey.toString();
      kDebug ( 14303 ) << "1337 " << pubKey.creationDate();
      kDebug ( 14303 ) << "1337 " << pubKey.inKeyring();
    }
    else
    {
      kDebug ( 14303 ) << "Key not supported.";
    }
}

GnupgSelectUserKey::~GnupgSelectUserKey()
{

}

#include "gnupgselectuserkey.moc"
