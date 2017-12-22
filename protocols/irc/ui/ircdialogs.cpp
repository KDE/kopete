void IRCAccount::setNetwork(const QString &network)
{
/*
    IRCNetwork *net = m_protocol->networks()[ network ];
    if( net )
    {
        m_network = net;
        configGroup()->writeEntry(Config::NETWORKNAME, network);
        setAccountLabel(network);
    }
    else
    {
        KMessageBox::queuedMessageBox(
        UI::Global::mainWidget(), KMessageBox::Error,
        i18n("<qt>The network associated with this account, <b>%1</b>, no longer exists. Please"
        " ensure that the account has a valid network. The account will not be enabled until you do so.</qt>",network),
        i18n("Problem Loading %1", accountId() ), 0 );
    }
*/
}

void IRCAccount::slotNickInUseAlert(const QString &nick)
{
    KMessageBox::error(UI::Global::mainWidget(), i18n("The nickname %1 is already in use", nick), i18n("IRC Plugin"));
}

void IRCAccount::slotNickInUse(const QString &nick)
{
    QString altNickName = altNick();
    if (triedAltNick || altNickName.isEmpty()) {
        QString newNick = KInputDialog::getText(
            i18n("IRC Plugin"),
            i18n("The nickname %1 is already in use. Please enter an alternative nickname:", nick),
            nick);

        if (newNick.isNull()) {
            disconnect();
        } else {
            m_engine->nick(newNick);
        }
    } else {
        triedAltNick = true;
        m_engine->nick(altNickName);
    }
}

void IRCAccount::slotJoinChannel()
{
    if (!isConnected()) {
        return;
    }

    QStringList chans = configGroup()->readListEntry("Recent Channel list");
    //kDebug(14120) << "Recent channel list from config: " << chans;

    KLineEditDlg dlg(
        i18n("Please enter the name of the channel you want to join:"),
        QString(),
        UI::Global::mainWidget()
        );

    KCompletion comp;
    comp.insertItems(chans);

    dlg.lineEdit()->setCompletionObject(&comp);
    dlg.lineEdit()->setCompletionMode(KGlobalSettings::CompletionPopup);

    while (true)
    {
        if (dlg.exec() != QDialog::Accepted) {
            break;
        }

        QString chan = dlg.text();
        if (chan.isNull()) {
            break;
        }

        if (KIRC::Entity::isChannel(chan)) {
//			contactManager()->findChannel( chan )->startChat();

            // push the joined channel to first in list
            chans.remove(chan);
            chans.prepend(chan);

            configGroup()->writeEntry("Recent Channel list", chans);
            break;
        }

        KMessageBox::error(UI::Global::mainWidget(),
                           i18n("\"%1\" is an invalid channel name. Channels must start with '#', '!', '+', or '&'.", chan),
                           i18n("IRC Plugin")
                           );
    }
}

void IRCAccount::slotSearchChannels()
{
    if (!m_channelList) {
        m_channelList = new ChannelListDialog(m_engine,
                                              i18n("Channel List for %1", m_engine->currentHost()), this,
                                              SLOT(slotJoinNamedChannel(QString)));
    } else {
        m_channelList->clear();
    }

    m_channelList->show();
}

void IRCAccount::listChannels()
{
    slotSearchChannels();
    m_channelList->search();
}

void IRCAccount::slotServerBusy()
{
    KMessageBox::queuedMessageBox(
        UI::Global::mainWidget(), KMessageBox::Error,
        i18n("The IRC server is currently too busy to respond to this request."),
        i18n("Server is Busy"), 0
        );
}
