#include "termwidget.h"
#include "config.h"
#include "properties.h"

static int TermWidgetCount = 0;


TermWidgetImpl::TermWidgetImpl(const QString & wdir, QWidget * parent)
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

void TermWidgetImpl::propertiesChanged()
{
    setColorScheme(Properties::Instance()->colorScheme);
    setTerminalFont(Properties::Instance()->font);
    qDebug() << "TermWidgetImpl::propertiesChanged" << this << "emulation:" << Properties::Instance()->emulation;
    setKeyBindings(Properties::Instance()->emulation);
    update();
}

void TermWidgetImpl::customContextMenuCall(const QPoint & pos)
{
    QMenu menu;
    menu.addActions(actions());
    menu.addSeparator();
    menu.addAction(QIcon(":/icons/close.png"), tr("Close session"), this, SIGNAL(removeCurrentSession()), Properties::Instance()->shortcuts[CLOSE_TAB]);
    
    menu.exec(mapToGlobal(pos));
}

void TermWidgetImpl::act_splitVertical()
{
    emit splitVertical();
}

void TermWidgetImpl::act_splitHorizontal()
{
    emit splitHorizontal();
}

void TermWidgetImpl::act_splitCollapse()
{
    emit splitCollapse();
}

void TermWidgetImpl::enableCollapse(bool enable)
{
    actCollapse->setEnabled(enable);
}



TermWidget::TermWidget(const QString & wdir, QWidget * parent)
    : QWidget(parent)
{
    m_border = palette().color(QPalette::Window);
    m_term = new TermWidgetImpl(wdir, this);
    setFocusProxy(m_term);
    m_layout = new QVBoxLayout;
    m_layout->setContentsMargins(3, 3, 3, 3);
    setLayout(m_layout);
    
    m_layout->addWidget(m_term);
    
    connect(m_term, SIGNAL(finished()), this, SIGNAL(finished()));
    connect(m_term, SIGNAL(splitHorizontal()),
            this, SLOT(term_splitHorizontal()));
    connect(m_term, SIGNAL(splitVertical()),
            this, SLOT(term_splitVertical()));
    connect(m_term, SIGNAL(splitCollapse()),
            this, SLOT(term_splitCollapse()));
    connect(m_term, SIGNAL(termGetFocus()), this, SLOT(term_termGetFocus()));
    connect(m_term, SIGNAL(termLostFocus()), this, SLOT(term_termLostFocus()));
}

void TermWidget::enableCollapse(bool enable)
{
    m_term->enableCollapse(enable);
}

void TermWidget::term_splitHorizontal()
{
    emit splitHorizontal(this);
}

void TermWidget::term_splitVertical()
{
    emit splitVertical(this);
}

void TermWidget::term_splitCollapse()
{
    emit splitCollapse(this);
}

void TermWidget::term_termGetFocus()
{
    qDebug() << "get focus" << this << this->size();
    m_border = palette().color(QPalette::Highlight);
    update();
}

void TermWidget::term_termLostFocus()
{
    qDebug() << "lost focus" << this;
    m_border = palette().color(QPalette::Window);
    update();
}

void TermWidget::paintEvent (QPaintEvent * event)
{
    qDebug() << "paintEvent";
    QPainter p(this);
    QPen pen(m_border);
    pen.setWidth(30);
    pen.setBrush(m_border);
    p.setPen(pen);
    p.drawRect(0, 0, width()-1, height()-1);
}
