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

#include <qtermwidget.h>

#include <QDebug>
#include <QStyleFactory>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QScreen>
#include <QWindow>

#include "propertiesdialog.h"
#include "properties.h"
#include "fontdialog.h"
#include "palettedialog.h"
#include "config.h"
#include "qterminalapp.h"

void KeySequenceEdit::keyPressEvent(QKeyEvent* event)
{
    // by not allowing multiple shortcuts,
    // the Qt bug that makes Meta a non-modifier is worked around
    clear();
    QKeySequenceEdit::keyPressEvent(event);
}

Delegate::Delegate (QObject *parent)
    : QStyledItemDelegate (parent)
{
}

QWidget* Delegate::createEditor(QWidget *parent,
                                const QStyleOptionViewItem& /*option*/,
                                const QModelIndex& /*index*/) const
{
    return new KeySequenceEdit(parent);
}

bool Delegate::eventFilter(QObject *object, QEvent *event)
{
    KeySequenceEdit *editor = qobject_cast<KeySequenceEdit*>(object);
    if (editor && event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        int k = ke->key();
        // commit data and close the editor with Enter/Return
        // NOTE: "Enter" from numeric pad is accompanied by "KeypadModifier"
        if ((ke->modifiers() == Qt::NoModifier || ke->modifiers() == Qt::KeypadModifier)
            && (k == Qt::Key_Return || k == Qt::Key_Enter)) {
            emit QAbstractItemDelegate::commitData(editor);
            emit QAbstractItemDelegate::closeEditor(editor);
            return true;
        }
        // treat Tab and Backtab like other keys (instead of changing focus)
        if (k == Qt::Key_Tab || k ==  Qt::Key_Backtab) {
            editor->pressKey(ke);
            return true;
        }
    }
    return QStyledItemDelegate::eventFilter (object, event);
}

PropertiesDialog::PropertiesDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked,
            this, &PropertiesDialog::apply);
    connect(changeFontButton, &QPushButton::clicked,
            this, &PropertiesDialog::changeFontButton_clicked);
    connect(chooseBackgroundImageButton, &QPushButton::clicked,
            this, &PropertiesDialog::chooseBackgroundImageButton_clicked);

    // fixed size
