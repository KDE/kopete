#include "ui_contentdialog.h"

#include "jingletasks.h"

#include <QCheckBox>

class ContentDialog : public QDialog
{
	Q_OBJECT
public:
	ContentDialog();
	~ContentDialog();
	void setContents(QList<XMPP::JingleContent> c);
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
