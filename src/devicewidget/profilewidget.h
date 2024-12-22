// Copyright (C) 2017-2024  Luca Weiss <luca (at) z3ntu (dot) xyz>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROFILEWIDGET_H
#define PROFILEWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <libopenrazer.h>
#include "profile.h"
#include <vector>
#include <string>

using namespace std;

class ProfileWidget : public QWidget
{
    Q_OBJECT
public:
    ProfileWidget(libopenrazer::Device *device);
    ~ProfileWidget() override;
    vector<Profile> profiles;
    Profile *selectedProfile;

    void loadProfilesIntoDropdown(QComboBox *comboBox);
    static bool isAvailable(libopenrazer::Device *device);

private:
    libopenrazer::Device *device;
    string cfgFilePath;
    void loadProfiles();
    void saveProfile(Profile *profile);
};

#endif // PROFILEWIDGET_H
