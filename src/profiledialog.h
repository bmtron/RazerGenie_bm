#ifndef PROFILEDIALOG_H
#define PROFILEDIALOG_H

#include "devicewidget/profile.h"
#include <string>
#include <QApplication>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QWidget>

class ProfileDialog : public QDialog
{
    Q_OBJECT
public:
    ProfileDialog(QWidget *parent, Profile *profile);
};

#endif // PROFILEDIALOG_H