#if (QT_VERSION >= QT_VERSION_CHECK(6,7,0))
    connect(saveSizeOnExitCheckBox, &QCheckBox::checkStateChanged, [this] (int state)
#else
    connect(saveSizeOnExitCheckBox, &QCheckBox::stateChanged, [this] (int state)
#endif
    {
        fixedSizeLabel->setEnabled(state == Qt::Unchecked);
        xLabel->setEnabled(state == Qt::Unchecked);
        fixedWithSpinBox->setEnabled(state == Qt::Unchecked);
        fixedHeightSpinBox->setEnabled(state == Qt::Unchecked);
        getCurrentSizeButton->setEnabled(state == Qt::Unchecked);
    });
    connect(getCurrentSizeButton, &QAbstractButton::clicked, [this, parent] {
        if (parent != nullptr)
        {
            QSize pSize = parent->window()->geometry().size();
            fixedWithSpinBox->setValue(pSize.width());
            fixedHeightSpinBox->setValue(pSize.height());
        }
    });
    QSize ag;
    QSize minWinSize(0, 0);
    if (parent != nullptr)
    {
        minWinSize = parent->minimumSize();
        if (QWindow *win = parent->windowHandle())
        {
            if (QScreen *sc = win->screen())
            {
                ag = sc->availableVirtualGeometry().size()
                     // also consider the parent frame thickness because the parent window is fully formed
                     - (parent->window()->frameGeometry().size()
                        - parent->window()->geometry().size());
            }
        }
    }
    fixedWithSpinBox->setMinimum(minWinSize.width());
    fixedHeightSpinBox->setMinimum(minWinSize.height());
    if (!ag.isEmpty())
    {
        fixedWithSpinBox->setMaximum(qMax(ag.width(), minWinSize.width()));
        fixedHeightSpinBox->setMaximum(qMax(ag.height() , minWinSize.height()));
    }

    QStringList emulations = QTermWidget::availableKeyBindings();
    QStringList colorSchemes = QTermWidget::availableColorSchemes();
    colorSchemes.sort(Qt::CaseInsensitive);

    listWidget->setCurrentRow(0);
    // resize the list widget to its content
    listWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    listWidget->setMaximumWidth(listWidget->sizeHintForColumn(0) + 2 * listWidget->frameWidth() + 4);

    colorSchemaCombo->addItems(colorSchemes);
    int csix = colorSchemaCombo->findText(Properties::Instance()->colorScheme);
    if (csix != -1)
        colorSchemaCombo->setCurrentIndex(csix);
    nightSchemaCombo->addItems(colorSchemes);
    nightSchema->setChecked(!Properties::Instance()->nightScheme.isEmpty());
    nightSchemaStart->setTime(Properties::Instance()->nightStart);
    nightSchemaEnd->setTime(Properties::Instance()->nightEnd);
    if (nightSchema->isChecked()) {
        csix = nightSchemaCombo->findText(Properties::Instance()->nightScheme);
        if (csix != -1)
            nightSchemaCombo->setCurrentIndex(csix);
    }
    QMenu *schemeActions = new QMenu(this);
    QAction *csNewAct = schemeActions->addAction(tr("New…"));
    QAction *csEditAct = schemeActions->addAction(tr("Edit…"));
    QAction *csImportAct = schemeActions->addAction(tr("Import…"));
    schemeActions->addSeparator();
    QAction *csDeleteAct = schemeActions->addAction(tr("Delete"));
    configColorSchema->setMenu(schemeActions);
    configNightSchema->setMenu(schemeActions);
    connect(configColorSchema, &QToolButton::triggered, [=](QAction *act) {
        if (act == csNewAct)
            newColorScheme(colorSchemaCombo);
        else if (act == csEditAct)
            editColorScheme(colorSchemaCombo);
        else if (act == csImportAct)
            importColorScheme(colorSchemaCombo);
        else if (act == csDeleteAct)
            deleteColorScheme(colorSchemaCombo);
    });
    connect(configNightSchema, &QToolButton::triggered, [=](QAction *act) {
        if (act == csNewAct)
            newColorScheme(nightSchemaCombo);
        else if (act == csEditAct)
            editColorScheme(nightSchemaCombo);
        else if (act == csImportAct)
            importColorScheme(nightSchemaCombo);
        else if (act == csDeleteAct)
            deleteColorScheme(nightSchemaCombo);
    });
    backgroundImageLineEdit->setText(Properties::Instance()->backgroundImage);

    backgroundModecomboBox->setCurrentIndex(Properties::Instance()->backgroundMode);

    emulationComboBox->addItems(emulations);
    int eix = emulationComboBox->findText(Properties::Instance()->emulation);
    emulationComboBox->setCurrentIndex(eix != -1 ? eix : 0 );

    /* set the delegate of shortcut widget as well as its contents */
    Delegate *del = new Delegate(shortcutsWidget);
    shortcutsWidget->setItemDelegate(del);
    shortcutsWidget->sortByColumn(0, Qt::AscendingOrder);
    setupShortcuts();

    /* scrollbar position */
    QStringList scrollBarPosList;
    scrollBarPosList << tr("No scrollbar") << tr("Left") << tr("Right");
    scrollBarPos_comboBox->addItems(scrollBarPosList);
    scrollBarPos_comboBox->setCurrentIndex(Properties::Instance()->scrollBarPos);

    /* tabs position */
    QStringList tabsPosList;
    tabsPosList << tr("Top") << tr("Bottom") << tr("Left") << tr("Right");
    tabsPos_comboBox->addItems(tabsPosList);
    tabsPos_comboBox->setCurrentIndex(Properties::Instance()->tabsPos);

    /* fixed tabs width */
    fixedTabWidthCheckBox->setChecked(Properties::Instance()->fixedTabWidth);
    fixedTabWidthSpinBox->setValue(Properties::Instance()->fixedTabWidthValue);
    /* tabs features */
    closeTabButtonCheckBox->setChecked(Properties::Instance()->showCloseTabButton);
    closeTabOnMiddleClickCheckBox->setChecked(Properties::Instance()->closeTabOnMiddleClick);

    /* keyboard cursor shape */
    QStringList keyboardCursorShapeList;
    keyboardCursorShapeList << tr("BlockCursor") << tr("UnderlineCursor") << tr("IBeamCursor");
    keybCursorShape_comboBox->addItems(keyboardCursorShapeList);
    keybCursorShape_comboBox->setCurrentIndex(Properties::Instance()->keyboardCursorShape);

    hideTabBarCheckBox->setChecked(Properties::Instance()->hideTabBarWithOneTab);

    // bold font face for intense colors
    boldIntenseCheckBox->setChecked(Properties::Instance()->boldIntense);

    // main menu bar
    menuAccelCheckBox->setChecked(Properties::Instance()->noMenubarAccel);
    showMenuCheckBox->setChecked(Properties::Instance()->menuVisible);

    borderlessCheckBox->setChecked(Properties::Instance()->borderless);

    /* actions by motion after paste */

    QStringList motionAfter;
    motionAfter << tr("No move") << tr("Scrolling to top") << tr("Scrolling to bottom");
    motionAfterPasting_comboBox->addItems(motionAfter);
    motionAfterPasting_comboBox->setCurrentIndex(Properties::Instance()->m_motionAfterPaste);

    disableBracketedPasteModeCheckBox->setChecked(Properties::Instance()->m_disableBracketedPasteMode);

    // word characters for text selection
    wordCharactersLineEdit->setText(Properties::Instance()->wordCharacters);

    // Setting windows style actions
    styleComboBox->addItem(tr("System Default"));
    styleComboBox->addItems(QStyleFactory::keys());

    int ix = styleComboBox->findText(Properties::Instance()->guiStyle);
    if (ix != -1)
        styleComboBox->setCurrentIndex(ix);

    setFontSample(Properties::Instance()->font);

    terminalMarginSpinBox->setValue(Properties::Instance()->terminalMargin);

    termTransparencyBox->setValue(Properties::Instance()->termTransparency);

    highlightCurrentCheckBox->setChecked(Properties::Instance()->highlightCurrentTerminal);

    focusOnMoueOverCheckBox->setChecked(Properties::Instance()->focusOnMoueOver);

    showTerminalSizeHintCheckBox->setChecked(Properties::Instance()->showTerminalSizeHint);

    askOnExitCheckBox->setChecked(Properties::Instance()->askOnExit);

    savePosOnExitCheckBox->setChecked(Properties::Instance()->savePosOnExit);
    saveSizeOnExitCheckBox->setChecked(Properties::Instance()->saveSizeOnExit);
    fixedWithSpinBox->setValue(Properties::Instance()->fixedWindowSize.width());
    fixedHeightSpinBox->setValue(Properties::Instance()->fixedWindowSize.height());

    useCwdCheckBox->setChecked(Properties::Instance()->useCWD);
    openNewTabRightToActiveTabCheckBox->setChecked(Properties::Instance()->m_openNewTabRightToActiveTab);

#ifdef HAVE_LIBCANBERRA
    audibleBellCheckBox->setChecked(Properties::Instance()->audibleBell);
#else
    audibleBellCheckBox->setEnabled(false);
#endif

    termComboBox->setCurrentText(Properties::Instance()->term);

    handleHistoryLineEdit->setText(Properties::Instance()->handleHistoryCommand);

    historyLimited->setChecked(Properties::Instance()->historyLimited);
    historyUnlimited->setChecked(!Properties::Instance()->historyLimited);
    historyLimitedTo->setValue(Properties::Instance()->historyLimitedTo);

    dropShowOnStartCheckBox->setChecked(Properties::Instance()->dropShowOnStart);
    dropKeepOpenCheckBox->setChecked(Properties::Instance()->dropKeepOpen);

    dropHeightSpinBox->setMaximum(100);
    dropWidthSpinBox->setMaximum(100);
    dropHeightSpinBox->setValue(Properties::Instance()->dropHeight);
    dropWidthSpinBox->setValue(Properties::Instance()->dropWidth);

    dropShortCutEdit = new KeySequenceEdit();
    dropShortCutFormLayout->setWidget(0, QFormLayout::FieldRole, dropShortCutEdit);
    dropShortCutEdit->installEventFilter(this);
    dropShortCutEdit->setKeySequence(Properties::Instance()->dropShortCut);

    useBookmarksCheckBox->setChecked(Properties::Instance()->useBookmarks);
    bookmarksLineEdit->setText(Properties::Instance()->bookmarksFile); // also needed by openBookmarksFile()
    connect(bookmarksLineEdit, &QLineEdit::editingFinished,
            this, &PropertiesDialog::bookmarksPathEdited); // manual editing of bookmarks file path
    openBookmarksFile();
    connect(bookmarksButton, &QPushButton::clicked,
            this, &PropertiesDialog::bookmarksButton_clicked);
    exampleBookmarksButton = nullptr;
#ifdef APP_DIR
    exampleBookmarksButton = new QPushButton(tr("Examples"));
    FindBookmarkLayout->addWidget(exampleBookmarksButton);
    connect(exampleBookmarksButton, &QPushButton::clicked,
            this, &PropertiesDialog::bookmarksButton_clicked);
#endif

    terminalPresetComboBox->setCurrentIndex(Properties::Instance()->terminalsPreset);

    changeWindowTitleCheckBox->setChecked(Properties::Instance()->changeWindowTitle);
    changeWindowIconCheckBox->setChecked(Properties::Instance()->changeWindowIcon);
    enabledBidiSupportCheckBox->setChecked(Properties::Instance()->enabledBidiSupport);
    useFontBoxDrawingCharsCheckBox->setChecked(Properties::Instance()->useFontBoxDrawingChars);

    trimPastedTrailingNewlinesCheckBox->setChecked(Properties::Instance()->trimPastedTrailingNewlines);
    confirmMultilinePasteCheckBox->setChecked(Properties::Instance()->confirmMultilinePaste);

    // save the size on canceling too (it's saved on accepting by apply())
    connect(this, &QDialog::rejected, [this] {
        Properties::Instance()->prefDialogSize = size();
        Properties::Instance()->saveSettings();
    });

    // show, hide or disable some widgets on Wayland
    bool onWayland(QGuiApplication::platformName() == QStringLiteral("wayland"));
    savePosOnExitCheckBox->setVisible(!onWayland);
    waylandLabel->setVisible(onWayland);
    dropShortCutLabel->setEnabled(!onWayland);
    dropShortCutEdit->setEnabled(!onWayland);

    // restore its size while fitting it into available desktop geometry
    QSize s;
    if (!Properties::Instance()->prefDialogSize.isEmpty())
        s = Properties::Instance()->prefDialogSize;
    else
        s = size(); // fall back to the ui size
    if (!ag.isEmpty())
        resize(s.boundedTo(ag));
    else // never happens
        resize(s);
}

