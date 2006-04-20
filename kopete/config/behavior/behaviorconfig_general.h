#ifndef __BEHAVIOURCONFIG_GENERAL_H
#define __BEHAVIOURCONFIG_GENERAL_H

#include "ui_behaviorconfig_general.h"

class BehaviorConfig_General : public QWidget, public Ui::BehaviorConfig_General
{
	Q_OBJECT

public:
	BehaviorConfig_General(QWidget *parent = 0);
};
#endif
