
#ifndef JABBERADDCONTACTPAGE_H
#define JABBERADDCONTACTPAGE_H

#include <qwidget.h>
#include <addcontactpage.h>
#include <qlabel.h>
#include "jabberprotocol.h"
#include "dlgaddcontact.h"

/**
  *@author Daniel Stone
  */
class dlgAddContact;
class JabberProtocol;

class JabberAddContactPage:public AddContactPage {
  Q_OBJECT public:
    JabberAddContactPage(JabberProtocol * owner, QWidget * parent =
			 0, const char *name = 0);
    ~JabberAddContactPage();
    virtual bool validateData();
    dlgAddContact *jabData;
    JabberProtocol *plugin;
    QLabel *noaddMsg1;
    QLabel *noaddMsg2;
    bool canadd;
    public slots:		// Public slots
  /** No descriptions */
     virtual void slotFinish(KopeteMetaContact *mc);

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

