/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#ifndef NETWORKCONFIG_UI_H
#define NETWORKCONFIG_UI_H

void NetworkConfig::accept()
{
    emit accepted();
    QDialog::accept();
}

void NetworkConfig::reject()
{
    emit rejected();
    QDialog::reject();
}

#endif // NETWORKCONFIG_UI_H
