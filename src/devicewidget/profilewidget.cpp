#include "profilewidget.h"
#include "profile.h"
#include "../profiledialog.h"

#include <fstream>
#include <iostream>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QMessageBox>

using namespace std;

ProfileWidget::ProfileWidget(libopenrazer::Device *device)
    : QWidget()
{
    this->device = device;
    this->selectedProfile = nullptr;
    this->cfgFilePath = "./profiles.cfg"; // hardcoding for now, no idea how this should actually operate

    auto *verticalLayout = new QVBoxLayout(this);
    QFont headerFont("Arial", 15, QFont::Bold);

    QLabel *activeProfileName = new QLabel(tr("Active Profile: "));
    QLabel *activeProfileDpi = new QLabel(tr("Profile DPI: "));
    QLabel *activeProfNameValue = new QLabel(tr(""));
    QLabel *activeProfDpiValue = new QLabel(tr(""));

    activeProfileName->setFont(headerFont);
    activeProfileDpi->setFont(headerFont);


    QHBoxLayout *labelNameGrid = new QHBoxLayout();
    QHBoxLayout *labelDpiGrid = new QHBoxLayout();

    activeProfileName->setFixedSize(150, 30);
    activeProfileDpi->setFixedSize(150, 30);

    activeProfNameValue->setFixedSize(150, 30);
    activeProfDpiValue->setFixedSize(150, 30);

    labelNameGrid->addWidget(activeProfileName, 0);
    labelNameGrid->addWidget(activeProfNameValue, 0);
    labelNameGrid->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    labelDpiGrid->addWidget(activeProfileDpi, 0);
    labelDpiGrid->addWidget(activeProfDpiValue, 0);
    labelDpiGrid->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));



    /* check to make sure device supports poll rate before loading or saving this info */

    if (device->hasFeature("poll_rate")) {
        QComboBox *profilesComboBox = new QComboBox;
        profilesComboBox->setFixedSize(150, 30);

        this->loadProfilesIntoDropdown(profilesComboBox);
        QHBoxLayout *profilesComboBtnLayout = new QHBoxLayout();

        profilesComboBox->setCurrentText(QString::fromStdString(""));

        auto *loadProfileBtn = new QPushButton(this);
        auto *saveProfileBtn = new QPushButton(this);
        auto *newProfileBtn = new QPushButton(this);

        loadProfileBtn->setText(tr("Load Profile"));
        saveProfileBtn->setText(tr("Save Profile"));
        newProfileBtn->setText(tr("Create New Profile"));

        loadProfileBtn->setEnabled(false);
        saveProfileBtn->setEnabled(false);

        newProfileBtn->setFixedSize(125, 30);
        loadProfileBtn->setFixedSize(100, 30);
        saveProfileBtn->setFixedSize(100, 30);



        connect(loadProfileBtn, &QPushButton::clicked, this, [=]() {
            openrazer::DPI dpi;
            dpi.dpi_x = this->selectedProfile->profileDpi;
            dpi.dpi_y = this->selectedProfile->profileDpi;
            this->device->setDPI(dpi);
            activeProfDpiValue->setText(QString::fromStdString(to_string(this->selectedProfile->profileDpi)));
            activeProfNameValue->setText(QString::fromStdString(this->selectedProfile->profileName));
        });
        connect(saveProfileBtn, &QPushButton::clicked, this, [=]() {
            if (this->selectedProfile == nullptr) {
                // TODO: implement saving/editing a selected profile. also delete.
            }

        });
        connect(newProfileBtn, &QPushButton::clicked, this, [=]() {
            Profile *p = new Profile(this->cfgFilePath);
            ProfileDialog pd(this, p);
            pd.exec();
            this->loadProfilesIntoDropdown(profilesComboBox);
        });

        profilesComboBtnLayout->addWidget(profilesComboBox);
        profilesComboBtnLayout->addWidget(newProfileBtn, 0);
        profilesComboBtnLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

        verticalLayout->addLayout(labelNameGrid, Qt::AlignLeft);
        verticalLayout->addLayout(labelDpiGrid, Qt::AlignLeft);
        verticalLayout->addLayout(profilesComboBtnLayout);
        verticalLayout->addWidget(loadProfileBtn, 0);
        verticalLayout->addWidget(saveProfileBtn, 0);

        connect(profilesComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {

            auto currentProfileName = profilesComboBox->itemText(index).toStdString();
            bool foundProfile = false;
            for (Profile &p : this->profiles) {
                if (p.profileName == currentProfileName) {
                    this->selectedProfile = &p;
                    foundProfile = true;
                }
            }
            if (!foundProfile) {
                this->selectedProfile = nullptr;
            }
            if (this->selectedProfile != nullptr) {
                loadProfileBtn->setEnabled(true);
                saveProfileBtn->setEnabled(true);
            }
            else {
                loadProfileBtn->setEnabled(false);
                saveProfileBtn->setEnabled(false);
            }
        });
    }
    auto *spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    verticalLayout->addItem(spacer);
}

ProfileWidget::~ProfileWidget() = default;

bool ProfileWidget::isAvailable(libopenrazer::Device *device)
{
    return device->hasFeature("dpi") || device->hasFeature("poll_rate");
}

void ProfileWidget::loadProfiles() {
    if (!this->profiles.empty()) {
        this->profiles.clear();
    }
    ifstream cfgFile(this->cfgFilePath);
    string line;
    bool enteredCfgSection = false;
    Profile *p;
    int lenOfCfgString;
    if (cfgFile.is_open()) {
        while (getline(cfgFile, line)) {

            if (!enteredCfgSection && line == "start") {
                enteredCfgSection = true;
                p = new Profile();
            }
            if (enteredCfgSection && line == "end") {
                enteredCfgSection = false;
                this->profiles.push_back(*p);
            }
            if (enteredCfgSection && line.find("name = ") != string::npos) {
                lenOfCfgString = 7;
                p->profileName = line.substr(line.find("name = ") + lenOfCfgString, line.length() - 1);
            }
            if (enteredCfgSection && line.find("dpi = ") != string::npos) {
                lenOfCfgString = 6;
                string dpiAsStr = line.substr(line.find("dpi = ") + lenOfCfgString, line.length() -1);
                p->profileDpi = (ushort) stoi(dpiAsStr);
            }
        }
    } else {
        QMessageBox msgBox;
        msgBox.setText(QString::fromStdString("The profiles config file was not found at the default location of " + this->cfgFilePath + "."));
        msgBox.exec();
    }

}

void ProfileWidget::loadProfilesIntoDropdown(QComboBox *profilesComboBox) {
    profilesComboBox->clear();
    this->loadProfiles();
    profilesComboBox->addItem(QString::fromStdString(""), (ushort)0);
    for (const Profile &p : this->profiles) {
        profilesComboBox->addItem(QString::fromStdString(p.profileName), p.profileDpi);
    }
}

// possibly don't need this anymore, since this functionality is handled
// by the profile object itself. Because...reasons...
void ProfileWidget::saveProfile(Profile *profile) {
    ofstream cfgFile(this->cfgFilePath, ios::app);
    if (cfgFile.is_open()) {
        cfgFile << "start\n";
        cfgFile << "\tname = " + profile->profileName + "\n";
        cfgFile << "\tdpi = " + to_string(profile->profileDpi) + "\n";
        cfgFile << "end\n";
        cfgFile.close();
    }
}
