
#ifndef AIMADDCONTACTPAGE_H
#define AIMADDCONTACTPAGE_H

#include <qwidget.h>
#include <qlabel.h>
#include "addcontactpage.h"

class aimAddContactUI;
class AIMAccount;
namespace Kopete
{
class Account; 
class MetaContact;
}

class AIMAddContactPage : public AddContactPage
{
Q_OBJECT

public:
	AIMAddContactPage(bool connected, QWidget *parent=0,
			    const char *name=0);
	~AIMAddContactPage();

	/** Validates the data entered */
	virtual bool validateData();
	/** Applies the addition to the account */
	virtual bool apply( Kopete::Account *account, Kopete::MetaContact *);

protected:
	/** The actual GUI */
	aimAddContactUI *m_gui;
	QLabel *noaddMsg1;
	QLabel *noaddMsg2;
	bool canadd;
};
#endif

//kate: tab-width 4; indent-mode csands;

