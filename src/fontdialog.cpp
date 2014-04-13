
#include "fontdialog.h"

FontDialog::FontDialog(const QFont &f)
    : QDialog(0)
{
    setupUi(this);

    fontComboBox->setFontFilters(QFontComboBox::MonospacedFonts
                                 | QFontComboBox::NonScalableFonts
                                 | QFontComboBox::ScalableFonts);

    fontComboBox->setCurrentFont(f);
    fontComboBox->setEditable(false);

    sizeSpinBox->setValue(f.pointSize());

    setFontSample();

    connect(fontComboBox, SIGNAL(currentFontChanged(QFont)),
            this, SLOT(setFontSample()));
    connect(sizeSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(setFontSample()));
}

QFont FontDialog::getFont()
{
    QFont f = fontComboBox->currentFont();
    f.setPointSize(sizeSpinBox->value());
    return f;
}

void FontDialog::setFontSample()
{
    QFont f = getFont();
    previewLabel->setFont(f);
    QString sample("%1 %2 pt");
    previewLabel->setText(sample.arg(f.family()).arg(f.pointSize()));
}
