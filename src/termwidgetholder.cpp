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

#include <QGridLayout>
#include <QSplitter>
#include <QInputDialog>

#ifdef HAVE_QDBUS
    #include <QtDBus/QtDBus>
    #include "tabadaptor.h"
#endif

#include "qterminalapp.h"
#include "mainwindow.h"
#include "termwidgetholder.h"
#include "termwidget.h"
#include "properties.h"
#include <assert.h>


TermWidgetHolder::TermWidgetHolder(TerminalConfig &config, QWidget * parent)
    : QWidget(parent)
      #ifdef HAVE_QDBUS
      , DBusAddressable("/tabs")
      #endif
{
    #ifdef HAVE_QDBUS
    new TabAdaptor(this);
    QDBusConnection::sessionBus().registerObject(getDbusPathString(), this);
    #endif
    setFocusPolicy(Qt::NoFocus);
    QGridLayout * lay = new QGridLayout(this);
    lay->setSpacing(0);
    lay->setContentsMargins(0, 0, 0, 0);

    QSplitter *s = new QSplitter(this);
    s->setFocusPolicy(Qt::NoFocus);
    TermWidget *w = newTerm(config);
    s->addWidget(w);
    lay->addWidget(s);
    m_currentTerm = w;

    setLayout(lay);
}

TermWidgetHolder::~TermWidgetHolder()
{
}

void TermWidgetHolder::setInitialFocus()
{
    QList<TermWidget*> list = findChildren<TermWidget*>();
    TermWidget * w = list.count() == 0 ? 0 : list.at(0);
    if (w)
        w->setFocus(Qt::OtherFocusReason);
}

void TermWidgetHolder::loadSession()
{
    bool ok;
    QString name = QInputDialog::getItem(this, tr("Load Session"),
                                         tr("List of saved sessions:"),
                                         Properties::Instance()->sessions.keys(),
                                         0, false, &ok);
    if (!ok || name.isEmpty())
        return;
#if 0
    foreach (QWidget * w, findChildren<QWidget*>())
    {
        if (w)
        {
            delete w;
            w = 0;
        }
    }

    qDebug() << "load" << name << QString(Properties::Instance()->sessions[name]);
    QStringList splitters = QString(Properties::Instance()->sessions[name]).split("|", QString::SkipEmptyParts);
    foreach (QString splitter, splitters)
    {
        QStringList components = splitter.split(",");
        qDebug() << "comp" << components;
        // orientation
        Qt::Orientation orientation;
        if (components.size() > 0)
            orientation = components.takeAt(0).toInt();
        // sizes
        QList<int> sizes;
        QList<TermWidget*> widgets;
        foreach (QString s, components)
        {
            sizes << s.toInt();
            widgets << newTerm();
        }
        // new terms
    }
#endif
}

void TermWidgetHolder::saveSession(const QString & name)
{
    Session dump;
    QString num("%1");
    foreach(QSplitter *w, findChildren<QSplitter*>())
    {
        dump += '|' + num.arg(w->orientation());
        foreach (int i, w->sizes())
            dump += ',' + num.arg(i);
    }
    Properties::Instance()->sessions[name] = dump;
    qDebug() << "dump" << dump;
}

TermWidget* TermWidgetHolder::currentTerminal()
{
    return m_currentTerm;
}

void TermWidgetHolder::switchNextSubterminal()
{
    // TODO/FIXME: merge switchPrevSubterminal with switchNextSubterminal
    QList<TermWidget*> l = findChildren<TermWidget*>();
    int ix = -1;
    foreach (TermWidget * w, l)
    {
        ++ix;
        if (w->impl()->hasFocus())
        {
            break;
        }
    }

    if (ix < l.count()-1)
    {
        l.at(ix+1)->impl()->setFocus(Qt::OtherFocusReason);
    }
    else if (ix == l.count()-1)
    {
        l.at(0)->impl()->setFocus(Qt::OtherFocusReason);
    }
}

void TermWidgetHolder::switchPrevSubterminal()
{
    // TODO/FIXME: merge switchPrevSubterminal with switchNextSubterminal
    QList<TermWidget*> l = findChildren<TermWidget*>();
    int ix = -1;
    foreach (TermWidget * w, l)
    {
        ++ix;
        if (w->impl()->hasFocus())
        {
            break;
        }
    }

    if (ix > 0)
    {
        l.at(ix-1)->impl()->setFocus(Qt::OtherFocusReason);
    }
    else if (ix == 0)
    {
        l.at(l.count()-1)->impl()->setFocus(Qt::OtherFocusReason);
    }
}

void TermWidgetHolder::clearActiveTerminal()
{
    currentTerminal()->impl()->clear();
}

void TermWidgetHolder::propertiesChanged()
{
    foreach(TermWidget *w, findChildren<TermWidget*>())
        w->propertiesChanged();
}

void TermWidgetHolder::splitHorizontal(TermWidget * term)
{
    TerminalConfig defaultConfig;
    split(term, Qt::Vertical, defaultConfig);
}

void TermWidgetHolder::splitVertical(TermWidget * term)
{
    TerminalConfig defaultConfig;
    split(term, Qt::Horizontal, defaultConfig);
}

