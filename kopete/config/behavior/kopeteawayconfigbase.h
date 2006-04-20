#ifndef __KOPETEAWAYCONFIGBASE_H
#define __KOPETEAWAYCONFIGBASE_H

#include "ui_kopeteawayconfigbase.h"

class KopeteAwayConfigBaseUI : public QWidget, public Ui::KopeteAwayConfigBaseUI
{
	Q_OBJECT

public:
	KopeteAwayConfigBaseUI(QWidget *parent = 0);
};
#endif
