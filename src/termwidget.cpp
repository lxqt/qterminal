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
#include <QMessageBox>
#include <QAbstractButton>
#include <QMouseEvent>
#include <QRegularExpression>
#include <assert.h>

#ifdef HAVE_QDBUS
    #include <QtDBus/QtDBus>
    #include "termwidgetholder.h"
    #include "terminaladaptor.h"
#endif


#include "mainwindow.h"
#include "termwidget.h"
#include "config.h"
#include "properties.h"
#include "qterminalapp.h"

static int TermWidgetCount = 0;


TermWidgetImpl::TermWidgetImpl(TerminalConfig &cfg, QWidget * parent)
    : QTermWidget(0, parent)
{
    TermWidgetCount++;
    QString name("TermWidget_%1");
    setObjectName(name.arg(TermWidgetCount));

    setFlowControlEnabled(FLOW_CONTROL_ENABLED);
    setFlowControlWarningEnabled(FLOW_CONTROL_WARNING_ENABLED);

    propertiesChanged();

    setHistorySize(5000);

    setWorkingDirectory(cfg.getWorkingDirectory());

    QString shell = cfg.getShell();
    if (!shell.isEmpty())
    {
        qDebug() << "Shell program:" << shell;
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

    connect(this, &QTermWidget::urlActivated, this, &TermWidgetImpl::activateUrl);

    startShellProgram();
}

void TermWidgetImpl::propertiesChanged()
{
    setColorScheme(Properties::Instance()->colorScheme);
    setTerminalFont(Properties::Instance()->font);
    setMotionAfterPasting(Properties::Instance()->m_motionAfterPaste);
    setTerminalSizeHint(Properties::Instance()->showTerminalSizeHint);

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
    setTerminalBackgroundImage(Properties::Instance()->backgroundImage);

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
        setKeyboardCursorShape(QTermWidget::KeyboardCursorShape::UnderlineCursor);
        break;
    case 2:
        setKeyboardCursorShape(QTermWidget::KeyboardCursorShape::IBeamCursor);
        break;
    default:
    case 0:
        setKeyboardCursorShape(QTermWidget::KeyboardCursorShape::BlockCursor);
        break;
    }

    update();
}

