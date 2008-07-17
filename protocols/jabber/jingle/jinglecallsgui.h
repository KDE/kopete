#ifndef JINGLE_CALLS_GUI_H
#define JINGLE_CALLS_GUI_H
#include "ui_jinglecallsgui.h"

class JingleCallsManager;
class JingleCallsGui : public QMainWindow
{
	Q_OBJECT
public:
	JingleCallsGui(JingleCallsManager*);
	~JingleCallsGui();

private:
	void setupActions();
	JingleCallsManager *m_callsManager;
	Ui::jingleCallsGui ui;
};
#endif
