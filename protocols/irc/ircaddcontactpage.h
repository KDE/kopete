
#ifndef MSNADDCONTACTPAGE_H
#define MSNADDCONTACTPAGE_H

#include <qwidget.h>
#include <addcontactpage.h>

/**
  *@author duncan
  */
class ircAddUI;
class IRCProtocol;

class IRCAddContactPage : public AddContactPage
{
   Q_OBJECT
public:
	IRCAddContactPage(IRCProtocol *owner, QWidget *parent=0, const char *name=0);
	~IRCAddContactPage();
	ircAddUI *ircdata;
	IRCProtocol *plugin;
public slots: // Public slots
  /** No descriptions */
  virtual void slotFinish();
private slots:
	void connectNowClicked();
};


#endif
