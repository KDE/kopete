
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
    Q_OBJECT

public:
	GaduEditAccount( GaduProtocol *proto, KopeteAccount *,
			    QWidget *parent=0, const char *name=0 );
	virtual bool validateData();
	KopeteAccount* apply();
	
public slots:
	void  registrationComplete( const QString&, const QString& );
	void  registrationError( const QString&, const QString& );
	
private:
	GaduProtocol *protocol_;
	bool reg_in_progress;
	
};

#endif

