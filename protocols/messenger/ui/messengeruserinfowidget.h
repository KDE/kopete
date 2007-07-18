#ifndef _MESSENGERUSERINFOWIDGET_H_
#define _MESSENGERUSERINFOWIDGET_H_

#include <kpagedialog.h>

class MessengerUserInfoWidget : public KPageDialog
{
Q_OBJECT
public:
	MessengerUserInfoWidget( QWidget* parent = 0);
	~MessengerUserInfoWidget();

private:
	Ui::MessengerGeneralInfoWidget* m_genInfoWidget;
	Ui::MessengerContactInfoWidget* m_contactInfoWidget;
	Ui::MessengerPersonalInfoWidget* m_personalInfoWidget;
	Ui::MessengerWorkInfoWidget* m_workInfoWidget;
	Ui::MessengerNotesInfoWidget* m_notesInfoWidget;

private slot:
	void slotUpdateNickName();
}

#endif
