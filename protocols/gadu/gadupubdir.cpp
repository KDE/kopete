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
	: KDialogBase(parent, name, false, QString::null, User1|User2|Cancel,
	User1 )
{
	mAccount = account;

	setCaption(i18n("Gadu-Gadu Public Directory"));

	mMainWidget = new GaduPublicDirectory(this);
	setMainWidget(mMainWidget);

	setButtonText(User1, "Search");
	setButtonText(User2, "Add user..");
	showButton(User2, false);

	connect( this, SIGNAL(user1Clicked()), SLOT(slotSearch()) );
	
	show();
}

void GaduPublicDir::slotSearch()
{
	
	// search more, or search ?
	
	if (mMainWidget->pubsearch->id(mMainWidget->pubsearch->visibleWidget())==0){ 
	    kdDebug(14100) << "start search... " << endl;
	    // validate data
	
	    // go on	
	    mMainWidget->pubsearch->raiseWidget(1);
	    setButtonText(User1, "Search more...");
	    showButton(User2, true);
	    enableButton(User2, false);
	    	
	}
	else{
	    kdDebug(14100) << "search more... " << endl;
	
	    // Search for more
	}
}

#include "gadupubdir.moc"
