#define PsiAccount JabberAccount
class PsiAccount;

#ifndef VOICECALLER_H
#define VOICECALLER_H

#include "im.h"




using namespace XMPP;

/**
 * \brief An abstract class for a voice call implementation.
 */
class VoiceCaller : public QObject
{
	Q_OBJECT

public:
	/**
	 * \brief Base constructor.
	 * 
	 * \param account the account to which this voice caller belongs
	 */
	VoiceCaller(PsiAccount* account) : account_(account) { };
	
	/**
	 * \brief Retrieves the account to which this voice caller belongs.
	 */
	PsiAccount* account() { return account_; }

	/**
	 * \brief Initializes the voice caller. 
	 * This should be called when the connection is open.
	 */
	virtual void initialize() = 0;

	/**
	 * \brief De-initializes the voice caller. 
	 * This should be called when the connection is about to be closed.
	 */
	virtual void deinitialize() = 0;

	/**
	 * \brief Call the given JID.
	 */
	virtual void call(const Jid&) = 0;

	/**
	 * \brief Accept a call from the given JID.
	 */
	virtual void accept(const Jid&) = 0;

	/**
	 * \brief Reject the call from the given JID.
	 */
	virtual void reject(const Jid&) = 0;
	
	/**
	 * \brief Terminate the call from the given JID.
	 */
	virtual void terminate(const Jid&) = 0;

signals:
	/**
	 * \brief Incoming call from the given JID.
	 */
	void incoming(const Jid&);
	
	/**
	 * \brief Contact accepted an incoming call.
	 */
	void accepted(const Jid&);

	/**
	 * \brief Contact rejected an incoming call.
	 */
	void rejected(const Jid&);

	/**
	 * \brief Call with given JID is in progress.
	 */
	void in_progress(const Jid&);

	/**
	 * \brief Call with given JID is terminated.
	 */
	void terminated(const Jid&);

private:
	PsiAccount* account_;
};

#endif
