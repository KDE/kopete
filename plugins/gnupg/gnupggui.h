#ifndef GNUPGGUI_H
#define GNUPGGUI_H

#include <qobject.h>
#include <kxmlguiclient.h>
#include <ktoggleaction.h>

namespace Kopete { class ChatSession; }


/**
 *@author Olivier Goffart
 *Add functionality to a chat window
 */
class GnupgGui : public QObject, public KXMLGUIClient
{
		Q_OBJECT
	public:
		explicit GnupgGui ( Kopete::ChatSession *parent = 0 );
		~GnupgGui();

		bool encrypting() { return m_encAction->isChecked(); }

		KToggleAction *m_encAction;


	private slots:
		void slotEncryptToggled();

};

#endif // GNUPGGUI_H