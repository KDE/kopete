/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include "searchdialog.h"
#include "ui_searchdialog.h"

//
#include <KDebug>
#include <KHTMLPart>

#include <QLineEdit>
#include <QListWidgetItem>
#include <QStringListModel>

#include <ontologies/nie.h>
#include <ontologies/nmo.h>
#include <ontologies/nco.h>
#include <ontologies/immessage.h>

#include <Nepomuk/Tag>
#include <Nepomuk/Query/Query>
#include <Nepomuk/Query/AndTerm>
#include <Nepomuk/Query/OrTerm>
#include <Nepomuk/Query/LiteralTerm>
#include <Nepomuk/Query/ResourceTerm>
#include <Nepomuk/Query/NegationTerm>
#include <Nepomuk/Query/ResourceTypeTerm>
#include <Nepomuk/Query/QueryParser>
#include <Nepomuk/Query/ComparisonTerm>
#include <Nepomuk/ResourceManager>

#include <Soprano/QueryResultIterator>
#include <Soprano/Query/QueryLanguage>
#include <Soprano/Model>
#include <Soprano/Vocabulary/NAO>

#include <Akonadi/ItemSearchJob>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>

#include <history/history.h>

SearchDialog::SearchDialog(QWidget* parent, Qt::WFlags flags): KDialog(parent, flags)
{
    kDebug() << " ";
    setAttribute (Qt::WA_DeleteOnClose, true);
    setCaption( i18n("Search Through Logs") );
//    setButtons(KDialog::Close);
    setButtons(KDialog::None);

    QWidget *w = new QWidget( this );
    
    m_MainWidget = new Ui::SearchDialog();
    m_MainWidget->setupUi( w );
    
    m_resultModel = new QStringListModel( this ) ;
    m_MainWidget->ListResultWidget->setModel( m_resultModel );
    
    /*
    m_MainWidget->DisplayResultWidget;
    m_MainWidget->LabelBox;
    m_MainWidget->ListResultWidget;
    m_MainWidget->progressBar;
    m_MainWidget->SearchButton;
    m_MainWidget->SearchTextBox;
    m_MainWidget->SortBy;
    */
    
    connect(m_MainWidget->SearchButton, SIGNAL(clicked(bool)), this, SLOT(slotSearchButtonClicked()) );
    connect( m_MainWidget->ListResultWidget, SIGNAL(clicked(const QModelIndex&)),
		   this, SLOT(itemSelected(const QModelIndex&)));
    
    setMainWidget( w );
//    show();
}

SearchDialog::~SearchDialog()
{
    delete m_MainWidget ;
}

void SearchDialog::reset()
{
    //TODO:should be in a reset function
    m_resultModel->setStringList( QStringList() );
    m_MainWidget->DisplayResultWidget->clear();
    m_items.clear();
	    
}


void SearchDialog::slotSearchButtonClicked()
{
    kDebug() << " " << m_MainWidget->SearchTextBox->text() ;
    
    //to reset the gui && clear the saved items
    reset(); 
    
    QString query = "select distinct ?r ?reqProp1 \
	    where { ?r <http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#plainTextMessageContent> ?o .\
	    FILTER REGEX(STR(?o),'" + m_MainWidget->SearchTextBox->text() + "', 'i') . FILTER isLiteral(?o) . \
	    ?r <http://akonadi-project.org/ontologies/aneo#akonadiItemId> ?reqProp1 . }";
	    
    Akonadi::ItemSearchJob *job = new Akonadi::ItemSearchJob( query );
    job->fetchScope().fetchFullPayload();
    connect(job, SIGNAL(result(KJob*)), this , SLOT(itemSearchJobDone(KJob*)) );
}

void SearchDialog::itemSearchJobDone(KJob* job)
{
    if( job->error() ) {
	kDebug() << job->errorText() ;
	return;
    }
    
    Akonadi::ItemSearchJob * j = static_cast<Akonadi::ItemSearchJob*> (job);
    m_items = j->items() ;
    kDebug() << "sixe=:" <<m_items.size() ;
    QStringList resultIndex;
    foreach( const Akonadi::Item &item , m_items )
    {
	if( !item.hasPayload<History>() )
	    continue;
	
	kDebug() << item.remoteId() ; 
	resultIndex << QString::number( item.id() ) + 
			item.payload<History>().remoteContactId() ; 
    }
    
    m_resultModel->setStringList( resultIndex );
}

