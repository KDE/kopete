/****************************************************************************
** Form interface generated from reading ui file './addscriptdialog.ui'
**
** Created: Thu Jan 30 20:01:58 2003
**      by: The User Interface Compiler ()
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef ADDSCRIPTDIALOG_H
#define ADDSCRIPTDIALOG_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class KLineEdit;
class KURLRequester;
class QLabel;
class QPushButton;

class AddScriptDialog : public QDialog
{
    Q_OBJECT

public:
    AddScriptDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~AddScriptDialog();

    QPushButton* buttonOk;
    QPushButton* buttonCancel;
    KLineEdit* kLineEdit2;
    KLineEdit* kLineEdit1;
    QLabel* textLabel1;
    QLabel* textLabel2;
    QLabel* textLabel3;
    KURLRequester* kURLRequester1;

signals:
    void scriptChosen(const QString &, const QString &, const QString &);
    
private slots:
    void accept();
    
protected:
    QGridLayout* AddScriptDialogLayout;
    QHBoxLayout* Layout1;

protected slots:
    virtual void languageChange();
};

#endif // ADDSCRIPTDIALOG_H
