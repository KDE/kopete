/****************************************************************************
** Form interface generated from reading ui file './icqinfobase.ui'
**
** Created: Sat May 25 23:04:53 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef DLGJABBERVCARD_H
#define DLGJABBERVCARD_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class KURLLabel;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;

class dlgJabberVCard : public QDialog
{ 
    Q_OBJECT

public:
    dlgJabberVCard( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~dlgJabberVCard();

    QGroupBox* GroupBox1;
    QLabel* TextLabel2;
    QLineEdit* nickNameLE;
    QLabel* TextLabel3;
    QLabel* JabberIDLabel;
    KURLLabel* homepageLabel;
    QLabel* TextLabel29;
    QLabel* ageLabel;
    QLabel* TextLabel27;
    QLabel* genderLabel;
    QLabel* TextLabel26;
    QLabel* TextLabel31;
    QLabel* birthdayLabel;
    QLabel* firstNameLabel;
    QLabel* TextLabel9;
    QLabel* TextLabel7;
    QLabel* lastNameLabel;
    QPushButton* cmdClose;
    QPushButton* cmdSave;
    QGroupBox* GroupBox3;
    QLabel* TextLabel18;
    QLabel* addressLabel;
    QLabel* TextLabel16;
    QLabel* cellularPhoneNumber;
    QLabel* TextLabel14;
    QLabel* phoneNumberLabel;
    QLabel* TextLabel12;
    KURLLabel* emailAddressLabel;
    QLabel* cityLabel;
    QLabel* TextLabel22;
    QLabel* TextLabel20;
    QLabel* countryLabel;
    QLabel* TextLabel24;
    QLabel* stateLabel;


protected:
    QGridLayout* dlgJabberVCardLayout;
    QGridLayout* GroupBox1Layout;
    QGridLayout* GroupBox3Layout;
    QGridLayout* Layout10;
    QGridLayout* Layout3;
    QGridLayout* Layout5;
    QGridLayout* Layout1;
};

#endif // DLGJABBERVCARD_H
