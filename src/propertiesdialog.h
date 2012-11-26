#ifndef PROPERTIESDIALOG_H
#define PROPERTIESDIALOG_H

#include "ui_propertiesdialog.h"

class PropertiesDialog : public QDialog, Ui::PropertiesDialog
{
    Q_OBJECT

    QString oldAccelText; // Placeholder when editing shortcut

    public:
        PropertiesDialog(QWidget *parent=NULL);
        ~PropertiesDialog();

    signals:
        void propertiesChanged();

    private:
        void setFontSample(const QFont & f);

    private slots:
        void apply();
        void accept();
        
        void changeFontButton_clicked();

    protected:
        void setupShortcuts();
        void saveShortcuts();
        void recordAction(int row, int column);
        void validateAction(int row, int column);
};


#endif