PropertiesDialog::~PropertiesDialog() = default;

void PropertiesDialog::accept()
{
    apply();
    QDialog::accept();
}

void PropertiesDialog::apply()
{
    Properties::Instance()->colorScheme = colorSchemaCombo->currentText();
    Properties::Instance()->nightScheme = nightSchema->isChecked() ? nightSchemaCombo->currentText() : QString();
    Properties::Instance()->nightStart = nightSchemaStart->time();
    Properties::Instance()->nightEnd = nightSchemaEnd->time();
    Properties::Instance()->font = fontSampleLabel->font();//fontComboBox->currentFont();
    Properties::Instance()->guiStyle = (styleComboBox->currentText() == tr("System Default")) ?
                                       QString() : styleComboBox->currentText();

    Properties::Instance()->emulation = emulationComboBox->currentText();

    Properties::Instance()->terminalMargin = terminalMarginSpinBox->value();
    Properties::Instance()->termTransparency = termTransparencyBox->value();
    Properties::Instance()->highlightCurrentTerminal = highlightCurrentCheckBox->isChecked();
    Properties::Instance()->focusOnMoueOver = focusOnMoueOverCheckBox->isChecked();
    Properties::Instance()->showTerminalSizeHint = showTerminalSizeHintCheckBox->isChecked();
    Properties::Instance()->backgroundImage = backgroundImageLineEdit->text();
    Properties::Instance()->backgroundMode = qBound(0, backgroundModecomboBox->currentIndex(), 4);

    Properties::Instance()->askOnExit = askOnExitCheckBox->isChecked();

    Properties::Instance()->savePosOnExit = savePosOnExitCheckBox->isChecked();
    Properties::Instance()->saveSizeOnExit = saveSizeOnExitCheckBox->isChecked();
    Properties::Instance()->fixedWindowSize = QSize(fixedWithSpinBox->value(), fixedHeightSpinBox->value()).expandedTo(QSize(300, 200)); // FIXME: make Properties variables private and use public methods for setting/getting them
    Properties::Instance()->prefDialogSize = size();

    Properties::Instance()->useCWD = useCwdCheckBox->isChecked();
    Properties::Instance()->m_openNewTabRightToActiveTab = openNewTabRightToActiveTabCheckBox->isChecked();
#ifdef HAVE_LIBCANBERRA
    Properties::Instance()->audibleBell = audibleBellCheckBox->isChecked();
#else
    Properties::Instance()->audibleBell = false;
#endif

    Properties::Instance()->term = termComboBox->currentText();
    Properties::Instance()->handleHistoryCommand = handleHistoryLineEdit->text();

    Properties::Instance()->scrollBarPos = scrollBarPos_comboBox->currentIndex();
    Properties::Instance()->tabsPos = tabsPos_comboBox->currentIndex();
    Properties::Instance()->fixedTabWidth = fixedTabWidthCheckBox->isChecked();
    Properties::Instance()->fixedTabWidthValue = fixedTabWidthSpinBox->value();
    Properties::Instance()->keyboardCursorShape = keybCursorShape_comboBox->currentIndex();
    Properties::Instance()->showCloseTabButton = closeTabButtonCheckBox->isChecked();
    Properties::Instance()->closeTabOnMiddleClick = closeTabOnMiddleClickCheckBox->isChecked();
    Properties::Instance()->hideTabBarWithOneTab = hideTabBarCheckBox->isChecked();
    Properties::Instance()->boldIntense = boldIntenseCheckBox->isChecked();
    Properties::Instance()->noMenubarAccel = menuAccelCheckBox->isChecked();
    Properties::Instance()->menuVisible = showMenuCheckBox->isChecked();
    Properties::Instance()->borderless = borderlessCheckBox->isChecked();
    Properties::Instance()->m_motionAfterPaste = motionAfterPasting_comboBox->currentIndex();
    Properties::Instance()->m_disableBracketedPasteMode = disableBracketedPasteModeCheckBox->isChecked();

    Properties::Instance()->historyLimited = historyLimited->isChecked();
    Properties::Instance()->historyLimitedTo = historyLimitedTo->value();

    applyShortcuts();

    Properties::Instance()->dropShowOnStart = dropShowOnStartCheckBox->isChecked();
    Properties::Instance()->dropKeepOpen = dropKeepOpenCheckBox->isChecked();
    Properties::Instance()->dropHeight = dropHeightSpinBox->value();
    Properties::Instance()->dropWidth = dropWidthSpinBox->value();
    Properties::Instance()->dropShortCut = dropShortCutEdit->keySequence();

    Properties::Instance()->useBookmarks = useBookmarksCheckBox->isChecked();

    Properties::Instance()->terminalsPreset = terminalPresetComboBox->currentIndex();

    Properties::Instance()->changeWindowTitle = changeWindowTitleCheckBox->isChecked();
    Properties::Instance()->changeWindowIcon = changeWindowIconCheckBox->isChecked();
    Properties::Instance()->enabledBidiSupport = enabledBidiSupportCheckBox->isChecked();
    Properties::Instance()->useFontBoxDrawingChars = useFontBoxDrawingCharsCheckBox->isChecked();

    Properties::Instance()->trimPastedTrailingNewlines = trimPastedTrailingNewlinesCheckBox->isChecked();
    Properties::Instance()->confirmMultilinePaste = confirmMultilinePasteCheckBox->isChecked();
    Properties::Instance()->wordCharacters = wordCharactersLineEdit->text();

    saveBookmarksFile();
    // NOTE: Because the path of the bookmarks file may be changed by saveBookmarksFile(),
    // it should be saved only after that.
    Properties::Instance()->bookmarksFile = bookmarksLineEdit->text();

    Properties::Instance()->saveSettings();

    emit propertiesChanged();
}

