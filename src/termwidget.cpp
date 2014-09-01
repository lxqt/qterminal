#include <QMenu>
#include <QVBoxLayout>
#include <QPainter>
#include <QDesktopServices>

#include "termwidget.h"
#include "config.h"
#include "properties.h"

static int TermWidgetCount = 0;


TermWidgetImpl::TermWidgetImpl(const QString & wdir, const QString & shell, QWidget * parent)
    : QTermWidget(0, parent)
{
    TermWidgetCount++;
    QString name("TermWidget_%1");
    setObjectName(name.arg(TermWidgetCount));

    setFlowControlEnabled(FLOW_CONTROL_ENABLED);
    setFlowControlWarningEnabled(FLOW_CONTROL_WARNING_ENABLED);

    propertiesChanged();

    setHistorySize(5000);

    if (!wdir.isNull())
        setWorkingDirectory(wdir);

    if (shell.isNull())
    {
        if (!Properties::Instance()->shell.isNull())
            setShellProgram(Properties::Instance()->shell);
    }
    else
    {
        qDebug() << "Settings custom shell program:" << shell;
        QStringList parts = shell.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        qDebug() << parts;
        setShellProgram(parts.at(0));
        parts.removeAt(0);
        if (parts.count())
            setArgs(parts);
    }

    setMotionAfterPasting(Properties::Instance()->m_motionAfterPaste);
    
    actionMap[COPY_SELECTION] = new QAction(QIcon(":/icons/edit-copy.png"), tr(COPY_SELECTION), this);
    connect(actionMap[COPY_SELECTION], SIGNAL(triggered()), this, SLOT(copyClipboard()));
    addAction(actionMap[COPY_SELECTION]);

    actionMap[PASTE_CLIPBOARD] = new QAction(QIcon(":/icons/edit-paste.png"), tr(PASTE_CLIPBOARD), this);
    connect(actionMap[PASTE_CLIPBOARD], SIGNAL(triggered()), this, SLOT(pasteClipboard()));
    addAction(actionMap[PASTE_CLIPBOARD]);

    actionMap[PASTE_SELECTION] = new QAction(QIcon(":/icons/edit-paste.png"), tr(PASTE_SELECTION), this);
    connect(actionMap[PASTE_SELECTION], SIGNAL(triggered()), this, SLOT(pasteSelection()));
    addAction(actionMap[PASTE_SELECTION]);

    actionMap[ZOOM_IN] = new QAction(QIcon(":/icons/zoom-in.png"), tr(ZOOM_IN), this);
    connect(actionMap[ZOOM_IN], SIGNAL(triggered()), this, SLOT(zoomIn()));
    addAction(actionMap[ZOOM_IN]);

    actionMap[ZOOM_OUT] = new QAction(QIcon(":/icons/zoom-out.png"), tr(ZOOM_OUT), this);
    connect(actionMap[ZOOM_OUT], SIGNAL(triggered()), this, SLOT(zoomOut()));
    addAction(actionMap[ZOOM_OUT]);

    actionMap[ZOOM_RESET] = new QAction(QIcon(":/icons/zoom-out.png"), tr(ZOOM_RESET), this);
    connect(actionMap[ZOOM_RESET], SIGNAL(triggered()), this, SLOT(zoomReset()));
    addAction(actionMap[ZOOM_RESET]);

    QAction *act = new QAction(this);
    act->setSeparator(true);
    addAction(act);

    actionMap[CLEAR_TERMINAL] = new QAction(tr(CLEAR_TERMINAL), this);
    connect(actionMap[CLEAR_TERMINAL], SIGNAL(triggered()), this, SLOT(clear()));
    addAction(actionMap[CLEAR_TERMINAL]);

    actionMap[SPLIT_HORIZONTAL] = new QAction(tr(SPLIT_HORIZONTAL), this);
    connect(actionMap[SPLIT_HORIZONTAL], SIGNAL(triggered()), this, SLOT(act_splitHorizontal()));
    addAction(actionMap[SPLIT_HORIZONTAL]);

    actionMap[SPLIT_VERTICAL] = new QAction(tr(SPLIT_VERTICAL), this);
    connect(actionMap[SPLIT_VERTICAL], SIGNAL(triggered()), this, SLOT(act_splitVertical()));
    addAction(actionMap[SPLIT_VERTICAL]);

    actionMap[SUB_COLLAPSE] = new QAction(tr(SUB_COLLAPSE), this);
    connect(actionMap[SUB_COLLAPSE], SIGNAL(triggered()), this, SLOT(act_splitCollapse()));
    addAction(actionMap[SUB_COLLAPSE]);

    //act = new QAction(this);
    //act->setSeparator(true);
    //addAction(act);
    //
    //act = new QAction(tr("&Rename session..."), this);
    //act->setShortcut(Properties::Instance()->shortcuts[RENAME_SESSION]);
    //connect(act, SIGNAL(triggered()), this, SIGNAL(renameSession()));
    //addAction(act);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(customContextMenuCall(const QPoint &)));

    updateShortcuts();

    connect(this, SIGNAL(urlActivated(QUrl)), this, SLOT(activateUrl(const QUrl&)));
    //setKeyBindings("linux");
    startShellProgram();
}

