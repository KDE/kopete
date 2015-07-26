#ifndef GNUPGPREFERENCES_H
#define GNUPGPREFERENCES_H

#include <kcmodule.h>
#include <QTableView>
#include <QStandardItemModel>
#include <QListView>
#include <QPushButton>
#include <QVariantList>

class GnupgPreferences: public KCModule
{
    Q_OBJECT
public:
    explicit GnupgPreferences(QWidget *parent=0, const QVariantList &args = QVariantList() );
    virtual ~GnupgPreferences();
    virtual void save();
    virtual void load();
    virtual void defaults();
    void buttonsStatus();

private slots:
    void addPair();
    void remPair();

private:
    QTableView *resultsTable;
    QStandardItemModel *resultsModel;
    QStandardItemModel *accountsModel;
    QListView *accountsList;
    QListView *keysList;
    QStandardItemModel *keysModel;
    QPushButton *addCombination;
    QPushButton *remCombination;
};

#endif
