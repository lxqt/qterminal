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

#include "termwidgetholder.h"
#include "termwidget.h"
#include "properties.h"
#include <assert.h>


TermWidgetHolder::TermWidgetHolder(const QString & wdir, const QString & shell, QWidget * parent)
    : QWidget(parent),
      m_wdir(wdir),
      m_shell(shell),
      m_currentTerm(0)
{
    setFocusPolicy(Qt::NoFocus);
    QGridLayout * lay = new QGridLayout(this);
    lay->setSpacing(0);
    lay->setContentsMargins(0, 0, 0, 0);

    QSplitter *s = new QSplitter(this);
    s->setFocusPolicy(Qt::NoFocus);
    TermWidget *w = newTerm();
    s->addWidget(w);
    lay->addWidget(s);

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

void TermWidgetHolder::setWDir(const QString & wdir)
{
    m_wdir = wdir;
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
    split(term, Qt::Vertical);
}

void TermWidgetHolder::splitVertical(TermWidget * term)
{
    split(term, Qt::Horizontal);
}

void TermWidgetHolder::splitCollapse(TermWidget * term)
{
    QSplitter * parent = qobject_cast<QSplitter*>(term->parent());
    assert(parent);
    term->setParent(0);
    delete term;

    QList<TermWidget *> siblings = parent->findChildren<TermWidget*>();
    int cnt = siblings.count();
    if (cnt == 0)
    {
        parent->setParent(0);
        delete parent;
        parent = Q_NULLPTR;
    }
    if (cnt == 1)
    {
        // We don't need a splitter to display a single element.
        // Find the topmost useless splitter from the hierarchy...
        QSplitter *uselessSplitter = parent;
        do
        {
            QObject *maybeUselessObject = parent->parent();
            QSplitter *maybeUselessSplitter = Q_NULLPTR;
            if (maybeUselessSplitter = qobject_cast<QSplitter *>(maybeUselessObject))
            {
                if (maybeUselessSplitter->count() == 1)
                {
                    uselessSplitter = maybeUselessSplitter;
                    continue;
                }
            }
        } while (false);
        // ... and replace it by a single terminal, if possible
        QSplitter *uselessSplitterParent = qobject_cast<QSplitter *>(uselessSplitter->parent());
        if (uselessSplitterParent)
        {
            int idx = uselessSplitterParent->indexOf(uselessSplitter);
            assert(idx != -1);
            uselessSplitterParent->insertWidget(idx, siblings.at(0));
            uselessSplitter->setParent(0);
            delete uselessSplitter;
            // Make sure there's no access to removed parent
            parent = uselessSplitterParent;
        }
    }

    QList<TermWidget*> tlist = findChildren<TermWidget *>();
    int localCnt = tlist.count();

    if (localCnt > 0)
    {
        tlist.at(0)->setFocus(Qt::OtherFocusReason);
        update();
        if (parent)
            parent->update();
    }
    else
        emit finished();
}

void TermWidgetHolder::split(TermWidget *term, Qt::Orientation orientation)
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

    // wdir settings
    QString wd(m_wdir);
    if (Properties::Instance()->useCWD)
    {
        wd = term->impl()->workingDirectory();
        if (wd.isEmpty())
            wd = m_wdir;
    }

    TermWidget * w = newTerm(wd);
    s->insertWidget(1, w);
    s->setSizes(sizes);

    parent->insertWidget(ix, s);
    parent->setSizes(parentSizes);

    w->setFocus(Qt::OtherFocusReason);
}

TermWidget *TermWidgetHolder::newTerm(const QString & wdir, const QString & shell)
{
    QString wd(wdir);
    if (wd.isEmpty())
        wd = m_wdir;

    QString sh(shell);
    if (shell.isEmpty())
        sh = m_shell;

    TermWidget *w = new TermWidget(wd, sh, this);
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
