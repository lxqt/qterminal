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

#ifndef PROPERTIESDIALOG_H
#define PROPERTIESDIALOG_H

#include <QStyledItemDelegate>
#include <QKeySequenceEdit>
#include <QPushButton>
#include "ui_propertiesdialog.h"

class KeySequenceEdit : public QKeySequenceEdit
{
    Q_OBJECT

public:
    KeySequenceEdit(QWidget *parent = nullptr) : QKeySequenceEdit(parent) {}

    // to be used with Tab and Backtab
    void pressKey(QKeyEvent *event) {
        QKeySequenceEdit::keyPressEvent(event);
    }

protected:
    void keyPressEvent(QKeyEvent* event) override;
};

class Delegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    Delegate (QObject *parent = nullptr);

    QWidget* createEditor(QWidget *parent,
                                  const QStyleOptionViewItem&,
                                  const QModelIndex&) const override;
    bool eventFilter(QObject *object, QEvent *event) override;
};

class PropertiesDialog : public QDialog, Ui::PropertiesDialog
{
    Q_OBJECT

    QString oldAccelText; // Placeholder when editing shortcut

    public:
        PropertiesDialog(QWidget *parent=nullptr);
        ~PropertiesDialog() override;

        bool eventFilter(QObject *object, QEvent *event) override;

    signals:
        void propertiesChanged();

    private:
        void setFontSample(const QFont & f);
        void openBookmarksFile();
        void saveBookmarksFile();

        KeySequenceEdit *dropShortCutEdit;
        QPushButton *exampleBookmarksButton;

        void newColorScheme(QComboBox *target);
        void editColorScheme(QComboBox *target);
        void importColorScheme(QComboBox *target);
        void deleteColorScheme(QComboBox *target);

    private slots:
        void apply();
        void accept() override;

        void changeFontButton_clicked();
        void chooseBackgroundImageButton_clicked();
        void bookmarksPathEdited();
        void bookmarksButton_clicked();

    protected:
        void setupShortcuts();
        void applyShortcuts();
};


#endif

