/***************************************************************************
 *   Copyright (C) 2010 by Petr Vanek                                      *
 *   petr@scribus.info                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

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

    setKeyBindings(Properties::Instance()->emulation);
    setTerminalOpacity(1.0 - Properties::Instance()->termTransparency/100.0);

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

    switch(Properties::Instance()->keyboardCursorShape) {
    case 1:
        setKeyboardCursorShape(QTermWidget::UnderlineCursor);
        break;
    case 2:
        setKeyboardCursorShape(QTermWidget::IBeamCursor);
        break;
    default:
    case 0:
        setKeyboardCursorShape(QTermWidget::BlockCursor);
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
    connect(m_term, &QTermWidget::titleChanged, this, [this] { emit termTitleChanged(m_term->title(), m_term->icon()); });
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
  if (Properties::Instance()->highlightCurrentTerminal)
    {
      QPainter p(this);
      QPen pen(m_border);
      pen.setWidth(3);
      pen.setBrush(m_border);
      p.setPen(pen);
      p.drawRect(0, 0, width()-1, height()-1);
    }
}
