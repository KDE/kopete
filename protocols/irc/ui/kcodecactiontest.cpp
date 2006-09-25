#include <QToolBar>
#include <QTextCodec>

#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kmenubar.h>
#include <kdebug.h>

#include "kcodecactiontest.h"

#include <kcodecaction.h>

int main( int argc, char **argv )
{
    KCmdLineArgs::init( argc, argv, "kcodecactiontest", "KCodecActionTest", "kselectaction test app", "1.0" );
    KApplication app;

    CodecActionTest* test = new CodecActionTest;
    test->show();

    return app.exec();
}

CodecActionTest::CodecActionTest(QWidget *parent)
    : KMainWindow(parent)
    , m_comboCodec(new KCodecAction("Combo Codecion", actionCollection(), "combo"))
    , m_buttonCodec(new KCodecAction("Button Codecion", actionCollection(), "button"))
{
    m_comboCodec->setToolBarMode(KCodecAction::ComboBoxMode);
    connect(m_comboCodec, SIGNAL(triggered(QAction*)), SLOT(triggered(QAction*)));
    connect(m_comboCodec, SIGNAL(triggered(int)), SLOT(triggered(int)));
    connect(m_comboCodec, SIGNAL(triggered(const QString&)), SLOT(triggered(const QString&)));
    connect(m_comboCodec, SIGNAL(triggered(QTextCodec *)), SLOT(triggered(QTextCodec *)));

    m_buttonCodec->setToolBarMode(KCodecAction::MenuMode);
    connect(m_buttonCodec, SIGNAL(triggered(QAction*)), SLOT(triggered(QAction*)));
    connect(m_buttonCodec, SIGNAL(triggered(int)), SLOT(triggered(int)));
    connect(m_buttonCodec, SIGNAL(triggered(const QString&)), SLOT(triggered(const QString&)));

    menuBar()->addAction(m_comboCodec);
    menuBar()->addAction(m_buttonCodec);
    menuBar()->addAction("Add an action", this, SLOT(addAction()));
    menuBar()->addAction("Remove an action", this, SLOT(removeAction()));

    QToolBar* toolBar = addToolBar("Test");
    toolBar->addAction(m_comboCodec);
    toolBar->addAction(m_buttonCodec);
}

void CodecActionTest::triggered(QAction* action)
{
  kDebug() << k_funcinfo << action << endl;
}

void CodecActionTest::triggered(int index)
{
  kDebug() << k_funcinfo << index << endl;
}

void CodecActionTest::triggered(const QString& text)
{
  kDebug() << k_funcinfo << '"' << text << '"' << endl;
}

void CodecActionTest::triggered(QTextCodec *codec)
{
  kDebug() << k_funcinfo << '"' << codec->name() << '"' << endl;
}

void CodecActionTest::addAction()
{
    QAction* action = m_comboCodec->addAction(QString ("Combo Action %1").arg(m_comboCodec->actions().count()));
    connect(action, SIGNAL(triggered(bool)), SLOT(slotActionTriggered(bool)));
    action = m_buttonCodec->addAction(QString ("Action %1").arg(m_buttonCodec->actions().count()));
    connect(action, SIGNAL(triggered(bool)), SLOT(slotActionTriggered(bool)));
}

void CodecActionTest::removeAction()
{
    if (!m_comboCodec->actions().isEmpty())
        m_comboCodec->removeAction(m_comboCodec->actions().last());

    if (!m_buttonCodec->actions().isEmpty())
        m_buttonCodec->removeAction(m_buttonCodec->actions().last());
}

void CodecActionTest::slotActionTriggered(bool state)
{
    kDebug() << k_funcinfo << sender() << " state " << state << endl;
}

#include "kcodecactiontest.moc"

