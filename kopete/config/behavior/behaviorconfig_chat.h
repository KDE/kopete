/****************************************************************************
** Form interface generated from reading ui file './behaviorconfig_chat.ui'
**
** Created: Dom Mai 8 08:12:20 2005
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.4   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef BEHAVIORCONFIG_CHAT_H
#define BEHAVIORCONFIG_CHAT_H

#include <qvariant.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QButtonGroup;
class QComboBox;
class QLabel;
class QRadioButton;
class QCheckBox;
class QSpinBox;

class BehaviorConfig_Chat : public QWidget
{
    Q_OBJECT

public:
    BehaviorConfig_Chat( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~BehaviorConfig_Chat();

    QButtonGroup* interfaceGroup;
    QComboBox* viewPlugin;
    QLabel* viewPluginLabel;
    QButtonGroup* chatWindowGroup;
    QRadioButton* mNewWindow;
    QRadioButton* mTabAccountWindow;
    QRadioButton* mTabWindow;
    QRadioButton* mTabGroup;
    QRadioButton* mTabMetaContact;
    QCheckBox* highlightEnabled;
    QCheckBox* cb_ShowEventsChk;
    QCheckBox* cb_RaiseMsgWindowChk;
    QCheckBox* truncateContactNameEnabled;
    QSpinBox* mMaxContactNameLength;
    QLabel* txtChatViewBufferSize;
    QSpinBox* mChatViewBufferSize;

protected:
    QGridLayout* BehaviorConfig_ChatLayout;
    QSpacerItem* spacer2;
    QHBoxLayout* interfaceGroupLayout;
    QSpacerItem* spacer5;
    QVBoxLayout* chatWindowGroupLayout;
    QHBoxLayout* layout3;
    QSpacerItem* spacer3;
    QHBoxLayout* layout4;
    QSpacerItem* spacer6;

protected slots:
    virtual void languageChange();

};

#endif // BEHAVIORCONFIG_CHAT_H
