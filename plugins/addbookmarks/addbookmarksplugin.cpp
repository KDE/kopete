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

#include <kdebug.h>
#include <kbookmark.h>
#include <qvariant.h>
#include <qtextcodec.h>
#include <qregexp.h>

#include "addbookmarksplugin.moc"
#include "addbookmarksplugin.h"
#include "kopetecontact.h"
#include "kopetechatsessionmanager.h"
#include "kopeteglobal.h"
#include "kopetemetacontact.h"


K_EXPORT_COMPONENT_FACTORY( kopete_addbookmarks, BookmarksPluginFactory( "kopete_addbookmarks" )  )


static bool isURLInGroup(const KURL& url, const KBookmarkGroup& group)
{
	KBookmark bookmark = group.first();
	
	for( ; !bookmark.isNull() ; bookmark = group.next(bookmark) ){
		if( !bookmark.isGroup() && !bookmark.isSeparator() )
			if( url == bookmark.url() )
				return true;
	}
	return false;
}

BookmarksPlugin::BookmarksPlugin(QObject *parent, const char *name, const QStringList &/*args*/)
 : Kopete::Plugin(BookmarksPluginFactory::instance(), parent, name)
{
	//kdDebug(14501) << "plugin loading" << endl;
	connect( Kopete::ChatSessionManager::self(), SIGNAL( aboutToDisplay( Kopete::Message & ) ), this, SLOT( slotBookmarkURLsInMessage( Kopete::Message & ) ) );
}

/*!
    \fn BookmarksPlugin::slotBookmarkURLsInMessage(KopeteMessage & msg)
 */
void BookmarksPlugin::slotBookmarkURLsInMessage(Kopete::Message & msg)
{
	//kdDebug(14501) << "recieved message:" << endl << msg.parsedBody() << endl;
	if(msg.direction() != Kopete::Message::Inbound)
		return;
	KURL::List *URLsList;
	KURL::List::iterator it;
	URLsList = extractURLsFromString( msg.parsedBody() );
	if (!URLsList->empty()) {
		for( it = URLsList->begin() ; it != URLsList->end() ; ++it){
			if ( m_settings.addBookmarksFromUnknownContacts() || !msg.from()->metaContact()->isTemporary() )
			{
				if ( msg.from()->metaContact() ) {
					addKopeteBookmark(*it, msg.from()->metaContact()->displayName() );
					//kdDebug (14501) << "name:" << msg.from()->metaContact()->displayName() << endl;
				}
				else {
					addKopeteBookmark(*it, msg.from()->property(Kopete::Global::Properties::self()->nickName()).value().toString() );
					//kdDebug (14501) << "name:" << msg.from()->property(Kopete::Global::Properties::self()->nickName()).value().toString() << endl;
				}
			}
		}
	}
	delete URLsList;
}

void BookmarksPlugin::slotAddKopeteBookmark( KIO::Job *transfer, const QByteArray &data )
{
	QTextCodec *codec = getPageEncoding( data );
	QString htmlpage = codec->toUnicode( data );
	QRegExp rx("<title>([^<]*){1,96}</title>");
	rx.setCaseSensitive(false);
	int pos = rx.search( htmlpage );
	KBookmarkManager *mgr = KBookmarkManager::userBookmarksManager();
	KBookmarkGroup group = getKopeteFolder();
	QString sender = m_map[(KIO::TransferJob*)transfer].sender;
	
	if ( m_settings.useSubfolderForContact( sender ) )
		group = getFolder( group, sender );

	if( pos == -1 ){
		group.addBookmark( mgr, m_map[(KIO::TransferJob*)transfer].url.prettyURL(), m_map[(KIO::TransferJob*)transfer].url.url() );
		kdDebug( 14501 ) << "failed to extract title from first data chunk" << endl;
	}else {
		group.addBookmark( mgr, rx.cap( 1 ).simplifyWhiteSpace(),
						   m_map[(KIO::TransferJob*)transfer].url.url() );
	}
	mgr->save();
	mgr->emitChanged( group );
	m_map.remove( (KIO::TransferJob*)transfer );
	transfer->kill();
}

