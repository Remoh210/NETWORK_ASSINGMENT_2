#include <iostream>
#include <string>
#include <vector>
#include "cConnection.h"
#include "InitInfo.h"
#include <MessageIDs.h>
#define ARRAY_SIZE 128  
#define SERVER_ADDRESS "127.0.0.1"
using namespace std;

char Answer;

void checkRoom(InitInfo&, string, vector<string>&);
int readCase(string&, InitInfo&, vector<string>&, cConnection&);

int main()
{
    
    InitInfo user;  

	std::cout << " \n\
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::" << std::endl << "\
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::" << std::endl << "\
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::" << std::endl << "\
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::" << std::endl << "\
::::::::::::::::##############                              :::::::::::::::::::" << std::endl << "\
############################  ##############################  :::::::::::::::::" << std::endl << "\
#########################  ######???????????????????????######  :::::::::::::::" << std::endl << "\
=========================  ####??????????()????()?????????####  :::::::::::::::" << std::endl << "\
=========================  ##????()??????????????    ()?????##  ::::    :::::::" << std::endl << "\
------------=============  ##??????????????????  ;;;;  ?????##  ::  ;;;;  :::::" << std::endl << "\
-------------------------  ##??????????()??????  ;;;;;;?????##    ;;;;;;  :::::" << std::endl << "\
-------------------------  ##??????????????????  ;;;;;;         ;;;;;;;;  :::::" << std::endl << "\
++++++++++++-------------  ##??????????????????  ;;;;;;;;;;;;;;;;;;;;;;;  :::::" << std::endl << "\
+++++++++++++++++++++++++  ##????????????()??  ;;;;;;;;;;;;;;;;;;;;;;;;;;;  :::" << std::endl << "\
+++++++++++++++++    ;;;;  ##??()????????????  ;;;;;;@@  ;;;;;;;;@@  ;;;;;  :::" << std::endl << "\
~~~~~~~~~~~~~++++;;;;;;;;  ##????????????????  ;;;;;;    ;;;  ;;;    ;;;;;  :::" << std::endl << "\
~~~~~~~~~~~~~~~  ;;  ~~~~  ####??????()??????  ;;[];;;;;;;;;;;;;;;;;;;;;[]  :::" << std::endl << "\
$$$$$$$$$$$$$~~~~  ~~~~~~  ######?????????????  ;;;;;;              ;;;;  :::::" << std::endl << "\
$$$$$$$$$$$$$$$$$$$$$$$$$    ###################  ;;;;;;;;;;;;;;;;;;;;  :::::::" << std::endl << "\
$$$$$$$$$$$$$$$$$$$$$$$  ;;;;                                       :::::::::::" << std::endl << "\
:::::::::::::$$$$$$$$$$  ;;;;  ::  ;;  ::::::::::::  ;;  ::  ;;;;  ::::::::::::" << std::endl << "\
:::::::::::::::::::::::      ::::::    :::::::::::::     ::::      ::::::::::::" << std::endl << "\
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::" << std::endl << "\
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::" << std::endl << "\
::::::::::::::::NN::::NN::YY::::YY:::AAAAAA:::NN::::NN:::!!::::::::::::::::::::" << std::endl << "\
::::::::::::::::NNNN::NN::YY::::YY::AA::::AA::NNNN::NN:::!!::::::::::::::::::::" << std::endl << "\
::::::::::::::::NNNN::NN::YY::::YY::AA::::AA::NNNN::NN:::!!::::::::::::::::::::" << std::endl << "\
::::::::::::::::NN::NNNN::::YYYY::::AAAAAAAA::NN::NNNN:::!!::::::::::::::::::::" << std::endl << "\
::::::::::::::::NN::NNNN:::::YY:::::AA::::AA::NN::NNNN:::::::::::::::::::::::::" << std::endl << "\
::::::::::::::::NN::::NN:::::YY:::::AA::::AA::NN::::NN:::!!::::::::::::::::::::" << std::endl << "\
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::" << std::endl << "\
::::::::::::::::::::::::YOU:HAVE:DONE:THE:NYAN:::::::::::::::::::::::::::::::::" << std::endl;
	std::cout << "Hello there, stranger. Here it is the console. The console which will demonstrate you the wonderful networking possibilities of a basic command-line interface\n";
	std::cout << "Before then, should the console connect you to the localhost network. It is likely that you're not running a production demo of this thing, so console assumes that the localhost would be the right address to find the server.\n";
	std::cout << "Type \"y\" or \"n\": ";
	std::cin >> Answer;

	if (Answer == 'y' || Answer == 'Y') {
		std::cout << "Okay then, the console was right, it is just for the local demonstration. \nServer 127.0.0.1 will see this client application in just a moment... \n";
		user.serverAddr = SERVER_ADDRESS;
	}
	else if (Answer == 'n' || Answer == 'N') {
		char serverAddr[ARRAY_SIZE];
		std::cout << "No? Who said running this in production was a good idea? Specify the IP address to connect in this case?\n";
		std::cin >> serverAddr;
		user.serverAddr = serverAddr;
	}
	else {
		std::cout << "The console awaits the correct answer. Don't mess with it";
	};

    // Read the user's first name
	std::cout << "Console needs to know your first name. It is important: \n";
	char FirstName[ARRAY_SIZE];
	std::cin >> FirstName;
	user.firstName = FirstName;

    // Try to make a connection
    cConnection myConn;
    myConn.connectToServer(user);

    if(myConn.m_isAlive) {
		cout << "You appear to be connected!\n"
			<< "Typing -j means that you're gonna join to the room number ... (which you also have to specify). \n-l means LEAVE!\n";
		cout << "To authoarize use cammands -a , to register new user use -r \n";
        system("pause");
    } else {
        cout << "Looks like the server is down, no luck this time.\n";
        Sleep(6000);
    }

    string myMessage;            
    string srvMessage;           
    string chatBuffer;              
    vector<string> connectedRooms;  


    while(myConn.m_isAlive) {
        // Get new messages
        srvMessage = myConn.getMessages();

		if (srvMessage != "") {
			chatBuffer += srvMessage;

			cout << chatBuffer;
			cout << ".-\"-._,-'_`-._,-'_`-._,-'_`-._,-'_`-,_,-'_`-,_,-'_`-,_,-'_`-,_,-'_`-,.\n";
			cout << "The console reminds you! : Typing -j means that you're gonna join to the room number ... (which you also have to specify). \n-l means LEAVE!\n";
			cout << "To authoarize use cammands -a , to register new user use -r \n";
			cout << "If the chat isn't updated, use the Enter key, my dear friend!\n";
			cout << ".-\"-._,-'_`-._,-'_`-._,-'_`-._,-'_`-,_,-'_`-,_,-'_`-,_,-'_`-,_,-'_`-,.\n";
		}
        getline(cin, myMessage);

        if(readCase(myMessage, user, connectedRooms, myConn))            
            break;  

    }

    
    myConn.closeConnection();
    return 0;
}



