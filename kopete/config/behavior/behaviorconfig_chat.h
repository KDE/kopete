#ifndef __BEHAVIOURCONFIG_CHAT_H
#define __BEHAVIOURCONFIG_CHAT_H

#include "ui_behaviorconfig_chat.h"

class BehaviorConfig_Chat : public QWidget, public Ui::BehaviorConfig_Chat
{
	Q_OBJECT

public:
	BehaviorConfig_Chat(QWidget *parent = 0);
};
#endif