void PropertiesDialog::setFontSample(const QFont & f)
{
    fontSampleLabel->setFont(f);
    QString sample = QString::fromLatin1("%1 %2 pt");
    fontSampleLabel->setText(sample.arg(f.family()).arg(f.pointSize()));
}

void PropertiesDialog::changeFontButton_clicked()
{
    FontDialog dia(fontSampleLabel->font());
    if (!dia.exec())
        return;
    QFont f = dia.getFont();
    if (QFontInfo(f).fixedPitch())
        setFontSample(f);
}

void PropertiesDialog::chooseBackgroundImageButton_clicked()
{
    QString filename = QFileDialog::getOpenFileName(
                            this, tr("Choose a background image"),
                            QString(), tr("Images (*.bmp *.jpg *.png *.svg *.xpm)"));
    if (!filename.isNull())
        backgroundImageLineEdit->setText(filename);
}

void PropertiesDialog::applyShortcuts()
{
    auto winList = QTerminalApp::Instance()->getWindowList();
    if (winList.isEmpty())
    {
        return;
    }
    QMap<QString, QAction*> actions = winList.at(0)->leaseActions();
    QList< QString > shortcutKeys = actions.keys();
    int shortcutCount = shortcutKeys.count();

    shortcutsWidget->setRowCount( shortcutCount );

    for( int x=0; x < shortcutCount; x++ )
    {
        const QString& keyValue = shortcutKeys.at(x);
        QAction *keyAction = actions[keyValue];

        QTableWidgetItem *item = nullptr;
        QString txt = keyAction->text();
        Properties::removeAccelerator(txt);
        auto items = shortcutsWidget->findItems(txt, Qt::MatchExactly);
        if (!items.isEmpty())
            item = shortcutsWidget->item(shortcutsWidget->row(items.at(0)), 1);
        if (item == nullptr)
            continue;

        QList<QKeySequence> shortcuts;
        const auto sequences = item->text().split(QLatin1Char('|'));
        for (const QString& sequenceString : sequences)
            shortcuts.append(QKeySequence(sequenceString, QKeySequence::NativeText));
        keyAction->setShortcuts(shortcuts);
    }
}

