#include "gadupubdir.h"

#include <qpushbutton.h>
#include <qtextedit.h>
#include <qwidgetstack.h>

#include <kapplication.h>
#include <kdatewidget.h>
#include <klineedit.h>
#include <klocale.h>
#include <kurllabel.h>


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

void GaduPublicDir::slotSearchResult( const searchResult &result )
{
    enableButton(User2, true);
    enableButton(User1, true);
    
    kdDebug(14100) << "searchResult" << endl;
}

void GaduPublicDir::slotNewSearch()
{
    mMainWidget->pubsearch->raiseWidget(0);

    setButtonText( User2, i18n( "Search" ) );
    
    showButton(User1, false);
    showButton(User3, false);
    enableButton(User2, true);
        
}

void GaduPublicDir::slotSearch()
{
	
	// search more, or search ?
	
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
