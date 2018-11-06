#ifndef _cUserManager_HG_
#define _cUserManager_HG_

#include <string>

class cUserManager {
public:

    long long createUserAccount(std::string email, std::string password);
    long long authenticateAccount(std::string email, std::string password, std::string &creationDate);

};

#endif 

