
#ifndef ADDCONTACTPAGE_H
#define ADDCONTACTPAGE_H

#include <qwidget.h>
#include <improtocol.h>

/**
  *@author duncan
  */

class AddContactPage : public QWidget
{
   Q_OBJECT
public: 
	AddContactPage(QWidget *parent=0, const char *name=0);
	~AddContactPage();
	//IMProtocol *protocol;
public slots: // Public slots
  /** No descriptions */
  virtual void slotFinish();
};
#endif