void checkRoom(InitInfo& user, string message, vector<string>& connectedRooms)
{
    // The beggining og the message should be:
    string msgHead = "Chat Server->" + user.firstName + " has connected to ";

    // Is it big enough?
    if(message.size() > msgHead.size()) {
        // Check if the begginings match
        for(int i = 0; i < msgHead.size(); i++)
            if(message.at(i) != msgHead.at(i)) 
                return;

        // Format the string to contain only the room name
        string theRoom = message.substr(msgHead.size(),
                                        message.size() - msgHead.size() - 1);
        connectedRooms.push_back(theRoom);
    }
}


int readCase(string& myMessage,
             InitInfo& user,
             vector<string>& connectedRooms,
             cConnection& myConn)
{


	getline(cin, myMessage);
	// NEW user
	if (myMessage == "-r") {
		myMessage = "";
		cout << "Please type in your email\n";
		string answer;
		cin >> answer;
		user.email = answer;
		cout << "Please type in your password\n";
		cin >> answer;
		while (answer.size() < 6) {
			cout << "Password should be at least 6 characters. Try again!\n";
			cin >> answer;
		}
		user.password = answer;
		myConn.sendMessage(user, CREATE_ACCOUNT, "");
	}

	// AUTHENTICATION
	if (myMessage == "-a") {
		myMessage = "";
		cout << "Type in your email\n";
		string answer;
		cin >> answer;
		user.email = answer;
		cout << "Type in your password\n";
		cin >> answer;
		while (answer.size() < 6) {
			cout << "At least 6 characters, try one more time!\n";
			cin >> answer;
		}
		user.password = answer;
		myConn.sendMessage(user, AUTHENTICATE, "");
	}

	// JOIN
	if (myMessage == "-j") {
		myMessage = "";
		cout << "Specify the ame of the room to connect!\n";
		string answer;
		cin >> answer;
		user.room = answer;
		myConn.sendMessage(user, JOIN_ROOM, "");
	}

	// LEAVE
	if (myMessage == "-l") {
		myMessage = "";
		cout << "Specify the name of the room to leave!\n";
		string answer;
		cin >> answer;
		user.room = answer;
		for (int i = 0; i < connectedRooms.size(); i++) {
			if (connectedRooms.at(i) == user.room)
				connectedRooms.at(i).erase();
		}
		myConn.sendMessage(user, LEAVE_ROOM, "");
	}

	if (myMessage != "") {
		myConn.sendMessage(user, SEND_TEXT, myMessage);
	}

	return 0;

}
