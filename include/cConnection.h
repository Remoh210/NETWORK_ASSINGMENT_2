#ifndef _cConnection_HG
#define _cConnection_HG

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <algorithm> 
#include <string>
#include <Windows.h>

#include "InitInfo.h"

#define UNICODE
#define WIN32_LEAN_AND_MEAN

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_PORT "5000"
#define DEFAULT_BUFFER_LENGTH 512
#define MAX_ARRAY_SIZE 256

using namespace std;


class cConnection {
public:
    cConnection();
    ~cConnection();

    WSADATA m_wsaData;            
    SOCKET m_connectSocket;       
    struct addrinfo* m_result;    
    struct addrinfo* m_ptr;       
    struct addrinfo m_hints;      
    int m_iResult;                
    char* m_recvBuffer;           
    int m_recvBufferLength;       
    bool m_isAlive = 0;           


    void connectToServer(InitInfo info);
    void sendMessage(InitInfo info, char msgID, string message);
    void closeConnection();

    string getMessages();

private:

};
#endif 

