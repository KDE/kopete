
#ifndef MSNADDCONTACTPAGE_H
#define MSNADDCONTACTPAGE_H

#include <qwidget.h>
#include <addcontactpage.h>
#include <qlabel.h>

/**
  *@author duncan
  */
namespace Ui { class msnAddUI; }

class MSNAddContactPage : public AddContactPage
{
   Q_OBJECT
public:
	explicit MSNAddContactPage(bool connected, QWidget *parent=0);
	~MSNAddContactPage();
	Ui::msnAddUI *msndata;
	QLabel *noaddMsg1;
	QLabel *noaddMsg2;
	bool canadd;
	virtual bool validateData();
	virtual bool apply( Kopete::Account*, Kopete::MetaContact* );

};

#endif

// vim: set noet ts=4 sts=4 sw=4:

