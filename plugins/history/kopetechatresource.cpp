#include "kopetechatresource.h"

#include "settings.h"
#include "settingsadaptor.h"
#include "history.h"
#include "historyxmlio.h"
#include <QtDBus/QDBusConnection>
#include <QtCore/QDir>
#include <akonadi/itemfetchscope.h>
#include <akonadi/changerecorder.h>
#include <kfiledialog.h>
#include <klocalizedstring.h>
#include <krandom.h>


using namespace Akonadi;

kopeteChatResource::kopeteChatResource( const QString &id )
  : ResourceBase( id )
{
  new SettingsAdaptor( Settings::self() );
  QDBusConnection::sessionBus().registerObject( QLatin1String( "/Settings" ),
                            Settings::self(), QDBusConnection::ExportAdaptors );
  
  changeRecorder()->itemFetchScope().fetchFullPayload();
  // TODO: you can put any resource specific initialization code here.
}

kopeteChatResource::~kopeteChatResource()
{
}

void kopeteChatResource::retrieveCollections()
{
  Collection c;
  c.setParent( Collection::root() );
  c.setRemoteId( Settings::self()->path() );
  c.setName("kopeteChat" );
 
  QStringList mimeTypes;
  mimeTypes << "inode/directory" ;
  c.setContentMimeTypes( mimeTypes );
 
  Collection::List list;
  list << c;
  collectionsRetrieved( list );
  // TODO: this method is called when Akonadi wants to have all the
  // collections your resource provides.
  // Be sure to set the remote ID and the content MIME types
}

void kopeteChatResource::retrieveItems( const Akonadi::Collection &collection )
{
  const QString path = collection.remoteId();
  QDir dir( path );
  QStringList filters;
  filters << QLatin1String( "*.xml" );
  const QStringList fileList = dir.entryList( filters, QDir::Files );
 
  Item::List items;
  
//here a loop is starting
  foreach( const QString &file, fileList ) {
    Item item( QLatin1String( "application/x-vnd.kde.kopetechathistory" ) );
    item.setRemoteId( path + QLatin1Char( '/' ) + file );
    items << item;
  }


  itemsRetrieved( items );


  // TODO: this method is called when Akonadi wants to know about all the
  // items in the given collection. You can but don't have to provide all the
  // data for each item, remote ID and MIME type are enough at this stage.
  // Depending on how your resource accesses the data, there are several
  // different ways to tell Akonadi when you are done.
}

bool kopeteChatResource::retrieveItem( const Akonadi::Item &item, const QSet<QByteArray> &parts )
{
  Q_UNUSED( parts );
  History history;

//  QBuffer buff;
//  buff.open( QIODevice::ReadWrite );
//  buff.write("\n part begins \n ");
//  kDebug() << buff.data();

  const QString fileName = item.remoteId();

  QFile file( fileName );
  if ( !file.open( QFile::ReadOnly ) )
    return false;
 
  if ( file.error() != QFile::NoError ){
   //buff.write("file error");
  //  kDebug() << buff.data();
    return false;
  }

    //buff.write("\n 000");
 HistoryXmlIo::readHistoryFromXml( &file,history );
//  buff.write("\n1");  
/*  HistoryXmlIo::readHistoryHeaderFromXml( &file,history );
    HistoryXmlIo::readHistoryMessagesFromXml( &file,history ); 
    buff.write("\n2");*/
  Item newItem( item );
	  
	  //const QByteArray data = file.readAll();
//	  buff.write("\n here it begins \n");
	  //buff.write(data);
  //HistoryXmlIo::writeHistoryToXml( history, &buff );
//	  kDebug() << buff.data();
/*	  HistoryXmlIo::writeHistoryHeaderToXml(history,&buff );
	  kDebug() << buff.data();
	  HistoryXmlIo::writeHistoryMessagesToXml( history, &buff );
*///	  kDebug() << buff.data();
	  //HistoryXmlIo::testXml(&buff);
	  //buff.write("\n here it ends \n ");
//	  HistoryXmlIo::writeHistoryToXml(history, &buff);
	//  HistoryXmlIo::readHistoryFromXml( &buff,history );
	  //buff.write("got anything?? :-) :-P ");
	  //kDebug() << buff.data();
	//  buff.close();


  newItem.setPayload < History > ( history );
	/*  QBuffer buffer;
	  buffer.open( QIODevice::WriteOnly );
	  HistoryXmlIo::readHistoryFromXml( &buffer,history );
	  kDebug() << buffer.data();
	  buffer.close(); */

  itemRetrieved( newItem );//here is the problem
  return true;

  // TODO: this method is called when Akonadi wants more data for a given item.
  // You can only provide the parts that have been requested but you are allowed
  // to provide all in one go

  return true;
}

