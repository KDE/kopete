#include "jinglecallsgui.h"
#include "jinglecallsmanager.h"

JingleCallsGui::JingleCallsGui(JingleCallsManager* parent)
: m_callsManager(parent)
{
	ui.setupUi(this);
	setupActions();
}

JingleCallsGui::~JingleCallsGui()
{

}

void JingleCallsGui::setupActions()
{

}
