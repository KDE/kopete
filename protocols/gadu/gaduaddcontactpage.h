#ifndef GADUADDCONTACTPAGE_H
#define GADUADDCONTACTPAGE_H

#include <qwidget.h>
#include <addcontactpage.h>

class GaduProtocol;
class gaduAddUI;
class QLabel;

class GaduAddContactPage : public AddContactPage
{
    Q_OBJECT
public:
    GaduAddContactPage( GaduProtocol *owner, QWidget *parent=0, const char *name=0 );
    ~GaduAddContactPage();
     virtual bool  validateData();
public slots:
    virtual void slotFinish(KopeteMetaContact *);
private:
    GaduProtocol *protocol_;
    gaduAddUI    *addUI_;
    bool          canAdd_;
    QLabel       *noaddMsg1_;
    QLabel       *noaddMsg2_;
};

#endif

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 *
 * vim: set et ts=4 sts=4 sw=4:
 */
