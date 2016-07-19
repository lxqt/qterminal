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

#ifndef TERMWIDGET_H
#define TERMWIDGET_H

#include <qtermwidget.h>

#include <QAction>


class TermWidgetImpl : public QTermWidget
{
    Q_OBJECT

//        QMap< QString, QAction * > actionMap;

    public:

        TermWidgetImpl(const QString & wdir, const QString & shell=QString(), QWidget * parent=0);
        void propertiesChanged();

    signals:
        void renameSession();
        void removeCurrentSession();

    public slots:
        void zoomIn();
        void zoomOut();
        void zoomReset();

    private slots:
        void customContextMenuCall(const QPoint & pos);
        void activateUrl(const QUrl& url);
};


class TermWidget : public QWidget
{
    Q_OBJECT

    TermWidgetImpl * m_term;
    QVBoxLayout * m_layout;
    QColor m_border;

    public:
        TermWidget(const QString & wdir, const QString & shell=QString(), QWidget * parent=0);

        void propertiesChanged(); 
        QStringList availableKeyBindings() { return m_term->availableKeyBindings(); }

        TermWidgetImpl * impl() { return m_term; }

    signals:
        void finished();
        void renameSession();
        void removeCurrentSession();
        void splitHorizontal(TermWidget * self);
        void splitVertical(TermWidget * self);
        void splitCollapse(TermWidget * self);
        void termGetFocus(TermWidget * self);
        void termTitleChanged(QString titleText, QString icon);

    public slots:

    protected:
        void paintEvent (QPaintEvent * event);

    private slots:
        void term_termGetFocus();
        void term_termLostFocus();
};

#endif

