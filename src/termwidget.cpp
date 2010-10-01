#include "termwidget.h"
#include "config.h"
#include "properties.h"

static int TermWidgetCount = 0;


TermWidget::TermWidget(const QString & wdir, QWidget * parent)
    : QTermWidget(0, parent)
{
    TermWidgetCount++;
    QString name("TermWidget_%1");
    setObjectName(name.arg(TermWidgetCount));

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

    act = new QAction(tr("Split &Horizontally"), this);
    connect(act, SIGNAL(triggered()), this, SLOT(act_splitHorizontal()));
    addAction(act);
    act = new QAction(tr("Split &Vertically"), this);
    connect(act, SIGNAL(triggered()), this, SLOT(act_splitVertical()));
    addAction(act);
    actCollapse = new QAction(tr("Close Terminal Sub-Window"), this);
    connect(actCollapse, SIGNAL(triggered()), this, SLOT(act_splitCollapse()));
    addAction(actCollapse);

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
    menu.addAction(QIcon(":/icons/close.png"), tr("Close session"), this, SIGNAL(removeCurrentSession()), Properties::Instance()->shortcuts[CLOSE_TAB]);
    
    menu.exec(mapToGlobal(pos));
}

void TermWidget::act_splitVertical()
{
    emit splitVertical(this);
}

void TermWidget::act_splitHorizontal()
{
    emit splitHorizontal(this);
}

void TermWidget::act_splitCollapse()
{
    emit splitCollapse(this);
}

void TermWidget::enableCollapse(bool enable)
{
    actCollapse->setEnabled(enable);
}

