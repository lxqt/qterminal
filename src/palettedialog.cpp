/***************************************************************************
 *   Copyright (C) 2025 by Thomas Lübking                                  *
 *   thomas.luebking@gmail.com                                             *
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

#include <QDebug>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QColorDialog>
#include <QMimeData>

#include "palettedialog.h"

PaletteDialog::PaletteDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    QColorDialog *cd = new QColorDialog(this);
    cd->hide();
    connect (colorbutton, SIGNAL(clicked()), cd, SLOT(show()));
    connect (colorbutton, SIGNAL(clicked()), cd, SLOT(raise()));
    QList<QLabel*> tileLabels = tiles->findChildren<QLabel*>();
    for (QLabel *l : tileLabels)
        l->installEventFilter(this);
}

void PaletteDialog::setColorSchemeName(QString name)
{
    nameLabel->setText(name);
}

#define IS_BACKGROUND objectName().contains(QLatin1String("Background"))
#define IS_INTENSE objectName().contains(QLatin1String("Intense"))

QMap<QString, QColor> PaletteDialog::colors()
{
    QMap<QString, QColor> map;
    QList<QLabel*> tileLabels = tiles->findChildren<QLabel*>();
    for (QLabel *l : tileLabels)
        map[l->objectName()] = l->palette().color(l->IS_BACKGROUND ? l->backgroundRole() : l->foregroundRole());
    return map;
}

int PaletteDialog::setColors(QMap<QString, QColor> map)
{
    int match = 0;
    for (auto i = map.cbegin(), end = map.cend(); i != end; ++i) {
        if (QLabel *l = tiles->findChild<QLabel*>(i.key())) {
            setColor(l, i.value());
            ++match;
        }
    }
    return match;
}

void PaletteDialog::setColor(QLabel *l, QColor c)
{
    if (l->IS_BACKGROUND) {
        bool intense = l->IS_INTENSE;
        QList<QLabel*> tileLabels = tiles->findChildren<QLabel*>();
        for (QLabel *fl : tileLabels) {
            if (fl->IS_INTENSE == intense) {
                QPalette pal = fl->palette();
                pal.setColor(fl->backgroundRole(), c);
                fl->setPalette(pal);
            }
        }
    } else {
        QPalette pal = l->palette();
        pal.setColor(l->foregroundRole(), c);
        l->setPalette(pal);
        if (l->objectName().contains(QLatin1String("Foreground"))) {
            const QString name = QString::fromLatin1(l->IS_INTENSE ? "BackgroundIntense" : "Background");
            if (QLabel *bl = tiles->findChild<QLabel*>(name)) // Q_ASSERT
                bl->setPalette(pal);
        }
    }
}

bool PaletteDialog::eventFilter(QObject *o, QEvent *e) {
    if (e->type() == QEvent::DragEnter) {
        QDropEvent *de = static_cast<QDragMoveEvent*>(e);
        de->mimeData()->hasColor() ? de->accept() : de->ignore();
        return false;
    }
    if (e->type() == QEvent::Drop) {
        QDropEvent *de = static_cast<QDropEvent*>(e);
        if (!de->mimeData()->hasColor())
            return false;
        if (QLabel *l = qobject_cast<QLabel*>(o))
            setColor(l, qvariant_cast<QColor>(de->mimeData()->colorData()));
        return false;
    }
    return false;
}
