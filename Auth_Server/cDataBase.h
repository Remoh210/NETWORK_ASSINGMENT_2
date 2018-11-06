#ifndef _cDataBase_HG_
#define _cDataBase_HG_
#include <string>
#include <stdlib.h>
#include <iostream>


#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

using namespace std;

class cDataBase
{
public:
		
	long long insertUser(std::string email, std::string hashedPassword, std::string salt);
	long long selectUser(std::string email, std::string &hashedPassword, std::string &salt, std::string &creationDate);

};

#endif 

