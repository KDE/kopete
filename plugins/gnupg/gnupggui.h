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

		bool signing() { return m_signAction->isChecked(); }
		bool encrypting() { return m_encAction->isChecked(); }

		KToggleAction *m_encAction;
		KToggleAction *m_signAction;
		KAction *m_exportAction;


	private slots:
		void slotEncryptToggled();
		void slotSignToggled();
		void slotExport();

};

#endif // GNUPGGUI_H