void PropertiesDialog::setupShortcuts()
{
    auto winList = QTerminalApp::Instance()->getWindowList();
    if (winList.isEmpty())
    {
        return;
    }
    // shortcuts may have changed by another running instance
    winList.at(0)->rebuildActions();

    shortcutsWidget->setSortingEnabled(false);

    QMap<QString, QAction*> actions = winList.at(0)->leaseActions();
    QList< QString > shortcutKeys = actions.keys();
    int shortcutCount = shortcutKeys.count();

    shortcutsWidget->setRowCount( shortcutCount );

    for( int x=0; x < shortcutCount; x++ )
    {
        const QString& keyValue = shortcutKeys.at(x);
        QAction *keyAction = actions[keyValue];
        QStringList sequenceStrings;

        const auto shortcuts = keyAction->shortcuts();
        for (const QKeySequence &shortcut : shortcuts)
            sequenceStrings.append(shortcut.toString(QKeySequence::NativeText));

        QString txt = keyAction->text();
        Properties::removeAccelerator(txt);
        QTableWidgetItem *itemName = new QTableWidgetItem(txt);
        QTableWidgetItem *itemShortcut = new QTableWidgetItem( sequenceStrings.join(QLatin1Char('|')) );

        itemName->setFlags( itemName->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsSelectable );

        shortcutsWidget->setItem(x, 0, itemName);
        shortcutsWidget->setItem(x, 1, itemShortcut);
    }

    shortcutsWidget->resizeColumnsToContents();

    shortcutsWidget->setSortingEnabled(true);

    // No shortcut validation is needed with QKeySequenceEdit.
}

