
#ifndef GADUEDITACCOUNT_H
#define GADUEDITACCOUNT_H

#include "gadueditaccountui.h"
#include "editaccountwidget.h"

class GaduAccount;
class GaduProtocol;
class KopeteAccount;

class GaduEditAccount : public GaduAccountEditUI,
                        public EditAccountWidget
{
public:
	GaduEditAccount( GaduProtocol *proto, KopeteAccount *,
									QWidget *parent=0, const char *name=0 );
	bool validateData();
	KopeteAccount* apply();
private:
	GaduAccount  *account_;
	GaduProtocol *protocol_;
};

#endif
