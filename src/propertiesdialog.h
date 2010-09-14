#ifndef PROPERTIESDIALOG_H
#define PROPERTIESDIALOG_H

#include "ui_propertiesdialog.h"


class PropertiesDialog : public QDialog, Ui::PropertiesDialog
{
    Q_OBJECT

    public:
        PropertiesDialog(QWidget * parent = 0);
        ~PropertiesDialog();

    signals:
        void propertiesChanged();

    private slots:
        void apply();
        void accept();

};


#endif

