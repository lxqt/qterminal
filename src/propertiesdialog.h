#ifndef PROPERTIESDIALOG_H
#define PROPERTIESDIALOG_H

#include "ui_propertiesdialog.h"


class PropertiesDialog : public QDialog, Ui::PropertiesDialog
{
    Q_OBJECT

    public:
        PropertiesDialog(const QStringList & emulations,
                         const QStringList & colorSchemes,
                         QWidget * parent = 0);
        ~PropertiesDialog();

    signals:
        void propertiesChanged();

    private:
        void setFontSample(const QFont & f);

    private slots:
        void apply();
        void accept();
        
        void changeFontButton_clicked();
};


#endif

