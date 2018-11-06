#include "cDataBase.h"
#include <vector>

struct UserInfo
{
	long long authId = -1;
	long long userId = -1;
	std::string email;
	std::string hashedPassword;
	std::string salt;
	std::string lastLogin;
	std::string creationDate;
};

std::vector<UserInfo> user;
long long gAuthId = 0;
long long  gUserId = 0;

bool findEmail(std::string email)
{
	string resultEmail, queryAsString;
	
	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt;
		sql::ResultSet  *res;

		driver = get_driver_instance();
		con = driver->connect( "tcp://localhost:3306", "root", "123123" );
		con->setSchema( "nyan_db" );

		stmt = con->createStatement();

		queryAsString = "SELECT email FROM web_auth WHERE email = '" + email + "'";
		res = stmt->executeQuery( queryAsString.c_str() );

		while( res->next() )
		{
			resultEmail = res->getString( "email" );
		}

		delete res;
		delete stmt;
		delete con;
	}
	
	catch( sql::SQLException &e ) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
		
		return false;
	}

	if( resultEmail == email )
	{
		return true;
	}
	
	return false;
}

long long cDataBase::insertUser(std::string email, std::string hashedPassword, std::string salt)
{

	if (findEmail(email))
	{
		return -1;
	}
	
	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::PreparedStatement  *prep_stmt;

		
		driver = get_driver_instance();
		con = driver->connect( "tcp://localhost:3306", "root", "123123" );
		con->setSchema( "nyan_db" );

		prep_stmt = con->prepareStatement( "INSERT INTO web_auth( email, salt, hashed_password, userId ) VALUES( ? , ? , ? , ? )" );
		prep_stmt->setString( 1, email );	//email, 
		prep_stmt->setString( 2, salt );	//salt
		prep_stmt->setString( 3, hashedPassword ); //hashed_password
		prep_stmt->setInt( 4, ++gUserId );		//userId
		prep_stmt->execute();

		prep_stmt = con->prepareStatement( "INSERT INTO user( last_login, creation_date )  VALUES( CURRENT_TIMESTAMP , CURRENT_TIMESTAMP )" );
		prep_stmt->execute();
	
		delete prep_stmt;
		delete con;
	}

	catch( sql::SQLException &e ) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}

	UserInfo thisUser;
	thisUser.authId = ++gAuthId;
	thisUser.userId = ++gUserId;
	thisUser.email = email;
	thisUser.hashedPassword = hashedPassword;
	thisUser.salt = salt;
	thisUser.creationDate = "Some date";
	thisUser.lastLogin = "Some date";
	user.push_back(thisUser);
	return thisUser.userId;

}

long long cDataBase::selectUser(std::string email, std::string &hashedPassword, std::string &salt, std::string &creationDate)
{
	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt;
		sql::ResultSet  *res;

		string queryAsString, userIdAsString;
		UserInfo thisUser;

		driver = get_driver_instance();
		con = driver->connect( "tcp://localhost:3306", "root", "123123" );
		con->setSchema( "nyan_db" );

		stmt = con->createStatement();

		queryAsString = "SELECT id, email, salt, hashed_password, userId FROM web_auth WHERE email = '" + email + "'";
		res = stmt->executeQuery( queryAsString.c_str() );
		while( res->next() )
		{
			thisUser.authId = res->getInt( "id" );
			thisUser.userId = res->getInt( "userId" );
			thisUser.email = res->getString( "email" );
			thisUser.hashedPassword = res->getString( "hashed_password" );
			thisUser.salt = res->getString( "salt" );
		}
	
		if( thisUser.email == email )
		{
			userIdAsString = std::to_string( thisUser.userId );
			queryAsString = "SELECT last_login, creation_date FROM user WHERE id = '" + userIdAsString + "'";
			res = stmt->executeQuery( queryAsString.c_str() );
			while( res->next() )
			{
				thisUser.creationDate = res->getString( "creation_date" );
				thisUser.lastLogin = res->getString( "last_login" );
			}

			sql::PreparedStatement  *prep_stmt;
			prep_stmt = con->prepareStatement( "UPDATE user SET last_login = CURRENT_TIMESTAMP WHERE id = '" + userIdAsString + "'" );
			prep_stmt->execute();

			delete prep_stmt;

			salt = thisUser.salt;
			creationDate = thisUser.creationDate;
			hashedPassword = thisUser.hashedPassword;
		
			delete stmt;
			delete res;
			delete con;

			return thisUser.userId;
		}

		delete stmt;
		delete res;
		delete con;
	}
	
	catch( sql::SQLException &e ) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;

		return -1;
	}

	return -1;
}
