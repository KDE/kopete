/***************************************************************************
                          jabbercontact.h  -  description
                             -------------------
    begin                : Fri Apr 12 2002
    copyright            : (C) 2002 by Daniel Stone
    email                : dstone@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef JABBERCONTACT_H
#define JABBERCONTACT_H


#include "kopetecontact.h"

#include "jabtasks.h"

class KAction;
class KListAction;
class KPopupMenu;
class KSelectAction;

class dlgJabberRename;
class dlgJabberVCard;
class JabberProtocol;
class JabberResource;
class Jid;
class KopeteHistoryDialog;
class KopeteMessage;
class KopeteMessageManager;
class KopeteMetaContact;

class JabberContact : public KopeteContact
{
	Q_OBJECT
  
	public:

		JabberContact( QString userid, QString name, QString group,
				JabberProtocol *protocol, KopeteMetaContact *mc);

		void initContact(QString &userID, QString &name, QString &group);


		// Reimplementations of the (uninteresting)
		// members in KopeteContact
		virtual ContactStatus status() const;
		QString statusText() const;
		QString statusIcon() const;
		int importance() const;

		/*
		 * Pop up a chat window for this contact
		 */
		void execute();

		/*
		 * Return the currently used resource for this contact
		 */
		QString resource() const
		{
			return mResource;
		}
		
		/*
		 * Return the group this contact resides in
		 */
		QString group() const
		{
			return mGroup;
		}
		
		/*
		 * Return this contact's user ID
		 */
		QString userID() const
		{
			return mUserID;
		}
		
		/*
		 * 
		 */
		bool localGroup()
		{
			return hasLocalGroup;
		}

    virtual void showContextMenu(const QPoint&, const QString&);

    JabberResource *bestResource();

    virtual QString id() const;
    virtual QString data() const;

  public slots:
    void slotNewMessage(const JabMessage &);
    void slotCloseHistoryDialog();
    void slotViewHistory();
    void slotSendMsgKCW(const KopeteMessage&);
    void slotSendMsgKEW(const KopeteMessage&);
    void slotResourceAvailable(const Jid &, const JabResource &);
    void slotResourceUnavailable(const Jid &);
    void slotRemoveFromGroup();
    void slotSelectResource();
    void slotGotVCard(JT_VCard *);
    void slotUpdateContact(QString, int, QString);

  private slots:
    void slotDeleteMySelf(bool);
    void slotRemoveThisUser();
    void slotRenameContact();
    void slotDoRenameContact();
    void slotMoveThisUser();
    void slotChatThisUser();
    void slotEmailUser();
    void slotSnarfVCard();
	void slotUpdateNickname(const QString);

  signals:
//    void statusChanged();
    void msgRecieved(QString, QString, QString, QString, QFont, QColor);

  private:
    void initActions();

    JabberProtocol *mProtocol;
    JabberResource *activeResource;

    QPtrList<JabberResource> resources;
    QPtrList<KopeteContact> theContacts;

    bool hasLocalName, hasLocalGroup, hasResource;
    QString mUserID, mResource, mGroup, mReason;
    int mStatus;

    KPopupMenu *popup;
    KAction *actionMessage, *actionRemove, *actionRemoveFromGroup, *actionChat, *actionInfo, *actionHistory, *actionRename, *actionSnarfVCard;
    KListAction *actionContactMove;
    KSelectAction *actionSelectResource;

    dlgJabberRename *dlgRename;
    dlgJabberVCard *dlgVCard;
    KopeteMessageManager *mMsgManagerKCW, *mMsgManagerKEW;
    KopeteMessageManager *msgManagerKCW(), *msgManagerKEW();

    KopeteHistoryDialog *historyDialog;

};

class JabberResource : public QObject {
    Q_OBJECT
  public:
    JabberResource();
    JabberResource(const QString &, const int &, const QDateTime &, const int &, const QString &);
    ~JabberResource();

    QString resource() { return mResource; }
    int priority() { return mPriority; }
    QDateTime timestamp() { return mTimestamp; }
    int status() { return mStatus; }
    QString reason() { return mReason; }

  private:
    QString mResource, mReason;
    int mPriority, mStatus;
    QDateTime mTimestamp;
};
#endif
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

