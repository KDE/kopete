#ifndef GADUPUBDIR_H
#define GADUPUBDIR_H

#include "gadusearch.h"
#include "gaduaccount.h"
#include "gaduprotocol.h"

#include <kdebug.h>
#include <qhbox.h>
#include <kdialogbase.h>

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

	private:
		GaduProtocol *p;
		GaduAccount *mAccount;
		GaduContact *mContact;
		GaduPublicDirectory *mMainWidget;
};
#endif
