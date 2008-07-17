/* 
 * This class is a dialog asking what contents the user accepts for an
 * incoming session.
 * This class could also be used to ask the user what contents he wants
 * to propose in the session-initiate jingle action
 */
#ifndef CONTENT_DIALOG_H
#define CONTENT_DIALOG_H
#include "ui_contentdialog.h"

#include "jingletasks.h"

#include <QCheckBox>

class ContentDialog : public QDialog
{
	Q_OBJECT
public:
	ContentDialog();
	~ContentDialog();
	void setContents(QList<XMPP::JingleContent*> c);
	void setSession(XMPP::JingleSession *s);
	XMPP::JingleSession *session();
	QStringList checked();
	QStringList unChecked();

private:
	Ui::contentDialog ui;
	XMPP::JingleSession *m_session;
	QList<QCheckBox*> m_checkBoxes;
	QStringList m_contentNames;
};

#endif