void SearchDialog::itemSelected(const QModelIndex& index)
{
    kDebug() << " " ;
    if( !index.isValid() )
	return;
    
    kDebug() << index.row() ;
    
    Akonadi::Item i = m_items.at( index.row() );
    History history = i.payload<History>();

    displayResult(history);
}

void SearchDialog::itemFetched(KJob* job )
{
    if( job->error() ) {
	kDebug() << job->errorText() ;
	return;
    }
    
    Akonadi::ItemFetchJob *f = static_cast<Akonadi::ItemFetchJob*>(job);
    
    Akonadi::Item item = f->items().first() ;
    History history = item.payload<History>();
    displayResult(history);

}

//set the formating of the chat to be displayed
void SearchDialog::displayResult(const History &his)
{
 //   m_MainWidget->DisplayResultWidget->setHtml(); ;
    QString finalHtml ;
    
    QString date = his.date().toString() ;
    QString remoteContact = his.remoteContactId() ;
    
    finalHtml += date + "<br/><b>" + remoteContact + "</b><br/>";
    
    foreach( const History::Message &msg, his.messages() )
    {
	QString time = "<b>" + msg.timestamp().time().toString() + "</b>" ;
	QString nick = msg.nick() ;
	QString sender = msg.sender();
	QString text ="<font color=\"red\">" + msg.text() + "</font>"  ;
	QString in = msg.in() ;

	if( text.contains(m_MainWidget->SearchTextBox->text(), Qt::CaseInsensitive ) )
	{
	    kDebug() << "contains " << m_MainWidget->SearchTextBox->text() << text ;
	    QString text2 = "<span style=\"background-color:blue\">" + m_MainWidget->SearchTextBox->text() + "</span>" ;
	    text.replace(m_MainWidget->SearchTextBox->text() , text2 , Qt::CaseInsensitive) ;
	}
	
	if( in.toInt() ==  0 )
	{
	    finalHtml += time + "<br/>" + "me: " + text + "<br/>" ;
	}
	else
	    finalHtml += time + "<br/>" + nick + ": " + text + "<br/>";
    }
    m_MainWidget->DisplayResultWidget->enableRichTextMode();
    
    m_MainWidget->DisplayResultWidget->setHtml(finalHtml);
}
/*
 full text search query
 "select distinct ?r ?reqProp1 \
	    where { ?r <http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#plainTextMessageContent> ?o .\
	    FILTER REGEX(STR(?o),'" + m_MainWidget->SearchTextBox->text() + "', 'i') . FILTER isLiteral(?o) . \
	    ?r <http://akonadi-project.org/ontologies/aneo#akonadiItemId> ?reqProp1 . }"
 
 
     Nepomuk::Query::Query query;
    Nepomuk::Query::OrTerm ot;
    Nepomuk::Query::ComparisonTerm ct(Vocabulary::NMO::plainTextMessageContent(),
			Nepomuk::Query::LiteralTerm( m_MainWidget->SearchTextBox->text() ), 
			Nepomuk::Query::ComparisonTerm::Contains );
    
    ot.addSubTerm(ct);
    query.setTerm(ot);
    const Nepomuk::Query::Query::RequestProperty itemIdProperty( Akonadi::ItemSearchJob::akonadiItemIdUri(), false );
    query.addRequestProperty(itemIdProperty);
    
    kDebug() << query.toSparqlQuery() ;
 
    Nepomuk::ResourceManager::instance()->init();
    Soprano::QueryResultIterator it2
    = Nepomuk::ResourceManager::instance()->mainModel()->executeQuery( query.toSparqlQuery() , 
                          Soprano::Query::QueryLanguageSparql );
    while( it2.next() ) {
	kDebug() << Nepomuk::Resource( it2.binding( "r" ).uri() ).uri() ;
	Soprano::Node n = it2.binding( "reqProp1" ) ;
	kDebug() << n.uri() ;
    }
    

 */




#include "searchdialog.moc"