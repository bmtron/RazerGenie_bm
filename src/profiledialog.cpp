#include "profiledialog.h"

ProfileDialog::ProfileDialog(QWidget *parent, Profile *profile) : QDialog(parent)
{
    setWindowTitle("New Profile");
    QVBoxLayout *mainDialogLayout = new QVBoxLayout(this);
    auto defaultProfileName = "New Profile";
    auto defaultProfileDpi = "1500";

    QLabel *profNameLabel = new QLabel("Profile Name:");
    QLineEdit *profNameEdit = new QLineEdit();
    profNameEdit->setPlaceholderText(defaultProfileName);
    mainDialogLayout->addWidget(profNameLabel);
    mainDialogLayout->addWidget(profNameEdit);

    QLabel *profDpiLabel = new QLabel("Profile DPI:");
    QLineEdit *profDpiEdit = new QLineEdit();
    profDpiEdit->setPlaceholderText(defaultProfileDpi);
    mainDialogLayout->addWidget(profDpiLabel);
    mainDialogLayout->addWidget(profDpiEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("Ok");
    QPushButton *cancelButton = new QPushButton("Cancel");

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainDialogLayout->addLayout(buttonLayout);

    connect(okButton, &QPushButton::clicked, this, [this, profNameEdit, profDpiEdit, profile, defaultProfileName, defaultProfileDpi]() {
        if (profNameEdit->text().isEmpty()) {
            profNameEdit->setText(defaultProfileName);
        }
        if (profDpiEdit->text().isEmpty()) {
            profDpiEdit->setText(defaultProfileDpi);
        }
        profile->profileName = profNameEdit->text().toStdString();
        profile->profileDpi = (ushort)stoi(profDpiEdit->text().toStdString());
        profile->saveNewProfile();
        accept();
    });
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}
