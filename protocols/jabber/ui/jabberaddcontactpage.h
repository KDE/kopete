
#ifndef JABBERADDCONTACTPAGE_H
#define JABBERADDCONTACTPAGE_H

#include <qwidget.h>
#include <addcontactpage.h>
#include <qlabel.h>
#include "jabberaccount.h"
#include "dlgaddcontact.h"

/**
  *@author Daniel Stone
  */
class dlgAddContact;
class JabberAccount;

class JabberAddContactPage:public AddContactPage
{
  Q_OBJECT public:
	  JabberAddContactPage (KopeteAccount * owner, QWidget * parent = 0, const char *name = 0);
	 ~JabberAddContactPage ();
	virtual bool validateData ();
	virtual bool apply (KopeteAccount *, KopeteMetaContact *);
	dlgAddContact *jabData;
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
