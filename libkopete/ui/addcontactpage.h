
#ifndef ADDCONTACTPAGE_H
#define ADDCONTACTPAGE_H

#include <qwidget.h>
#include <kopeteprotocol.h>

/**
  *@author duncan
  */

class AddContactPage : public QWidget
{
   Q_OBJECT
public: 
	AddContactPage(QWidget *parent=0, const char *name=0);
	~AddContactPage();
	//KopeteProtocol *protocol;
	virtual bool validateData();
public slots: // Public slots
  /** No descriptions */
  virtual void slotFinish();
};
#endif