void PropertiesDialog::bookmarksPathEdited()
{
    if(!bookmarksLineEdit->isModified()) {
        return;
    }
    auto fname = bookmarksLineEdit->text();
    if (!fname.isEmpty()) {
        QFileInfo fInfo(fname);
        if (fInfo.isFile() && fInfo.isReadable()) {
            openBookmarksFile();
        }
    }
}

void PropertiesDialog::bookmarksButton_clicked()
{
    QFileDialog dia(this, tr("Open bookmarks file"));
    dia.setFileMode(QFileDialog::ExistingFile);
    QString xmlStr = tr("XML files (*.xml)");
    QString allStr = tr("All files (*)");
    dia.setNameFilters(QStringList() << xmlStr << allStr);

    bool openAppDir(QObject::sender() != bookmarksButton);
    if (!openAppDir) {
        // if the path exists, select it; otherwise, open the app directory
        auto path = bookmarksLineEdit->text();
        if (!path.isEmpty() && QFile::exists(path)) {
            if (!path.endsWith(QLatin1String(".xml"))) {
                dia.selectNameFilter(allStr);
            }
            dia.selectFile(path);
        }
        else {
            openAppDir = true;
        }
    }
#ifdef APP_DIR
    if (openAppDir) {
        auto appDirStr = QString::fromUtf8(APP_DIR);
        if (!appDirStr.isEmpty()) {
            QDir appDir(appDirStr);
            if (appDir.exists()) {
                dia.setDirectory(appDir);
            }
        }
    }
#endif

    if (!dia.exec()) {
        return;
    }

    QString fname = dia.selectedFiles().count() ? dia.selectedFiles().at(0) : QString();
    if (fname.isNull()) {
        return;
    }

    bookmarksLineEdit->setText(fname);
    openBookmarksFile();
}

void PropertiesDialog::openBookmarksFile()
{
    auto fname = bookmarksLineEdit->text();
    if (fname.isEmpty()) {
        return;
    }

    QFile f(fname);
    QString content;
    if (!f.open(QFile::ReadOnly)) {
        content = QString::fromLatin1("<qterminal>\n  <group name=\"Change Directory\">\n    <command name=\"Home\" value=\"cd $HOME\"/>\n  </group>\n  <group name=\"File Manager\">\n    <command name=\"Open here\" value=\"xdg-open $(pwd)\"/>\n  </group>\n</qterminal>\n");
    }
    else {
        content = QString::fromUtf8(f.readAll());
    }

    bookmarkPlainEdit->setPlainText(content);
    bookmarkPlainEdit->document()->setModified(false);
}

void PropertiesDialog::saveBookmarksFile()
{
    auto fname = bookmarksLineEdit->text();
    if (fname.isEmpty()) {
        return;
    }

    bool fromAppDir = false;
#ifdef APP_DIR
    // if the file is chosen from the app directory, save it to the config directory
    auto appDirStr = QString::fromUtf8(APP_DIR);
    if (!appDirStr.isEmpty()) {
        QFileInfo fInfo(fname);
        if (fInfo.exists() && fInfo.dir() == QDir(appDirStr)) {
            QString configDir = Properties::Instance()->configDir();
            if (!configDir.isEmpty()) {
                fname = QDir(configDir).absoluteFilePath(fInfo.fileName());
                fromAppDir = true;
            }
        }
    }
#endif

    // don't proceed if the bookmarks file exists but isn't from the app directory
    // and the editor isn't modified
    if (!fromAppDir
        && !bookmarkPlainEdit->document()->isModified()
        && QFile::exists(fname)) {
        return;
    }

    QFile f(fname);

    // first show a prompt message if needed
    if (f.exists()) {
        QMessageBox::StandardButton btn = QMessageBox::Yes;
        if (fromAppDir) {
            btn = QMessageBox::question(this, tr("Question"), tr("Do you want to overwrite this bookmarks file?")
                                                              + QLatin1String("\n%1").arg(fname));
        }
        else if (!fname.endsWith(QLatin1String(".xml"))) {
            btn =  QMessageBox::question(this, tr("Question"), tr("The name of bookmarks file does not end with '.xml'.\nAre you sure that you want to overwrite it?"));
        }
        if (btn == QMessageBox::No) {
            return;
        }
    }

    if (!f.open(QFile::WriteOnly|QFile::Truncate)) {
        QMessageBox::warning(this, tr("Warning"), tr("Cannot write bookmarks to this file:")
                                                  + QLatin1String("\n%1").arg(fname));
    }
    else {
        f.write(bookmarkPlainEdit->toPlainText().toUtf8());
        if (fromAppDir) {
            bookmarksLineEdit->setText(fname); // update the bookmarks file path
        }
        bookmarkPlainEdit->document()->setModified(false); // the user may have clicked "Apply", not "OK"
    }
}

