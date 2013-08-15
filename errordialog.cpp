#include <QDir>

#include "errordialog.h"
#include "ui_errordialog.h"

ErrorDialog::ErrorDialog(std::string msg, QWidget *parent) :
    QDialog(parent),ui(new Ui::ErrorDialog)
{
    ui->setupUi(this);
    ui->Message->setText(msg.c_str());
}

ErrorDialog::~ErrorDialog()
{
    delete ui;
}
