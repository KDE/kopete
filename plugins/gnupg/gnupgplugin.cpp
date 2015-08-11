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
#include <QVariantList>
//=======================//


//=====Kopete Stuff=====//
#include <kopeteaccountmanager.h>
#include <kopeteaccount.h>
#include <kopeteprotocol.h>
#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include "kopeteprotocol.h"
#include "kopeteuiglobal.h"
#include "kopetechatsessionmanager.h"
//===========================//

//=======KDE Stuff=======//
#include <kapplication.h>
#include <kconfig.h>
#include <kpluginfactory.h>
#include <kconfiggroup.h>
#include <kgenericfactory.h>
#include <kmessagebox.h>
#include <kactioncollection.h>
//=======================//

//=======Other Stuff=======//
#include <qca2/QtCrypto/QtCrypto>
#include "gnupgpreferences.h"
#include "gnupgplugin.h"
//=========================//

GnupgPlugin* GnupgPlugin::mPluginStatic = 0L;

K_PLUGIN_FACTORY ( GnupgPluginFactory, registerPlugin<GnupgPlugin>(); )
K_EXPORT_PLUGIN ( GnupgPluginFactory ( "kopete_gnupg" ) )

GnupgPlugin::GnupgPlugin ( QObject *parent, const QVariantList &/*args*/ )
    : Kopete::Plugin ( GnupgPluginFactory::componentData(), parent )
{
    if ( !mPluginStatic )
        mPluginStatic=this;
    KAction *action = new KAction ( KIcon ( "document-encrypt" ), i18nc ( "@action", "&Select Public Key..." ), this );
    actionCollection()->addAction ( "contactSelectKey", action );
    //connect ( action, SIGNAL (triggered(bool)), this, SLOT (slotSelectContactKey()) );
    connect ( Kopete::ContactList::self() , SIGNAL (metaContactSelected(bool)) , action , SLOT (setEnabled(bool)) );
    action->setEnabled ( Kopete::ContactList::self()->selectedMetaContacts().count() == 1 );

}

void GnupgPlugin::slotIncomingMessage(Kopete::MessageEvent *msg)
{

}

void GnupgPlugin::slotOUtgoingMessage(Kopete::Message &msg)
{

}

GnupgPlugin::~GnupgPlugin()
{

}

#include "gnupgplugin.moc"
