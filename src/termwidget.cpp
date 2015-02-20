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

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(customContextMenuCall(const QPoint &)));

    connect(this, SIGNAL(urlActivated(QUrl)), this, SLOT(activateUrl(const QUrl&)));

    startShellProgram();
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

    update();
}

void TermWidgetImpl::customContextMenuCall(const QPoint & pos)
{
    QMenu menu;
    menu.addAction(Properties::Instance()->actions[COPY_SELECTION]);
    menu.addAction(Properties::Instance()->actions[PASTE_CLIPBOARD]);
    menu.addAction(Properties::Instance()->actions[PASTE_SELECTION]);
    menu.addAction(Properties::Instance()->actions[ZOOM_IN]);
    menu.addAction(Properties::Instance()->actions[ZOOM_OUT]);
    menu.addAction(Properties::Instance()->actions[ZOOM_RESET]);
    menu.addSeparator();
    menu.addAction(Properties::Instance()->actions[CLEAR_TERMINAL]);
    menu.addAction(Properties::Instance()->actions[SPLIT_HORIZONTAL]);
    menu.addAction(Properties::Instance()->actions[SPLIT_VERTICAL]);
#warning TODO/FIXME: disable the action when there is only one terminal
    menu.addAction(Properties::Instance()->actions[SUB_COLLAPSE]);
    menu.addSeparator();
    menu.addAction(Properties::Instance()->actions[TOGGLE_MENU]);
    menu.addAction(Properties::Instance()->actions[PREFERENCES]);
    menu.exec(mapToGlobal(pos));
}

void TermWidgetImpl::zoomIn()
{
    emit QTermWidget::zoomIn();
// note: do not save zoom here due the #74 Zoom reset option resets font back to Monospace
//    Properties::Instance()->font = getTerminalFont();
//    Properties::Instance()->saveSettings();
}

void TermWidgetImpl::zoomOut()
{
    emit QTermWidget::zoomOut();
// note: do not save zoom here due the #74 Zoom reset option resets font back to Monospace
//    Properties::Instance()->font = getTerminalFont();
//    Properties::Instance()->saveSettings();
}

void TermWidgetImpl::zoomReset()
{
// note: do not save zoom here due the #74 Zoom reset option resets font back to Monospace
//    Properties::Instance()->font = Properties::Instance()->font;
    setTerminalFont(Properties::Instance()->font);
//    Properties::Instance()->saveSettings();
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
