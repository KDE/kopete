#ifndef __BEHAVIOURCONFIG_EVENTS_H
#define __BEHAVIOURCONFIG_EVENTS_H

#include "ui_behaviorconfig_events.h"

class BehaviorConfig_Events : public QWidget, public Ui::BehaviorConfig_Events
{
	Q_OBJECT

public:
	BehaviorConfig_Events(QWidget *parent = 0);
};
#endif