KURL::List* BookmarksPlugin::extractURLsFromString( const QString& text )
{
	KURL::List *list = new KURL::List;
	QRegExp rx("<a href=\"[^\\s\"]+\"");
	int pos=0;
	KURL url;
	
	for(; (pos=rx.search(text, pos))!=-1; pos+=rx.matchedLength()){
	 //as long as there is a matching URL in text
		url = text.mid(pos+9, rx.matchedLength()-10);
		// assuming that in formatted messages links appear as <a href="link"
		if(url.isValid())
			list->append(url);
	}
	return list;
}

void BookmarksPlugin::addKopeteBookmark( const KURL& url, const QString& sender )
{
	KBookmarkGroup group = getKopeteFolder();

	if ( m_settings.useSubfolderForContact( sender ) ) {
		group = getFolder( group, sender );
	}
	// either restrict to http(s) or to KProtocolInfo::protocolClass() == :internet
	if( !isURLInGroup( url, group )
		&& url.isValid() && url.protocol().startsWith("http") ) {
		KIO::TransferJob *transfer;
		// make asynchronous transfer to avoid GUI freezing due to overloaded web servers
		transfer = KIO::get(url, false, false);
		transfer->setInteractive(false);
		connect ( transfer, SIGNAL ( data( KIO::Job *, const QByteArray & ) ),
		this, SLOT ( slotAddKopeteBookmark( KIO::Job *, const QByteArray & ) ) );
		m_map[transfer].url = url;
		m_map[transfer].sender = sender;
	}
}

KBookmarkGroup BookmarksPlugin::getKopeteFolder()
{
	KBookmarkManager *mgr = KBookmarkManager::userBookmarksManager();

	return getFolder( mgr->root(), QString::fromLatin1("kopete") );
}

KBookmarkGroup BookmarksPlugin::getFolder( KBookmarkGroup group, const QString& folder )
{
	KBookmark bookmark;


	for( bookmark=group.first(); !bookmark.isNull() && !(bookmark.isGroup() && !bookmark.fullText().compare( folder )); bookmark = group.next(bookmark));
	if( bookmark.isNull() ){
		KBookmarkManager *mgr = KBookmarkManager::userBookmarksManager();
		//kdDebug (14501) << "GetFolder:" << folder << endl;
		group = group.createNewFolder( mgr, folder, true);
	}else {
		group = bookmark.toGroup();
	}
	return group;
}

QTextCodec* BookmarksPlugin::getPageEncoding( const QByteArray& data )
{
	QString temp = QString::fromLatin1(data);
	QRegExp rx("<meta[^>]*(charset|CHARSET)\\s*=\\s*[^>]*>");
	int pos = rx.search( temp );
	QTextCodec *codec;
	
	if( pos == -1 ){
		kdDebug( 14501 ) << "charset not found in first data chunk" << endl;
		return QTextCodec::codecForName("iso8859-1");
	}
	//kdDebug(14501) << temp.mid(pos, rx.matchedLength()) << endl;
	temp = temp.mid(pos, rx.matchedLength()-1);
	temp = temp.mid( temp.find("charset", 0, false)+7);
	temp = temp.remove('=').simplifyWhiteSpace();
	for( pos = 0 ; temp[pos].isLetterOrNumber() || temp[pos] == '-' ; pos++ );
	temp = temp.left( pos );
	//kdDebug(14501) << "encoding: " << temp << endl;
	codec = QTextCodec::codecForName( temp.latin1() );
	if( !codec ){
		return QTextCodec::codecForName("iso8859-1");
	}
	return codec;
}

void BookmarksPlugin::slotReloadSettings()
{
	m_settings.load();
}
