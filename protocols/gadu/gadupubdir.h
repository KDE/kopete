#ifndef GADUPUBDIR_H
#define GADUPUBDIR_H

#include "gaduaccount.h"
#include "gaduprotocol.h"
#include "gadusearch.h"

#include <kdebug.h>
#include <qhbox.h>
#include <kdialogbase.h>
#include <krestrictedline.h>
#include <kcombobox.h>
#include <qcheckbox.h>
#include <qspinbox.h>


class GaduProtocol;
class GaduAccount;
class GaduPublicDirectory;

class GaduPublicDir : public KDialogBase
{
	Q_OBJECT

	public:
		GaduPublicDir(GaduAccount *,
			QWidget *parent = 0, const char* name = "GaduPublicDir");

	
	private slots:
		void slotSearch();
		void slotNewSearch();
		void slotSearchResult( const searchResult &result );

	private:

		void getData();
		bool validateData();

		GaduProtocol *p;
		GaduAccount *mAccount;
		GaduContact *mContact;
		GaduPublicDirectory *mMainWidget;
	
	// form data

		QString fName;
		QString fSurname;
		QString fNick;
		QString fCity;
		int	fUin;
		int	fGender;
		bool 	fOnlyOnline;
		int	fAgeFrom;
		int	fAgeTo;
};
#endif
