#include "profile.h"

Profile::Profile()
{
    this->profileName = "";
    this->profileDpi = 0;
}
Profile::Profile(string cfgFilePath) {
    this->cfgFilePath = cfgFilePath;
    Profile();
}

void Profile::saveNewProfile() {
    ofstream cfgFile(this->cfgFilePath, ios::app);
    if (cfgFile.is_open()) {
        cfgFile << "start\n";
        cfgFile << "\tname = " + this->profileName + "\n";
        cfgFile << "\tdpi = " + to_string(this->profileDpi) + "\n";
        cfgFile << "end\n";
        cfgFile.close();
    }
}
Profile::~Profile() = default;
