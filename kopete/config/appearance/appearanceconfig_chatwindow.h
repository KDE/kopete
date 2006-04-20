#ifndef __APPEARANCECONFIG_CHATWINDOW_H
#define __APPEARANCECONFIG_CHATWINDOW_H

#include "ui_appearanceconfig_chatwindow.h"

class AppearanceConfig_ChatWindow : public QWidget, public Ui::AppearanceConfig_ChatWindow
{
	Q_OBJECT

public:
	AppearanceConfig_ChatWindow(QWidget *parent = 0);
};
#endif