void TermWidgetImpl::updateShortcuts()
{
    QSettings settings;
    settings.beginGroup("Shortcuts");

    QKeySequence seq;

    if( actionMap.contains(COPY_SELECTION) && settings.contains(COPY_SELECTION) )
    {
        seq = QKeySequence::fromString( settings.value(COPY_SELECTION, QKeySequence::Copy).toString() );
        actionMap[COPY_SELECTION]->setShortcut(seq);
    }
    if( actionMap.contains(PASTE_CLIPBOARD) && settings.contains(PASTE_CLIPBOARD) )
    {
        seq = QKeySequence::fromString( settings.value(PASTE_CLIPBOARD, QKeySequence::Paste).toString() );
        actionMap[PASTE_CLIPBOARD]->setShortcut(seq);
    } 
    if( actionMap.contains(PASTE_SELECTION) && settings.contains(PASTE_SELECTION) )
    {
        seq = QKeySequence::fromString( settings.value(PASTE_SELECTION, QKeySequence::Paste).toString() );
        actionMap[PASTE_SELECTION]->setShortcut(seq);
    }
    
    if( actionMap.contains(ZOOM_IN) && settings.contains(ZOOM_IN) )
    {
        seq = QKeySequence::fromString( settings.value(ZOOM_IN, QKeySequence::ZoomIn).toString() );
        actionMap[ZOOM_IN]->setShortcut(seq);
    }
    if( actionMap.contains(ZOOM_OUT) && settings.contains(ZOOM_OUT) )
    {
        seq = QKeySequence::fromString( settings.value(ZOOM_OUT, QKeySequence::ZoomOut).toString() );
        actionMap[ZOOM_OUT]->setShortcut(seq);
    }
    if( actionMap.contains(ZOOM_RESET) && settings.contains(ZOOM_RESET) )
    {
        seq = QKeySequence::fromString( settings.value(ZOOM_RESET).toString() );
        actionMap[ZOOM_RESET]->setShortcut(seq);
    }
    
    if( actionMap.contains(SPLIT_HORIZONTAL) && settings.contains(SPLIT_HORIZONTAL) )
    {
        seq = QKeySequence::fromString( settings.value(SPLIT_HORIZONTAL).toString() );
        actionMap[SPLIT_HORIZONTAL]->setShortcut(seq);
    }
    if( actionMap.contains(SPLIT_VERTICAL) && settings.contains(SPLIT_VERTICAL) )
    {
        seq = QKeySequence::fromString( settings.value(SPLIT_VERTICAL).toString() );
        actionMap[SPLIT_VERTICAL]->setShortcut(seq);
    }
    if( actionMap.contains(SUB_COLLAPSE) && settings.contains(SUB_COLLAPSE) )
    {
        seq = QKeySequence::fromString( settings.value(SUB_COLLAPSE).toString() );
        actionMap[SUB_COLLAPSE]->setShortcut(seq);
    }

    settings.endGroup();
}

