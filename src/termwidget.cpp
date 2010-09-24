#include "termwidget.h"
#include "config.h"
#include "properties.h"


TermWidget::TermWidget(const QString & wdir, QWidget * parent)
    : QTermWidget(0, parent)
{

    setFlowControlEnabled(FLOW_CONTROL_ENABLED);
    setFlowControlWarningEnabled(FLOW_CONTROL_WARNING_ENABLED);
    
    propertiesChanged();

    setHistorySize(5000);
    setScrollBarPosition(QTermWidget::ScrollBarRight);
    
    if (!wdir.isNull())
        setWorkingDirectory(wdir);
	
    if (!Properties::Instance()->shell.isNull())
        setShellProgram(Properties::Instance()->shell);

    QAction* act = new QAction(QIcon(":/icons/editcopy.png"), tr("&Copy selection"), this);
    act->setShortcuts(QList<QKeySequence>() << Properties::Instance()->shortcuts[COPY_SELECTION]);
    connect(act, SIGNAL(triggered()), this, SLOT(copyClipboard()));
    addAction(act);

    act = new QAction(QIcon(":/icons/editpaste.png"), tr("&Paste Selection"), this);
    act->setShortcuts(QList<QKeySequence>() << Properties::Instance()->shortcuts[PASTE_SELECTION]);
    connect(act, SIGNAL(triggered()), this, SLOT(pasteClipboard()));
    addAction(act);

    act = new QAction(this);
    act->setSeparator(true);
    addAction(act);

    act = new QAction(tr("&Rename session..."), this);
    act->setShortcut(Properties::Instance()->shortcuts[RENAME_SESSION]);
    connect(act, SIGNAL(triggered()), this, SIGNAL(renameSession()));
    addAction(act);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(customContextMenuCall(const QPoint &)));

    //setKeyBindings("linux");
    startShellProgram();
}

void TermWidget::propertiesChanged()
{
    setColorScheme(Properties::Instance()->colorScheme);
    setTerminalFont(Properties::Instance()->font);
    qDebug() << "TermWidget::propertiesChanged" << this << "emulation:" << Properties::Instance()->emulation;
    setKeyBindings(Properties::Instance()->emulation);
    update();
}

void TermWidget::customContextMenuCall(const QPoint & pos)
{
    QMenu menu;
    menu.addActions(actions());
    menu.addSeparator();
    menu.addAction(QIcon(":/icons/close.png"), tr("Close session"), this, SIGNAL(removeCurrentTerminal()), Properties::Instance()->shortcuts[CLOSE_TAB]);
    
    menu.exec(mapToGlobal(pos));
}

