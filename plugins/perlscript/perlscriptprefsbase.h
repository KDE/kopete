/****************************************************************************
** Form interface generated from reading ui file './perlscriptprefsbase.ui'
**
** Created: Thu Jan 30 16:42:49 2003
**      by: The User Interface Compiler ()
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef PERLSCRIPTPREFSUI_H
#define PERLSCRIPTPREFSUI_H

#include <qvariant.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class KListView;
class KPushButton;
namespace KTextEditor {
	class View;
	class Document;
}

class QListViewItem;

class PerlScriptPrefsUI : public QWidget
{
    Q_OBJECT

public:
    PerlScriptPrefsUI( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~PerlScriptPrefsUI();

    KPushButton* saveButton;
    KPushButton* addButton;
    KPushButton* removeButton;
    KListView* scriptView;
    KTextEditor::View* editArea;
    KTextEditor::Document* editDocument;

protected:

protected slots:
    virtual void languageChange();
};

#endif // PERLSCRIPTPREFSUI_H