bool PropertiesDialog::eventFilter(QObject *object, QEvent *event)
{
    if (object == dropShortCutEdit) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *ke = static_cast<QKeyEvent *>(event);
            int k = ke->key();
            // treat Tab and Backtab like other keys (instead of changing focus)
            if (k == Qt::Key_Tab || k ==  Qt::Key_Backtab) {
                dropShortCutEdit->pressKey(ke);
                return true;
            }
            // apply with Enter/Return and cancel with Escape, like in other entries
            if (ke->modifiers() == Qt::NoModifier || ke->modifiers() == Qt::KeypadModifier)
            {
                if (k == Qt::Key_Return || k == Qt::Key_Enter) {
                    accept();
                    return true;
                }
                if (k == Qt::Key_Escape) {
                    reject();
                    return true;
                }
            }
        }
    }
    return QDialog::eventFilter(object, event);
}

void PropertiesDialog::newColorScheme(QComboBox *target)
{
    QString newName;
    bool ok = false;
    while (!ok) {
        newName = QInputDialog::getText(this, tr("New Colorscheme"),
                                              tr("Enter the name of the new colorscheme"),
                                              QLineEdit::Normal, newName, &ok);
        if (!ok)
            return;
        if (newName.contains(QLatin1Char('.'))) { // not allowed, see comment in ::import()
            ok = false;
            newName.replace(QLatin1Char('.'), QLatin1Char('-'));
            continue;
        }
        if (target->findText(newName) > -1) {
            ok = false;
            for (int i = 1;;++i) {
                if (target->findText(newName + QString::fromLatin1("_%1").arg(i)) < 0) {
                    newName = newName + QString::fromLatin1("_%1").arg(i);
                    break;
                }
            }
        }
    }
    // create and edit newName
    colorSchemaCombo->addItem(newName);
    nightSchemaCombo->addItem(newName);
    target->setCurrentText(newName);
    editColorScheme(target);
}

void PropertiesDialog::editColorScheme(QComboBox *target)
{
    QString name = target->currentText();
    QString writable = QFileInfo(QSettings().fileName()).canonicalPath() + QLatin1String("/color-schemes/")
                     + name + QLatin1String(".colorscheme");
    QString readable;
    QSettings *scheme = nullptr;
    if (QFile::exists(writable)) {
        scheme = new QSettings(writable, QSettings::IniFormat);
    } else {
        QStringList dirs = QStandardPaths::locateAll(   QStandardPaths::GenericDataLocation, 
                                                        QCoreApplication::applicationName(),
                                                        QStandardPaths::LocateDirectory)
                         + QStandardPaths::locateAll(   QStandardPaths::GenericDataLocation,
                                                        QLatin1String("qtermwidget6"),
                                                        QStandardPaths::LocateDirectory);
        dirs.removeDuplicates(); // QStandardPaths::locateAll() produces duplicates
        
        for (const QString& dir : std::as_const(dirs)) {
            readable = dir + QLatin1String("/color-schemes/") + name + QLatin1String(".colorscheme");
            if (QFile::exists(readable)) {
                scheme = new QSettings(readable, QSettings::IniFormat);
                break;
            }
        }
    }
    PaletteDialog dlg;
    dlg.setColorSchemeName(name);

    QMap<QString, QColor> colors;
    if (scheme) {
        QStringList keys = scheme->allKeys().filter(QLatin1String("/Color"));
        for (const QString &key : std::as_const(keys)) {
            QStringList nac = scheme->value(key).toStringList();
            if (nac.size() < 3) {
                qDebug() << "not a color" << key << scheme->value(key);
                continue;
            }
            colors[key.section(QLatin1Char('/'),0,0)] = QColor(nac.at(0).toShort(), nac.at(1).toShort(), nac.at(2).toShort());
        }
    }
    dlg.setColors(colors);

    if (dlg.exec()) {
        if (!readable.isEmpty())
            QFile::copy(readable, writable);
        if (scheme && scheme->fileName() != writable) {
            delete scheme;
            scheme = nullptr;
        }
        if (!scheme)
            scheme = new QSettings(writable, QSettings::IniFormat);
        colors = dlg.colors();
        for (auto i = colors.cbegin(), end = colors.cend(); i != end; ++i) {
            QColor c = i.value();
            QStringList nac;
            nac << QString::number(c.red()) << QString::number(c.green()) << QString::number(c.blue());
            scheme->setValue(i.key() + QLatin1String("/Color"), nac);
        }
        scheme->sync();
    }
    delete scheme;
}