void TermWidgetImpl::propertiesChanged()
{
    setColorScheme(Properties::Instance()->colorScheme);
    setTerminalFont(Properties::Instance()->font);
    setMotionAfterPasting(Properties::Instance()->m_motionAfterPaste);

    if (Properties::Instance()->historyLimited)
    {
        setHistorySize(Properties::Instance()->historyLimitedTo);
    }
    else
    {
        // Unlimited history
        setHistorySize(-1);
    }

    qDebug() << "TermWidgetImpl::propertiesChanged" << this << "emulation:" << Properties::Instance()->emulation;
    setKeyBindings(Properties::Instance()->emulation);
    setTerminalOpacity(Properties::Instance()->termOpacity/100.0);

    /* be consequent with qtermwidget.h here */
    switch(Properties::Instance()->scrollBarPos) {
    case 0:
        setScrollBarPosition(QTermWidget::NoScrollBar);
        break;
    case 1:
        setScrollBarPosition(QTermWidget::ScrollBarLeft);
        break;
    case 2:
    default:
        setScrollBarPosition(QTermWidget::ScrollBarRight);
        break;
    }

    updateShortcuts();

    update();
}

void TermWidgetImpl::customContextMenuCall(const QPoint & pos)
{
    QMenu menu;
    menu.addActions(actions());
/*
    menu.addSeparator();
    menu.addAction(QIcon(":/icons/close.png"),
        tr("Close session"), this,
        SIGNAL(removeCurrentSession()),
        Properties::Instance()->shortcuts[CLOSE_TAB]);
*/
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

void TermWidgetImpl::zoomIn()
{
    emit QTermWidget::zoomIn();
    Properties::Instance()->font = getTerminalFont();
    Properties::Instance()->saveSettings();
}

void TermWidgetImpl::zoomOut()
{
    emit QTermWidget::zoomOut();
    Properties::Instance()->font = getTerminalFont();
    Properties::Instance()->saveSettings();
}

void TermWidgetImpl::zoomReset()
{
    Properties::Instance()->font = Properties::Instance()->defaultFont();
    setTerminalFont(Properties::Instance()->font);
    Properties::Instance()->saveSettings();
}

void TermWidgetImpl::enableCollapse(bool enable)
{
    actionMap[SUB_COLLAPSE]->setEnabled(enable);
}

void TermWidgetImpl::activateUrl(const QUrl & url) {
    if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
        QDesktopServices::openUrl(url);
    }
}

TermWidget::TermWidget(const QString & wdir, const QString & shell, QWidget * parent)
    : QWidget(parent)
{
    m_border = palette().color(QPalette::Window);
    m_term = new TermWidgetImpl(wdir, shell, this);
    setFocusProxy(m_term);

    m_layout = new QVBoxLayout;
    setLayout(m_layout);

    m_layout->addWidget(m_term);

    propertiesChanged();

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

void TermWidget::propertiesChanged()
{
    if (Properties::Instance()->highlightCurrentTerminal)
        m_layout->setContentsMargins(2, 2, 2, 2);
    else
        m_layout->setContentsMargins(0, 0, 0, 0);

    m_term->propertiesChanged();
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
    m_border = palette().color(QPalette::Highlight);
    emit termGetFocus(this);
    update();
}

void TermWidget::term_termLostFocus()
{
    m_border = palette().color(QPalette::Window);
    update();
}

void TermWidget::paintEvent (QPaintEvent *)
{
    QPainter p(this);
    QPen pen(m_border);
    pen.setWidth(30);
    pen.setBrush(m_border);
    p.setPen(pen);
    p.drawRect(0, 0, width()-1, height()-1);
}
