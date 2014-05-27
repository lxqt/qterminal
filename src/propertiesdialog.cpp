#include <qtermwidget.h>

#include <QDebug>
#include <QStyleFactory>
#include <QFileDialog>

#include "propertiesdialog.h"
#include "properties.h"
#include "fontdialog.h"
#include "config.h"


PropertiesDialog::PropertiesDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()),
            this, SLOT(apply()));
    connect(changeFontButton, SIGNAL(clicked()),
            this, SLOT(changeFontButton_clicked()));

    QStringList emulations = QTermWidget::availableKeyBindings();
    QStringList colorSchemes = QTermWidget::availableColorSchemes();

    listWidget->setCurrentRow(0);

    colorSchemaCombo->addItems(colorSchemes);
    int csix = colorSchemaCombo->findText(Properties::Instance()->colorScheme);
    if (csix != -1)
        colorSchemaCombo->setCurrentIndex(csix);

    emulationComboBox->addItems(emulations);
    int eix = emulationComboBox->findText(Properties::Instance()->emulation);
    emulationComboBox->setCurrentIndex(eix != -1 ? eix : 0 );
   
    /* shortcuts */
    setupShortcuts();

    /* scrollbar position */
    QStringList scrollBarPosList;
    scrollBarPosList << "No scrollbar" << "Left" << "Right";
    scrollBarPos_comboBox->addItems(scrollBarPosList);
    scrollBarPos_comboBox->setCurrentIndex(Properties::Instance()->scrollBarPos);

    /* tabs position */
    QStringList tabsPosList;
    tabsPosList << "Top" << "Bottom" << "Left" << "Right";
    tabsPos_comboBox->addItems(tabsPosList);
    tabsPos_comboBox->setCurrentIndex(Properties::Instance()->tabsPos);

    alwaysShowTabsCheckBox->setChecked(Properties::Instance()->alwaysShowTabs);

    // show main menu bar
    showMenuCheckBox->setChecked(Properties::Instance()->menuVisible);

    /* actions by motion after paste */
    
    QStringList motionAfter;
    motionAfter << "No move" << "Move start" << "Move end";
    motionAfterPasting_comboBox->addItems(motionAfter);
    motionAfterPasting_comboBox->setCurrentIndex(Properties::Instance()->m_motionAfterPaste);
    
    // Setting windows style actions
    styleComboBox->addItem(tr("System Default"));
    styleComboBox->addItems(QStyleFactory::keys());

    int ix = styleComboBox->findText(Properties::Instance()->guiStyle);
    if (ix != -1)
        styleComboBox->setCurrentIndex(ix);
    
    setFontSample(Properties::Instance()->font);

    appOpacityBox->setValue(Properties::Instance()->appOpacity);
    //connect(appOpacityBox, SIGNAL(valueChanged(int)), this, SLOT(apply()));

    termOpacityBox->setValue(Properties::Instance()->termOpacity);
    //connect(termOpacityBox, SIGNAL(valueChanged(int)), this, SLOT(apply()));

    highlightCurrentCheckBox->setChecked(Properties::Instance()->highlightCurrentTerminal);

    askOnExitCheckBox->setChecked(Properties::Instance()->askOnExit);

    useCwdCheckBox->setChecked(Properties::Instance()->useCWD);

    historyLimited->setChecked(Properties::Instance()->historyLimited);
    historyUnlimited->setChecked(!Properties::Instance()->historyLimited);
    historyLimitedTo->setValue(Properties::Instance()->historyLimitedTo);

    dropShowOnStartCheckBox->setChecked(Properties::Instance()->dropShowOnStart);
    dropHeightSpinBox->setValue(Properties::Instance()->dropHeight);
    dropWidthSpinBox->setValue(Properties::Instance()->dropWidht);
    dropShortCutEdit->setText(Properties::Instance()->dropShortCut.toString());

    useBookmarksCheckBox->setChecked(Properties::Instance()->useBookmarks);
    bookmarksLineEdit->setText(Properties::Instance()->bookmarksFile);
    openBookmarksFile(Properties::Instance()->bookmarksFile);
    connect(bookmarksButton, SIGNAL(clicked()),
            this, SLOT(bookmarksButton_clicked()));
}


PropertiesDialog::~PropertiesDialog()
{
}

void PropertiesDialog::accept()
{
    apply();
    QDialog::accept();
}

