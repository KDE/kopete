//
// C++ Implementation: %{MODULE}
//
// Description:
//
//
// Author: Roie Kerstein <sf_kersteinroie@bezeqint.net>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "addbookmarksplugin.h"

#include <kdebug.h>
#include <kbookmark.h>
#include <qvariant.h>
#include <qtextcodec.h>
#include <qregexp.h>

#include "kopetecontact.h"
#include "kopetechatsessionmanager.h"
#include "kopeteglobal.h"
#include "kopetemetacontact.h"

K_PLUGIN_FACTORY( BookmarksPluginFactory, registerPlugin<BookmarksPlugin>(); )
K_EXPORT_PLUGIN( BookmarksPluginFactory( "kopete_addbookmarks" ) )

BookmarksPlugin::BookmarksPlugin(QObject *parent, const QVariantList &/*args*/)
 : Kopete::Plugin(BookmarksPluginFactory::componentData(), parent)
{
	//kDebug(14501) << "plugin loading";
	connect( Kopete::ChatSessionManager::self(), SIGNAL(aboutToDisplay(Kopete::Message&)), this, SLOT(slotBookmarkURLsInMessage(Kopete::Message&)) );
}


BookmarksPlugin::~BookmarksPlugin()
{
}


#include "addbookmarksplugin.moc"


/*!
    \fn BookmarksPlugin::slotBookmarkURLsInMessage(KopeteMessage & msg)
 */
void BookmarksPlugin::slotBookmarkURLsInMessage(Kopete::Message & msg)
{
	//kDebug(14501) << "received message:" << endl << msg.parsedBody();
	if(msg.direction() != Kopete::Message::Inbound)
		return;
	KUrl::List::iterator it;
	KUrl::List *URLsList = extractURLsFromString( msg.parsedBody() );
	if (!URLsList->empty()) {
		for( it = URLsList->begin() ; it != URLsList->end() ; ++it){
			if( msg.from()->metaContact() ) {
				addKopeteBookmark(*it, msg.from()->metaContact()->displayName() );
				//kDebug (14501) << "name:" << msg.from()->metaContact()->displayName();
			}
			else {
				addKopeteBookmark(*it, msg.from()->displayName() );
				//kDebug (14501) << "name:" << msg.from()->displayName();
			}
		}
	}
	delete URLsList;
}

void BookmarksPlugin::slotAddKopeteBookmark( KIO::Job *transfer, const QByteArray &data )
{
	QTextCodec *codec = getPageEncoding( data );
	const QString htmlpage = codec->toUnicode( data );
	QRegExp rx("<(?:title|TITLE)>([^<]*)</(?:title|TITLE)>");
	int pos = rx.indexIn( htmlpage );
	KBookmarkManager *mgr = KBookmarkManager::userBookmarksManager();
	KBookmarkGroup group = getKopeteFolder();
	const QString sender = m_map[(KIO::TransferJob*)transfer].sender;
	
	if ( m_settings.useSubfolderForContact( sender ) )
		group = getFolder( group, sender );

	if( pos == -1 ){
		group.addBookmark( m_map[(KIO::TransferJob*)transfer].url.prettyUrl(), m_map[(KIO::TransferJob*)transfer].url.url() );
		kDebug( 14501 ) << "failed to extract title from first data chunk";
	}else {
		group.addBookmark( rx.cap( 1 ).simplified(),
                                   m_map[(KIO::TransferJob*)transfer].url.url() );
	}
	mgr->emitChanged( group );
	m_map.remove( (KIO::TransferJob*)transfer );
	transfer->kill();
}

KUrl::List* BookmarksPlugin::extractURLsFromString( const QString &text )
{
	KUrl::List *list = new KUrl::List;
	QRegExp rx("<a href=\"[^\\s\"]+\"");
	int pos=0;
	KUrl url;
	
	for(; (pos=rx.indexIn(text, pos))!=-1; pos+=rx.matchedLength()){
	 //as long as there is a matching URL in text
		url = text.mid(pos+9, rx.matchedLength()-10);
		// assuming that in formatted messages links appear as <a href="link"
		if(url.isValid())
			list->append(url);
	}
	return list;
}

void BookmarksPlugin::addKopeteBookmark( const KUrl &url, const QString &sender )
{
	KBookmarkGroup group = getKopeteFolder();

	if ( m_settings.useSubfolderForContact( sender ) ) {
		group = getFolder( group, sender );
	}
	if( !isURLInGroup( url, group ) ){
		KIO::TransferJob *transfer;
		// make asynchronous transfer to avoid GUI freezing due to overloaded web servers
		transfer = KIO::get(url, KIO::NoReload, KIO::HideProgressInfo);
		connect ( transfer, SIGNAL (data(KIO::Job*,QByteArray)),
		this, SLOT (slotAddKopeteBookmark(KIO::Job*,QByteArray)) );
		m_map[transfer].url = url;
		m_map[transfer].sender = sender;
	}
}

KBookmarkGroup BookmarksPlugin::getKopeteFolder()
{
	KBookmarkManager *mgr = KBookmarkManager::userBookmarksManager();

	return getFolder( mgr->root(), "kopete" );
}

bool BookmarksPlugin::isURLInGroup(const KUrl &url, KBookmarkGroup group)
{
	KBookmark bookmark = group.first();
	
	for( ; !bookmark.isNull() ; bookmark = group.next(bookmark) ){
		if( !bookmark.isGroup() && !bookmark.isSeparator() )
			if( url == bookmark.url() )
				return true;
	}
	return false;
}

KBookmarkGroup BookmarksPlugin::getFolder( KBookmarkGroup group, QString folder )
{
	KBookmark bookmark;


	for( bookmark=group.first(); !bookmark.isNull() && !(bookmark.isGroup() && !bookmark.fullText().compare( folder )); bookmark = group.next(bookmark));
	if( bookmark.isNull() ){
		KBookmarkManager *mgr = KBookmarkManager::userBookmarksManager();
		//kDebug (14501) << "GetFolder:" << folder;
		group = group.createNewFolder( folder );
                mgr->emitChanged(group);
	}else {
		group = bookmark.toGroup();
	}
	return group;
}

QTextCodec* BookmarksPlugin::getPageEncoding( const QByteArray &data )
{
	QString temp = QString::fromLatin1(data);
	QRegExp rx("<meta[^>]*(charset|CHARSET)\\s*=\\s*[^>]*>");
	int pos = rx.indexIn( temp );
	QTextCodec *codec;
	
	if( pos == -1 ){
		kDebug( 14501 ) << "charset not found in first data chunk";
		return QTextCodec::codecForName("iso8859-1");
	}
	//kDebug(14501) << temp.mid(pos, rx.matchedLength());
	temp = temp.mid(pos, rx.matchedLength()-1);
	temp = temp.mid( temp.indexOf("charset", 0, Qt::CaseInsensitive)+7);
	temp = temp.remove('=').simplified();
	for( pos = 0 ; temp[pos].isLetterOrNumber() || temp[pos] == '-' ; ++pos ) {};
	temp = temp.left( pos );
	//kDebug(14501) << "encoding: " << temp;
	codec = QTextCodec::codecForName( temp.toLatin1() );
	if( !codec ){
		return QTextCodec::codecForName("iso8859-1");
	}
	return codec;
}

void BookmarksPlugin::slotReloadSettings()
{
	m_settings.load();
}
