/*************************************************************************
 * Copyright <2007 - 2013>  <Michael Zanetti> <mzanetti@kde.org>         *
 *                                                                       *
 * This program is free software; you can redistribute it and/or         *
 * modify it under the terms of the GNU General Public License as        *
 * published by the Free Software Foundation; either version 2 of        *
 * the License or (at your option) version 3 or any later version        *
 * accepted by the membership of KDE e.V. (or its successor approved     *
 * by the membership of KDE e.V.), which shall act as a proxy            *
 * defined in Section 14 of version 3 of the license.                    *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/

#include "otrguiclient.h"
#include "otrplugin.h"
#include "otrlchatinterface.h"

#include <QAction>
#include <KLocalizedString>
#include <kactionmenu.h>
#include <kopetechatsession.h>
#include <ui/kopeteview.h>
#include <kopetemessage.h>
#include "plugin_otr_debug.h"

#include <kmessagebox.h>
#include <qicon.h>
#include <kactioncollection.h>

/**
  * @author Frank Scheffold
  * @author Michael Zanetti
  */

OtrGUIClient::OtrGUIClient(Kopete::ChatSession *parent)
    : QObject(parent)
    , KXMLGUIClient(parent)
{
    setComponentName(QStringLiteral("kopete_otr"), i18n("Kopete"));
    connect(OTRPlugin::plugin(),
            SIGNAL(destroyed(QObject*)), this,
            SLOT(deleteLater())

            );

    connect(this, SIGNAL(signalOtrChatsession(Kopete::ChatSession*,bool)), OTRPlugin::plugin(), SLOT(slotEnableOtr(Kopete::ChatSession*,bool)));

    connect(OtrlChatInterface::self(), SIGNAL(goneSecure(Kopete::ChatSession*,int)),
            this, SLOT(encryptionEnabled(Kopete::ChatSession*,int)));

    connect(this, SIGNAL(signalVerifyFingerprint(Kopete::ChatSession*)), OTRPlugin::plugin(), SLOT(slotVerifyFingerprint(Kopete::ChatSession*)));

    m_manager = parent;

    otrActionMenu = new KActionMenu(QIcon::fromTheme(QStringLiteral("document-decrypt")), i18n("OTR Encryption"), actionCollection());
    otrActionMenu->setDelayed(false);
    actionCollection()->addAction(QStringLiteral("otr_settings"), otrActionMenu);

    actionEnableOtr = new QAction(QIcon::fromTheme(QStringLiteral("object-locked")), i18n("Start OTR Session"), this);
    actionCollection()->addAction(QStringLiteral("enableOtr"), actionEnableOtr);
    connect(actionEnableOtr, SIGNAL(triggered(bool)), this, SLOT(slotEnableOtr()));

    actionDisableOtr = new QAction(QIcon::fromTheme(QStringLiteral("object-unlocked")), i18n("End OTR Session"), this);
    actionCollection()->addAction(QStringLiteral("disableOtr"), actionDisableOtr);
    connect(actionDisableOtr, SIGNAL(triggered(bool)), this, SLOT(slotDisableOtr()));

    actionVerifyFingerprint = new QAction(QIcon::fromTheme(QStringLiteral("application-pgp-signature")), i18n("Authenticate Contact"), this);
    actionCollection()->addAction(QStringLiteral("verifyFingerprint"), actionVerifyFingerprint);
    connect(actionVerifyFingerprint, SIGNAL(triggered(bool)), this, SLOT(slotVerifyFingerprint()));

    // jpetso says: please request an icon named "document-verify" or something like that, the "sign" icon is not really appropriate for this purpose imho
    // mzanetti says: the "document-sign" icon is the same as kgpg uses to sign fingerprints. Anyways I will discuss that on #kopete. Re-using "document-sign for now.

    otrActionMenu->addAction(actionEnableOtr);
    otrActionMenu->addAction(actionDisableOtr);
    otrActionMenu->addAction(actionVerifyFingerprint);

    setXMLFile(QStringLiteral("otrchatui.rc"));

    encryptionEnabled(parent, OtrlChatInterface::self()->privState(parent));
}

OtrGUIClient::~OtrGUIClient()
{
}

void OtrGUIClient::slotEnableOtr()
{
    emit signalOtrChatsession(m_manager, true);
}

void OtrGUIClient::slotDisableOtr()
{
    emit signalOtrChatsession(m_manager, false);
}

void OtrGUIClient::slotVerifyFingerprint()
{
    emit signalVerifyFingerprint(m_manager);
}

void OtrGUIClient::encryptionEnabled(Kopete::ChatSession *session, int state)
{
    qCDebug(KOPETE_PLUGIN_OTR_LOG) << "OTRGUIClient switched security state to: " << state;
    if (session == m_manager) {
        switch (state) {
        case 0:
            otrActionMenu->setIcon(QIcon::fromTheme(QStringLiteral("object-unlocked")));
            actionEnableOtr->setText(i18n("Start OTR Session"));
            actionDisableOtr->setEnabled(false);
            actionVerifyFingerprint->setEnabled(false);
            break;
        case 1:
            otrActionMenu->setIcon(QIcon::fromTheme(QStringLiteral("object-locked-unverified")));
            actionEnableOtr->setText(i18n("Refresh OTR Session"));
            actionDisableOtr->setEnabled(true);
            actionVerifyFingerprint->setEnabled(true);
            break;
        case 2:
            otrActionMenu->setIcon(QIcon::fromTheme(QStringLiteral("object-locked-verified")));
            actionEnableOtr->setText(i18n("Refresh OTR Session"));
            actionDisableOtr->setEnabled(true);
            actionVerifyFingerprint->setEnabled(true);
            break;
        case 3:
            otrActionMenu->setIcon(QIcon::fromTheme(QStringLiteral("object-locked-finished")));
            actionEnableOtr->setText(i18n("Start OTR Session"));
            actionDisableOtr->setEnabled(true);
            actionVerifyFingerprint->setEnabled(false);
            break;
        }
    }
}

// vim: set noet ts=4 sts=4 sw=4:
