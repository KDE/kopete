
#ifndef AIMADDCONTACTPAGE_H
#define AIMADDCONTACTPAGE_H

#include <qwidget.h>
#include <qlabel.h>
#include "addcontactpage.h"

/**
  *@author duncan
  */
class aimAddContactUI;
class AIMAccount;
class KopeteAccount;
class KopeteMetaContact;

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
	virtual bool apply( KopeteAccount *account, KopeteMetaContact *);

protected:
	/** The actual GUI */
	aimAddContactUI *m_gui;
	QLabel *noaddMsg1;
	QLabel *noaddMsg2;
	bool canadd;
};
#endif
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

