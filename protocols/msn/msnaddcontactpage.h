
#ifndef MSNADDCONTACTPAGE_H
#define MSNADDCONTACTPAGE_H

#include <qwidget.h>
#include <addcontactpage.h>
#include <qlabel.h>

/**
  *@author duncan
  */
class msnAddUI;
class MSNProtocol;

class MSNAddContactPage : public AddContactPage
{
   Q_OBJECT
public:
	MSNAddContactPage(MSNProtocol *owner, QWidget *parent=0, const char *name=0);
	~MSNAddContactPage();
	msnAddUI *msndata;
	MSNProtocol *plugin;
	QLabel *noaddMsg1;
	QLabel *noaddMsg2;
	bool canadd;
	virtual bool validateData();
public slots: // Public slots
  /** No descriptions */
  virtual void slotFinish();

};


#endif
