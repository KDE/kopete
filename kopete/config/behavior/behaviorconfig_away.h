#ifndef __BEHAVIOURCONFIG_AWAY_H
#define __BEHAVIOURCONFIG_AWAY_H

#include "ui_behaviorconfig_away.h"

class BehaviorConfig_Away : public QWidget, public Ui::BehaviorConfig_Away
{
	Q_OBJECT

public:
	BehaviorConfig_Away(QWidget *parent = 0);
};
#endif
