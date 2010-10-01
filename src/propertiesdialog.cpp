#include <QtGui>
#include "propertiesdialog.h"
#include "properties.h"

PropertiesDialog::PropertiesDialog(const QStringList & emulations, QWidget * parent)
    : QDialog(parent)
{
    setupUi(this);
    connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()),
            this, SLOT(apply()));
    listWidget->setCurrentRow(0);

    QStringList colorSchemes;
    colorSchemes << "White On Black" << \
                    "Green On Black" << \
                    "Black On Light Yellow";
    colorSchemaCombo->addItems(colorSchemes);
    colorSchemaCombo->setCurrentIndex(Properties::Instance()->colorScheme - 1);

    emulationComboBox->addItems(emulations);
    int eix = emulationComboBox->findText(Properties::Instance()->emulation);
    emulationComboBox->setCurrentIndex(eix != -1 ? eix : 0 );
   

    // Setting windows style actions
    styleComboBox->addItem(tr("System Default"));
    styleComboBox->addItems(QStyleFactory::keys());

    int ix = styleComboBox->findText(Properties::Instance()->guiStyle);
    if (ix != -1)
        styleComboBox->setCurrentIndex(ix);
    
    fontComboBox->setCurrentFont(Properties::Instance()->font);
    sizeSpinBox->setValue(Properties::Instance()->font.pointSize());


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
    Properties::Instance()->colorScheme = colorSchemaCombo->currentIndex() + 1;
    Properties::Instance()->font = fontComboBox->currentFont();
    Properties::Instance()->font.setPointSize(sizeSpinBox->value());
    Properties::Instance()->guiStyle = styleComboBox->currentText() == tr("System Default") ? QString() : styleComboBox->currentText();
    Properties::Instance()->emulation = emulationComboBox->currentText();
    emit propertiesChanged();
}

