#ifndef __APPEARANCECONFIG_CONTACTLIST_H
#define __APPEARANCECONFIG_CONTACTLIST_H

#include "ui_appearanceconfig_contactlist.h"

class AppearanceConfig_ContactList : public QWidget, public Ui::AppearanceConfig_ContactList
{
	Q_OBJECT

public:
	AppearanceConfig_ContactList(QWidget *parent = 0);
};
#endif