void PropertiesDialog::importColorScheme(QComboBox *target)
{
    Q_UNUSED(target)
    const QString dir = QFileInfo(QSettings().fileName()).canonicalPath() + QLatin1String("/color-schemes/");
    QStringList imports = QFileDialog::getOpenFileNames(this, tr("Import Color Schemes"),
                                                        QString(), QString::fromLatin1("*.colorscheme"));
    if (imports.isEmpty())
        return;
    if (!QDir(dir).exists())
        QDir().mkpath(dir);
    QStringList baddies, collies;
    for (QString import : imports) {
        if (!QSettings(import, QSettings::IniFormat).allKeys().contains(QString::fromLatin1("Color0/Color"))) {
            baddies << import;
            continue;
        }
        // sanitize name. QTermWidget handles them by the files basename, if the import includes
        // multiple dots like "fancy.dark.colorscheme" this will cause a collision there
        // so we need to make sure the imports stay unambigious and replace inner dot's with
        // dashes ("·" would be nice by maybe don't rely on utf8 too much)
        QString newName = QFileInfo(import).fileName();
        newName = newName.left(newName.lastIndexOf(QLatin1String(".colorscheme"), -1))
                         .replace(QLatin1Char('.'), QLatin1Char('-'));
        
        QString dst = dir + newName + QLatin1String(".colorscheme");
        if (QFile::exists(dst)) {
            collies << import;
            continue;
        }
        QFile::copy(import, dst);
        colorSchemaCombo->addItem(newName);
        nightSchemaCombo->addItem(newName);
    }
    /// @todo QTermWidget doesn't pick up in the new items
    if (!(baddies.isEmpty() && collies.isEmpty())) {
        QString message;
        if (!baddies.isEmpty())
            message += tr("<b>These files don't have a Color0/Color entry - not color schemes?</b><ul><li>")
                    + baddies.join(QString::fromLatin1("</li><li>")) + QString::fromLatin1("</li></ul>");
        if (!collies.isEmpty())
            message += tr("<b>These schemes already exist</b><ul><li>")
                    + collies.join(QString::fromLatin1("</li><li>")) + QString::fromLatin1("</li></ul>");
        QMessageBox::warning(this, tr("Some files could not be imported"), message);
    }
}

void PropertiesDialog::deleteColorScheme(QComboBox *target)
{
    const QString name = target->currentText();
    QString path = QFileInfo(QSettings().fileName()).canonicalPath() + QLatin1String("/color-schemes/")
                 + name + QLatin1String(".colorscheme");
    if (QFile::exists(path)) {
        QFile::remove(path);
        colorSchemaCombo->removeItem(colorSchemaCombo->findText(name));
        nightSchemaCombo->removeItem(nightSchemaCombo->findText(name));
        return;
    }
    QMessageBox::warning(this, tr("%1 could not be removed").arg(target->currentText()),
                               tr("%1 is not a personal scheme and could not be removed").arg(target->currentText()));
}

/*
void PropertiesDialog::setupShortcuts()
{
    QList< QString > shortcutKeys = Properties::Instance()->shortcuts.keys();
    int shortcutCount = shortcutKeys.count();

    shortcutsWidget->setRowCount( shortcutCount );

    for( int x=0; x < shortcutCount; x++ )
    {
        QString keyValue = shortcutKeys.at(x);

        QLabel *lblShortcut = new QLabel( keyValue, this );
        QPushButton *btnLaunch = new QPushButton( Properties::Instance()->shortcuts.value( keyValue ), this );

        btnLaunch->setObjectName(keyValue);
        connect( btnLaunch, SIGNAL(clicked()), this, SLOT(shortcutPrompt()) );

        shortcutsWidget->setCellWidget( x, 0, lblShortcut );
        shortcutsWidget->setCellWidget( x, 1, btnLaunch );
    }
}

void PropertiesDialog::shortcutPrompt()
{
    QObject *objectSender = sender();

    if( !objectSender )
        return;

    QString name = objectSender->objectName();
    qDebug() << "shortcutPrompt(" << name << ")";

    DialogShortcut *dlgShortcut = new DialogShortcut(this);
    dlgShortcut->setTitle( tr("Select a key sequence for %1").arg(name) );

    QString sequenceString = Properties::Instance()->shortcuts[name];
    dlgShortcut->setKey(sequenceString);

    int result = dlgShortcut->exec();
    if( result == QDialog::Accepted )
    {
        sequenceString = dlgShortcut->getKey();
        Properties::Instance()->shortcuts[name] = sequenceString;
        Properties::Instance()->saveSettings();
    }
}
*/
