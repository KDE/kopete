#include "gadupubdir.h"

#include <qpushbutton.h>
#include <qtextedit.h>
#include <qwidgetstack.h>
#include <qlistview.h>
#include <qptrlist.h>

#include <kapplication.h>
#include <kdatewidget.h>
#include <klineedit.h>
#include <klocale.h>
#include <kurllabel.h>
#include <klistview.h>


GaduPublicDir::GaduPublicDir(GaduAccount *account,
	QWidget *parent, const char* name)
	: KDialogBase(parent, name, false, QString::null, User1|User2|User3|Cancel,
	User2 )
{
	mAccount = account;

	setCaption( i18n( "Gadu-Gadu Public Directory" ) );

	mMainWidget = new GaduPublicDirectory( this );
	setMainWidget( mMainWidget );
	
	mMainWidget->UIN->setValidChars( "1234567890" );

	setButtonText( User1, i18n( "New Search" ) );
	setButtonText( User2, i18n( "Search" ) );
	setButtonText( User3, i18n( "Add user.." ) );
	setButtonText( Cancel, i18n( "Exit.." ) );
	
	showButton(User1, false);
	showButton(User3, false);

	connect( this, SIGNAL(user2Clicked()), SLOT(slotSearch()) );
	connect( this, SIGNAL(user1Clicked()), SLOT(slotNewSearch()) );
	
	connect( account, SIGNAL(pubDirSearchResult( const searchResult & )),
				SLOT(slotSearchResult( const searchResult & )) );

	mAccount->pubDirSearchClose();
	
	show();
}
void GaduPublicDir::getData()
{

    fName	= mMainWidget->nameS->text();
    fSurname	= mMainWidget->surname->text();
    fNick	= mMainWidget->nick->text();
    fUin	= mMainWidget->UIN->text().toInt();
    fGender	= mMainWidget->gender->currentItem();
    fOnlyOnline	= mMainWidget->onlyOnline->isChecked();
    fAgeFrom	= mMainWidget->ageFrom->value();
    fAgeTo	= mMainWidget->ageTo->value();
    fCity	= mMainWidget->cityS->text();
}

bool GaduPublicDir::validateData()
{
    return true;
}

// Move to GaduProtocol someday
QPixmap GaduPublicDir::iconForStatus( uint status )
{
    QPixmap n;
    
    if (GaduProtocol::protocol()){
	return GaduProtocol::protocol()->convertStatus( status ).protocolIcon();
    }
    return n;
}

void GaduPublicDir::slotSearchResult( const searchResult &result )
{
    QListView *list=mMainWidget->listFound;    
    int i;
    
    kdDebug(14100) << "searchResults(" << result.count() <<")" << endl;

    // if not found anything, obviously we don't want to search for more
    
    if (result.count()){
	enableButton(User2, true);
    }
        
    enableButton(User1, true);
    
    QPtrListIterator< resLine > r (result);
    QListViewItem *sl;
    
    for ( i = result.count()  ; i-- ; ){
        kdDebug(14100) << "adding" << (*r)->uin << endl;
        sl= new QListViewItem( list, 
			    QString::fromLatin1(""),
			    (*r)->firstname,
			    (*r)->surname,
			    (*r)->nickname,
			    (*r)->age,
			    (*r)->city,
	    		    (*r)->uin);
	sl->setPixmap(0, iconForStatus((*r)->status) );
	++r;
    }
}

void GaduPublicDir::slotNewSearch()
{
    mMainWidget->pubsearch->raiseWidget(0);

    setButtonText( User2, i18n( "Search" ) );
    
    showButton(User1, false);
    showButton(User3, false);
    enableButton(User2, true);
    mAccount->pubDirSearchClose();
}

void GaduPublicDir::slotSearch()
{
	
	// search more, or search ?
	mMainWidget->listFound->clear();
	
	if (mMainWidget->pubsearch->id(mMainWidget->pubsearch->visibleWidget())==0){ 
	    
	    kdDebug(14100) << "start search... " << endl;
	    //
	    getData();
	    
	    // validate data
	    if (validateData()==false){
		return;
	    }
	    
	    // go on	
	    mMainWidget->pubsearch->raiseWidget(1);
	    	
	}
	else{
	    kdDebug(14100) << "search more... " << endl;	
	    // Search for more
	}
	
	setButtonText(User2, "Search more...");
	showButton(User3, true);    
	showButton(User1, true);    

	enableButton(User3, false);
	enableButton(User2, false);
	
	mAccount->pubDirSearch(fName, fSurname, fNick, 
			    fUin, fCity, fGender, fAgeFrom, fAgeTo, fOnlyOnline);
                            
}

#include "gadupubdir.moc"