void PropertiesDialog::apply()
{
    Properties::Instance()->colorScheme = colorSchemaCombo->currentText();
    Properties::Instance()->font = fontSampleLabel->font();//fontComboBox->currentFont();
    Properties::Instance()->guiStyle = (styleComboBox->currentText() == tr("System Default")) ?
                                       QString() : styleComboBox->currentText();

    Properties::Instance()->emulation = emulationComboBox->currentText();

    /* do not allow to go above 99 or we lose transparency option */
    (appOpacityBox->value() >= 100) ?
            Properties::Instance()->appOpacity = 99
                :
            Properties::Instance()->appOpacity = appOpacityBox->value();

    Properties::Instance()->termOpacity = termOpacityBox->value();
    Properties::Instance()->highlightCurrentTerminal = highlightCurrentCheckBox->isChecked();

    Properties::Instance()->askOnExit = askOnExitCheckBox->isChecked();

    Properties::Instance()->useCWD = useCwdCheckBox->isChecked();

    Properties::Instance()->scrollBarPos = scrollBarPos_comboBox->currentIndex();
    Properties::Instance()->tabsPos = tabsPos_comboBox->currentIndex();
    Properties::Instance()->alwaysShowTabs = alwaysShowTabsCheckBox->isChecked();
    Properties::Instance()->menuVisible = showMenuCheckBox->isChecked();
    Properties::Instance()->m_motionAfterPaste = motionAfterPasting_comboBox->currentIndex();

    Properties::Instance()->historyLimited = historyLimited->isChecked();
    Properties::Instance()->historyLimitedTo = historyLimitedTo->value();

    saveShortcuts();

    Properties::Instance()->saveSettings();

    Properties::Instance()->dropShowOnStart = dropShowOnStartCheckBox->isChecked();
    Properties::Instance()->dropHeight = dropHeightSpinBox->value();
    Properties::Instance()->dropWidht = dropWidthSpinBox->value();
    Properties::Instance()->dropShortCut = QKeySequence(dropShortCutEdit->text());

    Properties::Instance()->useBookmarks = useBookmarksCheckBox->isChecked();
    Properties::Instance()->bookmarksFile = bookmarksLineEdit->text();
    saveBookmarksFile(Properties::Instance()->bookmarksFile);

    emit propertiesChanged();
}

void PropertiesDialog::setFontSample(const QFont & f)
{
    fontSampleLabel->setFont(f);
    QString sample("%1 %2 pt");
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

void PropertiesDialog::saveShortcuts()
{
    QList< QString > shortcutKeys = Properties::Instance()->actions.keys();
    int shortcutCount = shortcutKeys.count();

    shortcutsWidget->setRowCount( shortcutCount );

    for( int x=0; x < shortcutCount; x++ )
    {
        QString keyValue = shortcutKeys.at(x);
        QAction *keyAction = Properties::Instance()->actions[keyValue];

        QTableWidgetItem *item = shortcutsWidget->item(x, 1);
        QKeySequence sequence = QKeySequence(item->text());
        QString sequenceString = sequence.toString();

        keyAction->setShortcut(sequenceString);
    }
}

void PropertiesDialog::setupShortcuts()
{
    QList< QString > shortcutKeys = Properties::Instance()->actions.keys();
    int shortcutCount = shortcutKeys.count();

    shortcutsWidget->setRowCount( shortcutCount );

    for( int x=0; x < shortcutCount; x++ )
    {
        QString keyValue = shortcutKeys.at(x);
        QAction *keyAction = Properties::Instance()->actions[keyValue];

        QTableWidgetItem *itemName = new QTableWidgetItem( tr(keyValue.toStdString().c_str()) );
        QTableWidgetItem *itemShortcut = new QTableWidgetItem( keyAction->shortcut().toString() );

        itemName->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );

        shortcutsWidget->setItem(x, 0, itemName);
        shortcutsWidget->setItem(x, 1, itemShortcut);
    }

    shortcutsWidget->resizeColumnsToContents();
/*
    connect(shortcutsWidget, SIGNAL(currentChanged(int, int)),
            this, SLOT(recordAction(int, int)));
    connect(shortcutsWidget, SIGNAL(valueChanged(int, int)),
            this, SLOT(validateAction(int, int)));
*/
}

void PropertiesDialog::recordAction(int row, int column)
{
    oldAccelText = shortcutsWidget->item(row, column)->text();
}

void PropertiesDialog::validateAction(int row, int column)
{
    QTableWidgetItem *item = shortcutsWidget->item(row, column);
    QString accelText = QKeySequence(item->text()).toString();

    if (accelText.isEmpty() && !item->text().isEmpty())
        item->setText(oldAccelText);
    else
        item->setText(accelText);
}

void PropertiesDialog::bookmarksButton_clicked()
{
    QFileDialog dia(this, tr("Open or create bookmarks file"));
    dia.setConfirmOverwrite(false);
    dia.setFileMode(QFileDialog::AnyFile);
    if (!dia.exec())
        return;

    QString fname = dia.selectedFiles().count() ? dia.selectedFiles().at(0) : QString();
    if (fname.isNull())
        return;

    bookmarksLineEdit->setText(fname);
    openBookmarksFile(bookmarksLineEdit->text());
}

void PropertiesDialog::openBookmarksFile(const QString &fname)
{
    QFile f(fname);
    QString content;
    if (!f.open(QFile::ReadOnly))
        content = "<qterminal>\n  <group name\"group1\">\n    <command name=\"cmd1\" value=\"cd $HOME\"/>\n  </group>\n</qterminal>";
    else
        content = f.readAll();

    bookmarkPlainEdit->setPlainText(content);
    bookmarkPlainEdit->document()->setModified(false);
}

void PropertiesDialog::saveBookmarksFile(const QString &fname)
{
    if (!bookmarkPlainEdit->document()->isModified())
        return;

    QFile f(fname);
    if (!f.open(QFile::WriteOnly|QFile::Truncate))
        qDebug() << "Cannot write to file" << f.fileName();
    else
        f.write(bookmarkPlainEdit->toPlainText().toUtf8());
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
