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
#include <kopetecontact.h>
#include <kbookmark.h>
#include <qregexp.h>
#include <kopetemessagemanagerfactory.h>
#include <kdebug.h>
#include <qvariant.h>
#include <kopeteglobal.h>
#include <qtextcodec.h>

K_EXPORT_COMPONENT_FACTORY( kopete_addbookmarks, AddBookmarksPluginFactory( "kopete_addbookmarks" )  )

AddBookmarksPlugin::AddBookmarksPlugin(QObject *parent, const char *name, const QStringList &args)
 : KopetePlugin(AddBookmarksPluginFactory::instance(), parent, name)
{
	//kdDebug(14501) << "plugin loading" << endl;
	connect( KopeteMessageManagerFactory::factory(), SIGNAL( aboutToDisplay( KopeteMessage & ) ), this, SLOT( slotBookmarkURLsInMessage( KopeteMessage & ) ) );
}


AddBookmarksPlugin::~AddBookmarksPlugin()
{
}


#include "addbookmarksplugin.moc"


/*!
    \fn AddBookmarksPlugin::slotBookmarkURLsInMessage(KopeteMessage & msg)
 */
void AddBookmarksPlugin::slotBookmarkURLsInMessage(KopeteMessage & msg)
{
	//kdDebug(14501) << "recieved message:" << endl << msg.parsedBody() << endl;
	if(msg.direction() != KopeteMessage::Inbound)
		return;
	KURL::List *URLsList;
	KURL::List::iterator it;
	URLsList = ExtractURLsFromString(msg.parsedBody());
	if (!URLsList->empty()) {
		for( it = URLsList->begin() ; it != URLsList->end() ; ++it){
			AddKopeteBookmark(*it, msg.from()->property(Kopete::Global::Properties::self()->nickName()).value().toString() );
			//kdDebug (14501) << "name:" << msg.from()->property(Kopete::Global::Properties::self()->nickName()).value().toString() << endl;
		}
	}
	delete URLsList;
}

void AddBookmarksPlugin::slotAddKopeteBookmark( KIO::Job *transfer, const QByteArray &data )
{
	QTextCodec *codec = GetPageEncoding( data );
	QString htmlpage = codec->toUnicode( data );
	QRegExp rx("<(title|TITLE)>[^<]*</(title|TITLE)>");
	int pos = rx.search( htmlpage );
	KBookmarkManager *mgr = KBookmarkManager::userBookmarksManager();
	KBookmarkGroup group = GetKopeteFolder();
	
	if( m_map[(KIO::TransferJob*)transfer].sender.compare( "" ) && m_settings.isUseSubfolderForContact( m_map[(KIO::TransferJob*)transfer].sender )){
		group = GetFolder( group, m_map[(KIO::TransferJob*)transfer].sender );
	} 
	if( pos == -1 ){
		group.addBookmark( mgr, m_map[(KIO::TransferJob*)transfer].url.prettyURL(), m_map[(KIO::TransferJob*)transfer].url.url() );
		kdDebug( 14501 ) << "failed to extract title from first data chunk" << endl;
	}else {
		group.addBookmark( mgr, htmlpage.mid(pos+7, rx.matchedLength()-15).simplifyWhiteSpace() , m_map[(KIO::TransferJob*)transfer].url.url() );
	}
	mgr->save();
	m_map.remove( (KIO::TransferJob*)transfer );
	transfer->kill();
}

KURL::List* AddBookmarksPlugin::ExtractURLsFromString(QString text)
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

void AddBookmarksPlugin::AddKopeteBookmark (KURL url, QString sender )
{
	KBookmarkGroup group = GetKopeteFolder();
	KIO::TransferJob *transfer;
	
	if( !isURLInGroup( url, group ) ){
		// make asynchronous transfer to avoid GUI freezing due to overloaded web servers
		transfer = KIO::get(url, false, false);
		connect ( transfer, SIGNAL ( data( KIO::Job *, const QByteArray & ) ),
		this, SLOT ( slotAddKopeteBookmark( KIO::Job *, const QByteArray & ) ) );
		m_map[transfer].url = url;
		m_map[transfer].sender = sender;
	}
}

KBookmarkGroup AddBookmarksPlugin::GetKopeteFolder()
{
	KBookmarkManager *mgr = KBookmarkManager::userBookmarksManager();

	return GetFolder( mgr->root(), "kopete" );
}

bool AddBookmarksPlugin::isURLInGroup(KURL url, KBookmarkGroup group)
{
	KBookmark bookmark = group.first();
	
	for( ; !bookmark.isNull() ; bookmark = group.next(bookmark) ){
		if( !bookmark.isGroup() && !bookmark.isSeparator() )
			if( url == bookmark.url() )
				return true;
	}
	return false;
}

KBookmarkGroup AddBookmarksPlugin::GetFolder( KBookmarkGroup group, QString folder )
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

QTextCodec* AddBookmarksPlugin::GetPageEncoding( QByteArray data )
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

void AddBookmarksPlugin::slotReloadSettings()
{
	m_settings.load();
}
