#include <cConnection.h>
#include <InitInfo.h>
#include <string>
#include <MessageIDs.h>
#include <authentication.pb.h>
#include "cUserManager.h"

using namespace std;

cUserManager g_userManager; 

int main()
{
    InitInfo authServer;


    cConnection myConn;

	std::cout << "Connect Authentication Server to the localhost network?\n";
	std::cout << "Type \"y\" or \"n\": ";
	char Answer;
	std::cin >> Answer;

	if (Answer == 'y' || Answer == 'Y') {
		std::cout << "Connecting to 127.0.0.1\n";

		authServer.serverAddr = "127.0.0.1";
		authServer.firstName = "Authentication Server";
		myConn.connectToServer(authServer);
	}
	else if (Answer == 'n' || Answer == 'N') {
		char serverAdress[256];
		std::cout << "Specify the IP address to connect \n";
		std::cin >> serverAdress;
		authServer.serverAddr = serverAdress;
		authServer.firstName = "Authentication Server";
		myConn.connectToServer(authServer);
	}
	else {
		std::cout << "The console awaits the correct answer. Don't mess with it";
	};

	//VALIDATION
	myConn.sendMessage(authServer, VALIDATE_SERVER, "TEMP_HASH");

	string chatServerMsg;
	chatServerMsg = myConn.getMessages();

	while (chatServerMsg == "" && myConn.m_isAlive) {

		cout << "The Chat Server did not validate your server yet.\n"
			<< "Try again? (y) or (n):";
		char answer;
		cin >> answer;

		while (answer != 'y' && answer != 'n') {
			cout << "You must type 'y' or 'n':";
			cin >> answer;
		}

		if (answer == 'y') {
			myConn.sendMessage(authServer, VALIDATE_SERVER, "TEMP_HASH");
		}
		else {
			myConn.m_isAlive = false;
			break;
		}

		chatServerMsg = myConn.getMessages();

	}


    cout << chatServerMsg;

    while(myConn.m_isAlive) {
        
        chatServerMsg = myConn.getMessages();
        
        if(chatServerMsg != "") {
            char msgID = chatServerMsg.at(0);
            string pbStr = chatServerMsg.substr(1, chatServerMsg.size() - 2);

            switch(msgID) {
            case CREATE_ACCOUNT_WEB:
            {
                // PARSE PROTO
                authentication::CreateAccountWeb caw;
                caw.ParseFromString(pbStr);

                long long result = g_userManager.createUserAccount(caw.email(), 
                                                                   caw.plaintextpassword());
                
                if(result != -1) {
                    authentication::CreateAccountWebSuccess caws;
                    caws.set_requestid(caw.requestid());
                    caws.set_userid(result);
                    string sendMsg = caws.SerializeAsString();
                    myConn.sendMessage(authServer, CREATE_ACCOUNT_WEB_SUCCESS, sendMsg);
                } else {
                    authentication::CreateAccountWebFailure cawf;
                    cawf.set_requestid(caw.requestid());
                    cawf.set_thereaseon(authentication::CreateAccountWebFailure_reason_ACCOUNT_ALREADY_EXISTS);
                    string sendMsg = cawf.SerializeAsString();
                    myConn.sendMessage(authServer, CREATE_ACCOUNT_WEB_FAILURE, sendMsg);
                }

            }
            break; 

            case AUTHENTICATE_WEB:
            {
				// PARSE PROTO
                authentication::AuthenticateWeb aw;
                aw.ParseFromString(pbStr);

                string mCreationDate;
                long long result 
                    = g_userManager.authenticateAccount(aw.email(),
                                                        aw.plaintextpassword(), 
                                                        mCreationDate);
                if(result != -1) {
                    authentication::AuthenticateWebSuccess aws;
                    aws.set_requestid(aw.requestid());
                    aws.set_userid(result);
                    aws.set_creationdate(mCreationDate);
                    string sendMsg = aws.SerializeAsString();
                    myConn.sendMessage(authServer, 
                                       AUTHENTICATE_WEB_SUCCESS, 
                                       sendMsg);
                } else {
                    authentication::AuthenticateWebFailure awf;
                    awf.set_requestid(aw.requestid());
                    awf.set_thereaseon(authentication::AuthenticateWebFailure_reason_INVALID_CREDENTIALS);
                    string sendMsg = awf.SerializeAsString();
                    myConn.sendMessage(authServer, 
                                       AUTHENTICATE_WEB_FAILURE, 
                                       sendMsg);
                }

            }
            break; 

            default:
                break;

            }

        }

    }

    system("pause");
}