void TermWidgetImpl::customContextMenuCall(const QPoint & pos)
{
    QMenu menu;
    QMap<QString, QAction*> actions = findParent<MainWindow>(this)->leaseActions();

    QList<QAction*> extraActions = filterActions(pos);
    for (auto& action : extraActions)
    {
        menu.addAction(action);
    }

    if (!actions.isEmpty())
    {
        menu.addSeparator();
    }

    menu.addAction(actions[COPY_SELECTION]);
    menu.addAction(actions[PASTE_CLIPBOARD]);
    menu.addAction(actions[PASTE_SELECTION]);
    menu.addAction(actions[ZOOM_IN]);
    menu.addAction(actions[ZOOM_OUT]);
    menu.addAction(actions[ZOOM_RESET]);
    menu.addSeparator();
    menu.addAction(actions[CLEAR_TERMINAL]);
    menu.addAction(actions[SPLIT_HORIZONTAL]);
    menu.addAction(actions[SPLIT_VERTICAL]);
    // warning TODO/FIXME: disable the action when there is only one terminal
    menu.addAction(actions[SUB_COLLAPSE]);
    menu.addSeparator();
    menu.addAction(actions[TOGGLE_MENU]);
    menu.addAction(actions[PREFERENCES]);
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

void TermWidgetImpl::activateUrl(const QUrl & url, bool fromContextMenu) {
    if (QApplication::keyboardModifiers() & Qt::ControlModifier || fromContextMenu) {
        QDesktopServices::openUrl(url);
    }
}

void TermWidgetImpl::pasteSelection()
{
    paste(QClipboard::Selection);
}

void TermWidgetImpl::pasteClipboard()
{
    paste(QClipboard::Clipboard);
}

void TermWidgetImpl::paste(QClipboard::Mode mode)
{
    // Paste Clipboard by simulating keypress events
    QString text = QApplication::clipboard()->text(mode);
    if ( ! text.isEmpty() )
    {
        text.replace("\r\n", "\n");
        text.replace('\n', '\r');
        QString trimmedTrailingNl(text);
        trimmedTrailingNl.replace(QRegExp("\\r+$"), "");
        bool isMultiline = trimmedTrailingNl.contains('\r');
        if (!isMultiline && Properties::Instance()->trimPastedTrailingNewlines)
        {
            text = trimmedTrailingNl;
        }
        if (Properties::Instance()->confirmMultilinePaste)
        {
            if (text.contains('\r') && Properties::Instance()->confirmMultilinePaste)
            {
                QMessageBox confirmation(this);
                confirmation.setWindowTitle(tr("Paste multiline text"));
                confirmation.setText(tr("Are you sure you want to paste this text?"));
                confirmation.setDetailedText(text);
                confirmation.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                // Click "Show details..." to show those by default
                foreach( QAbstractButton * btn, confirmation.buttons() )
                {
                    if (confirmation.buttonRole(btn) == QMessageBox::ActionRole && btn->text() == QMessageBox::tr("Show Details..."))
                    {
                        btn->clicked();
                        break;
                    }
                }
                confirmation.setDefaultButton(QMessageBox::Yes);
                confirmation.exec();
                if (confirmation.standardButton(confirmation.clickedButton()) != QMessageBox::Yes)
                {
                    return;
                }
            }
        }

        /* TODO: Support bracketedPasteMode
        if (bracketedPasteMode())
        {
            text.prepend("\e[200~");
            text.append("\e[201~");
        }*/
        sendText(text);
    }
}

bool TermWidget::eventFilter(QObject * obj, QEvent * ev)
{
    if (ev->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mev = (QMouseEvent *)ev;
        if ( mev->button() == Qt::MidButton )
        {
            impl()->pasteSelection();
            return true;
        }
    }
    return false;
}

TermWidget::TermWidget(TerminalConfig &cfg, QWidget * parent)
    : QWidget(parent),
      DBusAddressable("/terminals")
{

    #ifdef HAVE_QDBUS
    registerAdapter<TerminalAdaptor, TermWidget>(this);
    #endif
    m_border = palette().color(QPalette::Window);
    m_term = new TermWidgetImpl(cfg, this);
    setFocusProxy(m_term);

    m_layout = new QVBoxLayout;
    setLayout(m_layout);

    m_layout->addWidget(m_term);
    foreach (QObject *o, m_term->children())
    {
        // Find TerminalDisplay
        if (!o->isWidgetType() || qobject_cast<QWidget*>(o)->isHidden())
            continue;
        o->installEventFilter(this);
    }


    propertiesChanged();

    connect(m_term, SIGNAL(finished()), this, SIGNAL(finished()));
    connect(m_term, SIGNAL(termGetFocus()), this, SLOT(term_termGetFocus()));
    connect(m_term, SIGNAL(termLostFocus()), this, SLOT(term_termLostFocus()));
    connect(m_term, SIGNAL(titleChanged()), this, SLOT(term_titleChanged()));
}

void TermWidget::term_titleChanged()
{
    QString title = m_term->title();

    QRegularExpression re(Properties::Instance()->secondaryFontPattern);
    QFont font = Properties::Instance()->font;
    if (re.isValid() && re.match(title).hasMatch()) {
        qDebug() << "Terminal title changed, pattern match, use secondary font";
        font = Properties::Instance()->secondaryFont;
    } else {
        qDebug() << "Terminal title changed, use default font";
    }

    QFont current_font = m_term->getTerminalFont();
    if (current_font.family() != font.family() || current_font.pointSize() != font.pointSize()) {
        qDebug() << "Update terminal font to " << font << ", was " << current_font;
        m_term->setTerminalFont(font);
    }

    emit termTitleChanged(title, m_term->icon());
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

#if HAVE_QDBUS

QDBusObjectPath TermWidget::splitHorizontal(const QHash<QString,QVariant> &termArgs)
{
    TermWidgetHolder *holder = findParent<TermWidgetHolder>(this);
    assert(holder != NULL);
    TerminalConfig cfg = TerminalConfig::fromDbus(termArgs, this);
    return holder->split(this, Qt::Horizontal, cfg)->getDbusPath();
}

QDBusObjectPath TermWidget::splitVertical(const QHash<QString,QVariant> &termArgs)
{
    TermWidgetHolder *holder = findParent<TermWidgetHolder>(this);
    assert(holder != NULL);
    TerminalConfig cfg = TerminalConfig::fromDbus(termArgs, this);
    return holder->split(this, Qt::Vertical, cfg)->getDbusPath();
}

QDBusObjectPath TermWidget::getTab()
{
    return findParent<TermWidgetHolder>(this)->getDbusPath();
}

void TermWidget::closeTerminal()
{
    TermWidgetHolder *holder = findParent<TermWidgetHolder>(this);
    holder->splitCollapse(this);
}

void TermWidget::sendText(const QString text)
{
    if (impl())
    {
        impl()->sendText(text);
    }
}

#endif
