#ifndef PROFILE_H
#define PROFILE_H

#include <string>
#include <fstream>

using namespace std;

class Profile
{

public:
    Profile();
    Profile(string cfgFilePath);
    ~Profile();
    string cfgFilePath;
    string profileName;
    ushort profileDpi;
    void saveNewProfile();
};

#endif // PROFILE_H
