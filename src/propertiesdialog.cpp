#include <QtGui>
#include "propertiesdialog.h"
#include "properties.h"

PropertiesDialog::PropertiesDialog(const QStringList & emulations, const QStringList & colorSchemes, QWidget * parent)
    : QDialog(parent)
{
    setupUi(this);
    connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()),
            this, SLOT(apply()));
    connect(changeFontButton, SIGNAL(clicked()),
            this, SLOT(changeFontButton_clicked()));


    listWidget->setCurrentRow(0);

    colorSchemaCombo->addItems(colorSchemes);
    int csix = colorSchemaCombo->findText(Properties::Instance()->colorScheme);
    if (csix != -1)
        colorSchemaCombo->setCurrentIndex(csix);

    emulationComboBox->addItems(emulations);
    int eix = emulationComboBox->findText(Properties::Instance()->emulation);
    emulationComboBox->setCurrentIndex(eix != -1 ? eix : 0 );
   

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

    Properties::Instance()->scrollBarPos = scrollBarPos_comboBox->currentIndex();
    Properties::Instance()->tabsPos = tabsPos_comboBox->currentIndex();

    Properties::Instance()->saveSettings();

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
    bool ok;
    QFont f = QFontDialog::getFont(&ok, fontSampleLabel->font(), this, tr("Select Terminal Font"));
    if (ok)
        setFontSample(f);
}

