/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include "akonadihistoryplugin.h"
////
#include <KDialog>
#include <Akonadi/Item>

namespace Ui { class SearchDialog; }
class KJob ;
class QListWidgetItem;
class History;
//class QListView;
class QModelIndex;
class QStringListModel;

class SearchDialog : public KDialog
{
Q_OBJECT
public:
    SearchDialog(QWidget* parent = 0, Qt::WFlags flags = 0);
    ~SearchDialog();
     
    enum SearchType { SingleWordInChat , MultipleWordInChat , SingleWordContact , MultipleWordContact,
	Date , Exhaustive };
	
private:
    Ui::SearchDialog *m_MainWidget;
    Akonadi::Item::List m_items;
    QStringListModel *m_resultModel;
    bool m_chats, m_contacts , m_date , m_exhaustive ;
    QStringList m_searchStrings;
    SearchType m_searchType;
	
private slots:
    void slotSearchButtonClicked();
    void itemSearchJobDone(KJob*);
    void itemSelected(const QModelIndex&);
    void itemFetched(KJob*);
    
    void slotCBoxLogs(int);
    void slotCBoxContact(int);
    void slotCBoxExhaustive(int);
    void slotCBoxDate(int);
    void slotGetTags(KJob*);
    void slotLableSelected(int);
    
    void slotExhaustiveSearchDone(KJob*);
    
private:
    void displayResult(const History&);
    void reset();
    QString sparqlQuery( QString);
    
    QDate parseDate(QStringList);
    int findMonth(QString);
	
};

#endif // SEARCHDIALOG_H