void kopeteChatResource::aboutToQuit()
{
  // TODO: any cleanup you need to do while there is still an active
  // event loop. The resource will terminate after this method returns
}

void kopeteChatResource::configure( WId windowId )
{
  Q_UNUSED( windowId );

  const QString oldPath = Settings::self()->path();
  KUrl url;
  if ( !oldPath.isEmpty() )
    url = KUrl::fromPath( oldPath );
  else
    url = KUrl::fromPath( QDir::homePath() );
 
  const QString title = i18nc( "@title:window", "reading xml history" );
  const QString newPath = KFileDialog::getExistingDirectory( url, 0, title );
 
  if ( newPath.isEmpty() )
    return;
 
  if ( oldPath == newPath )
    return;
 
  Settings::self()->setPath( newPath );
 
  Settings::self()->writeConfig();
 
  synchronize();

  // TODO: this method is usually called when a new resource is being
  // added to the Akonadi setup. You can do any kind of user interaction here,
  // e.g. showing dialogs.
  // The given window ID is usually useful to get the correct
  // "on top of parent" behavior if the running window manager applies any kind
  // of focus stealing prevention technique
}

void kopeteChatResource::itemAdded( const Akonadi::Item &item, const Akonadi::Collection &collection )
{
    const QString path = collection.remoteId();
 
  History history;
  if ( item.hasPayload<History>() )
    history = item.payload<History>();
 
//  if ( history.uid().isEmpty() )
//  history.setUid( KRandom::randomString( 10 ) );
    QDate date = QDate::currentDate();
    
 
  QFile file( path + QLatin1Char( '/' ) +KRandom::randomString( 10 )+date.toString(".yyyyMM")  + QLatin1String( ".xml" ) );
 
  if ( !file.open( QFile::WriteOnly ) )
    return;
 
  //KABC::VCardConverter converter;
  //file.write( converter.createVCard( addressee ) );
  if ( file.error() != QFile::NoError )
    return;
 
  Item newItem( item );
  newItem.setRemoteId( file.fileName() );
  newItem.setPayload<History>( history );
 
  changeCommitted( newItem );


  // TODO: this method is called when somebody else, e.g. a client application,
  // has created an item in a collection managed by your resource.

  // NOTE: There is an equivalent method for collections, but it isn't part
  // of this template code to keep it simple
}

void kopeteChatResource::itemChanged( const Akonadi::Item &item, const QSet<QByteArray> &parts )
{
  Q_UNUSED( parts );
 
  const QString fileName = item.remoteId();
 
  History history;
  if ( item.hasPayload<History>() )
    history = item.payload<History>();
 
//  if ( history.uid().isEmpty() )
//    history.setUid( KRandom::randomString( 10 ) );
 
  QFile file( fileName );
 
  if ( !file.open( QFile::WriteOnly ) )
    return;
 
  //KABC::VCardConverter converter;
  //file.write( converter.createVCard( addressee ) );
  if ( file.error() != QFile::NoError )
    return;
 
  Item newItem( item );
  newItem.setPayload<History>( history );
 
  changeCommitted( newItem );

  // TODO: this method is called when somebody else, e.g. a client application,
  // has changed an item managed by your resource.

  // NOTE: There is an equivalent method for collections, but it isn't part
  // of this template code to keep it simple
}

void kopeteChatResource::itemRemoved( const Akonadi::Item &item )
{
  const QString fileName = item.remoteId();
 
  QFile::remove( fileName );
 
  changeCommitted( item );


  // TODO: this method is called when somebody else, e.g. a client application,
  // has deleted an item managed by your resource.

  // NOTE: There is an equivalent method for collections, but it isn't part
  // of this template code to keep it simple
}

AKONADI_RESOURCE_MAIN( kopeteChatResource )

#include "kopetechatresource.moc"