void TermWidgetHolder::splitCollapse(TermWidget * term)
{
    QSplitter * parent = qobject_cast<QSplitter*>(term->parent());
    assert(parent);
    term->setParent(0);
    delete term;

    QWidget *nextFocus = Q_NULLPTR;

    // Collapse splitters containing a single element, excluding the top one.
    if (parent->count() == 1)
    {
        QSplitter *uselessSplitterParent = qobject_cast<QSplitter*>(parent->parent());
        if (uselessSplitterParent != Q_NULLPTR) {
            int idx = uselessSplitterParent->indexOf(parent);
            assert(idx != -1);
            QWidget *singleHeir = parent->widget(0);
            uselessSplitterParent->insertWidget(idx, singleHeir);
            if (qobject_cast<TermWidget*>(singleHeir))
            {
                nextFocus = singleHeir;
            }
            else
            {
                nextFocus = singleHeir->findChild<TermWidget*>();
            }
            parent->setParent(0);
            delete parent;
            // Make sure there's no access to the removed parent
            parent = uselessSplitterParent;
        }
    }

    if (parent->count() > 0)
    {
        if (nextFocus)
        {
            nextFocus->setFocus(Qt::OtherFocusReason);
        }
        else
        {
            parent->widget(0)->setFocus(Qt::OtherFocusReason);
        }
        parent->update();
    }
    else
        emit finished();
}

TermWidget * TermWidgetHolder::split(TermWidget *term, Qt::Orientation orientation, TerminalConfig cfg)
{
    QSplitter *parent = qobject_cast<QSplitter *>(term->parent());
    assert(parent);

    int ix = parent->indexOf(term);
    QList<int> parentSizes = parent->sizes();

    QList<int> sizes;
    sizes << 1 << 1;

    QSplitter *s = new QSplitter(orientation, this);
    s->setFocusPolicy(Qt::NoFocus);
    s->insertWidget(0, term);

    cfg.provideCurrentDirectory(term->impl()->workingDirectory());
    
    TermWidget * w = newTerm(cfg);
    s->insertWidget(1, w);
    s->setSizes(sizes);

    parent->insertWidget(ix, s);
    parent->setSizes(parentSizes);

    w->setFocus(Qt::OtherFocusReason);
    return w;
}

TermWidget *TermWidgetHolder::newTerm(TerminalConfig &cfg)
{
    TermWidget *w = new TermWidget(cfg, this);
    // proxy signals
    connect(w, SIGNAL(renameSession()), this, SIGNAL(renameSession()));
    connect(w, SIGNAL(removeCurrentSession()), this, SIGNAL(lastTerminalClosed()));
    connect(w, SIGNAL(finished()), this, SLOT(handle_finished()));
    // consume signals

    connect(w, SIGNAL(splitHorizontal(TermWidget *)),
            this, SLOT(splitHorizontal(TermWidget *)));
    connect(w, SIGNAL(splitVertical(TermWidget *)),
            this, SLOT(splitVertical(TermWidget *)));
    connect(w, SIGNAL(splitCollapse(TermWidget *)),
            this, SLOT(splitCollapse(TermWidget *)));
    connect(w, SIGNAL(termGetFocus(TermWidget *)),
            this, SLOT(setCurrentTerminal(TermWidget *)));
    connect(w, &TermWidget::termTitleChanged, this, &TermWidgetHolder::onTermTitleChanged);

    return w;
}

void TermWidgetHolder::setCurrentTerminal(TermWidget* term)
{
    TermWidget * old_current = m_currentTerm;
    m_currentTerm = term;
    if (old_current != m_currentTerm)
    {
        if (m_currentTerm->impl()->isTitleChanged())
        {
            emit termTitleChanged(m_currentTerm->impl()->title(), m_currentTerm->impl()->icon());
        } else
        {
            emit termTitleChanged(windowTitle(), QString{});
        }
    }
}

void TermWidgetHolder::handle_finished()
{
    TermWidget * w = qobject_cast<TermWidget*>(sender());
    if (!w)
    {
        qDebug() << "TermWidgetHolder::handle_finished: Unknown object to handle" << w;
        assert(0);
    }
    splitCollapse(w);
}

void TermWidgetHolder::onTermTitleChanged(QString title, QString icon) const
{
    TermWidget * term = qobject_cast<TermWidget *>(sender());
    if (m_currentTerm == term)
        emit termTitleChanged(title, icon);
}

#ifdef HAVE_QDBUS

QDBusObjectPath TermWidgetHolder::getActiveTerminal()
{
    if (m_currentTerm != NULL)
    {
        return m_currentTerm->getDbusPath();
    }
    return QDBusObjectPath();
}

QList<QDBusObjectPath> TermWidgetHolder::getTerminals()
{
    QList<QDBusObjectPath> terminals;
    foreach (TermWidget* w, findChildren<TermWidget*>())
    {
        terminals.push_back(w->getDbusPath());
    }
    return terminals;
}

QDBusObjectPath TermWidgetHolder::getWindow()
{
    return findParent<MainWindow>(this)->getDbusPath();
}

void TermWidgetHolder::closeTab()
{
    QTabWidget *parent = findParent<QTabWidget>(this);
    int idx = parent->indexOf(this);
    assert(idx != -1);
    parent->tabCloseRequested(idx);
}

#endif

