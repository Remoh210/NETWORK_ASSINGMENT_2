#include "cConnection.h"
#include "cBuffer.h"
#include "MessageIDs.h"


cConnection::cConnection()
{
    m_connectSocket = INVALID_SOCKET;
    m_result = NULL;
    m_ptr = NULL;
    m_recvBuffer = new char[DEFAULT_BUFFER_LENGTH];
    m_recvBufferLength = DEFAULT_BUFFER_LENGTH;
}

cConnection::~cConnection()
{
    delete m_recvBuffer;
}

void cConnection::connectToServer(InitInfo info)
{
    m_iResult = WSAStartup(MAKEWORD(2, 2), &m_wsaData);
    if(m_iResult != 0) {
        printf("WSAStartup failed: %d\n", m_iResult);
        return;
    }
    
    ZeroMemory(&m_hints, sizeof(m_hints));
    m_hints.ai_family = AF_UNSPEC;
    m_hints.ai_socktype = SOCK_STREAM;
    m_hints.ai_protocol = IPPROTO_TCP;

    m_iResult = getaddrinfo(info.serverAddr, DEFAULT_PORT, &m_hints, &m_result);
    if(m_iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", m_iResult);
        WSACleanup();
        return;
    }

    for(m_ptr = m_result; m_ptr != NULL; m_ptr = m_ptr->ai_next) {

        m_connectSocket 
            = socket(m_ptr->ai_family, m_ptr->ai_socktype, m_ptr->ai_protocol);
        
        if(m_connectSocket == INVALID_SOCKET) {
            printf("socket() failed with error: %d\n", m_iResult);
            WSACleanup();
            return;
        }

        m_iResult 
            = connect(m_connectSocket, m_ptr->ai_addr, (int)m_ptr->ai_addrlen);
        
        if(m_iResult == SOCKET_ERROR) {
            closesocket(m_connectSocket);
            m_connectSocket = INVALID_SOCKET;
            continue;
        }

        break;
    }

    freeaddrinfo(m_result);

    if(m_connectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server");
        WSACleanup();
        return;
    }

    this->m_isAlive = 1;

}


 //   Send Messese according to msgID

void cConnection::sendMessage(InitInfo info, char msgID, string message)
{
    int packetLength = 0;
    cBuffer* connBuff = NULL;

    switch(msgID) {
    case JOIN_ROOM:
    {

        char userNameLength = info.firstName.size();

        char roomNameLength = info.room.size();

        packetLength = sizeof(int) + sizeof(char) + sizeof(char) + roomNameLength + sizeof(char) + userNameLength;

        if(packetLength > 519) {      
            cout << "Buffer overflow sendind the JOIN message!\n";
            Sleep(3000);
            return;
        }

        connBuff = new cBuffer(packetLength);
        connBuff->WriteInt32LE(packetLength);
        connBuff->WriteChar(JOIN_ROOM);

        connBuff->WriteChar(roomNameLength);
        for(int i = 0; i < roomNameLength; i++)
            connBuff->WriteChar(info.room[i]);

        connBuff->WriteChar(userNameLength);
        for(int i = 0; i < userNameLength; i++)
            connBuff->WriteChar(info.firstName[i]);

    }

    break;	

    case LEAVE_ROOM:
    {
        
        char roomNameLength = info.room.size();
        packetLength = sizeof(int) + sizeof(char) + sizeof(char) + roomNameLength;

        if(packetLength > 262) {         
            cout << "Buffer overflow sendind the LEAVE message!\n";
            Sleep(3000);
            return;
        }

        connBuff = new cBuffer(packetLength);
        connBuff->WriteInt32LE(packetLength);
        connBuff->WriteChar(LEAVE_ROOM);

        connBuff->WriteChar(roomNameLength);
        for(int i = 0; i < roomNameLength; i++)
            connBuff->WriteChar(info.room[i]);

    }

    break;	

    case SEND_TEXT:
    {
       
        short msgLength = message.size();

        packetLength = sizeof(int) + sizeof(char) + sizeof(short) + msgLength;

        if(packetLength > 65542) {       
            cout << "Buffer overflow sendind the TEXT message!\n";
            Sleep(3000);
            return;
        }


        connBuff = new cBuffer(packetLength);
        connBuff->WriteInt32LE(packetLength);
        connBuff->WriteChar(SEND_TEXT);

        connBuff->WriteInt16LE(msgLength);
        for(int i = 0; i < msgLength; i++)
            connBuff->WriteChar(message.at(i));

    }

    break;	

    case CREATE_ACCOUNT:
    {
         
        char emailLength = info.email.size();     
        char passwordLength = info.password.size();
        packetLength = sizeof(int) + sizeof(char) + sizeof(char) + emailLength + sizeof(char) + passwordLength;

        if(packetLength > 519) {        
            cout << "Buffer overflow sendind the ADD message!\n";
            Sleep(3000);
            return;
        }

        connBuff = new cBuffer(packetLength);
        connBuff->WriteInt32LE(packetLength);
        connBuff->WriteChar(CREATE_ACCOUNT);

        connBuff->WriteChar(emailLength);
        for(int i = 0; i < emailLength; i++)
            connBuff->WriteChar(info.email[i]);

        connBuff->WriteChar(passwordLength);
        for(int i = 0; i < passwordLength; i++)
            connBuff->WriteChar(info.password[i]);

    }
    break; 

    case AUTHENTICATE:
    {
        char emailLength = info.email.size();
        char passwordLength = info.password.size();
        packetLength = sizeof(int) + sizeof(char) + sizeof(char) + emailLength + sizeof(char) + passwordLength;
        if(packetLength > 519) {      
            cout << "Buffer overflow sendind the ADD message!\n";
            Sleep(3000);
            return;
        }
        connBuff = new cBuffer(packetLength);
        connBuff->WriteInt32LE(packetLength);
        connBuff->WriteChar(AUTHENTICATE);

        connBuff->WriteChar(emailLength);
        for(int i = 0; i < emailLength; i++)
            connBuff->WriteChar(info.email[i]);

        connBuff->WriteChar(passwordLength);
        for(int i = 0; i < passwordLength; i++)
            connBuff->WriteChar(info.password[i]);
    }
    break;

    case VALIDATE_SERVER:
    {
        
        char serverNameLength = info.firstName.size();
        char hashLength = message.size();
        packetLength = sizeof(int) + sizeof(char) + sizeof(char) + serverNameLength + sizeof(char) + hashLength;

        if(packetLength > 519) {       
            cout << "Buffer overflow sendind the VALIDATE_SERVER message!\n";
            Sleep(3000);
            return;
        }

        connBuff = new cBuffer(packetLength);
        connBuff->WriteInt32LE(packetLength);
        connBuff->WriteChar(VALIDATE_SERVER);

        connBuff->WriteChar(serverNameLength);
        for(int i = 0; i < serverNameLength; i++)
            connBuff->WriteChar(info.firstName[i]);

        connBuff->WriteChar(hashLength);
        for(int i = 0; i < hashLength; i++)
            connBuff->WriteChar(message.at(i));

    }

    break;

    case CREATE_ACCOUNT_WEB_SUCCESS:
    {
        short msgLength = message.size();

        packetLength = sizeof(int) + sizeof(char) + sizeof(short) + msgLength;

        if(packetLength > 65543) {                                    
            cout << "Buffer overflow sending the "
                 << "CREATE_ACCOUNT_WEB_SUCCESS message!\n";
            Sleep(3000);
            return;
        }

        connBuff = new cBuffer(packetLength);
        connBuff->WriteInt32LE(packetLength);
        connBuff->WriteChar(CREATE_ACCOUNT_WEB_SUCCESS);

        connBuff->WriteInt16LE(msgLength);
        for(int i = 0; i < msgLength; i++)
            connBuff->WriteChar(message.at(i));

    }

    break;	

    case CREATE_ACCOUNT_WEB_FAILURE:
    {
        short msgLength = message.size();
        packetLength = sizeof(int) + sizeof(char) + sizeof(short) + msgLength;
        if(packetLength > 65543) {
                                          
            cout << "Buffer overflow sending the "
                 << "CREATE_ACCOUNT_WEB_FAILURE message!\n";
            Sleep(3000);
            return;
        }
        connBuff = new cBuffer(packetLength);
        connBuff->WriteInt32LE(packetLength);
        connBuff->WriteChar(CREATE_ACCOUNT_WEB_FAILURE);

        connBuff->WriteInt16LE(msgLength);
        for(int i = 0; i < msgLength; i++)
            connBuff->WriteChar(message.at(i));

    }

    break;

    case AUTHENTICATE_WEB_SUCCESS:
    {
        short msgLength = message.size();
        packetLength = sizeof(int) + sizeof(char) + sizeof(short) + msgLength;
        if(packetLength > 65543) {     
            cout << "Buffer overflow sending the "
                 << "CREATE_ACCOUNT_WEB_SUCCESS message!\n";
            Sleep(3000);
            return;
        }
        connBuff = new cBuffer(packetLength);
        connBuff->WriteInt32LE(packetLength);
        connBuff->WriteChar(AUTHENTICATE_WEB_SUCCESS);

        connBuff->WriteInt16LE(msgLength);
        for(int i = 0; i < msgLength; i++)
            connBuff->WriteChar(message.at(i));

    }

    break;	

    case AUTHENTICATE_WEB_FAILURE:
    {
        short msgLength = message.size();
        packetLength = sizeof(int) + sizeof(char) + sizeof(short) + msgLength;
        if(packetLength > 65543) { 
                                        
            cout << "Buffer overflow sending the "
                 << "CREATE_ACCOUNT_WEB_FAILURE message!\n";
            Sleep(3000);
            return;
        }
        connBuff = new cBuffer(packetLength);
        connBuff->WriteInt32LE(packetLength);
        connBuff->WriteChar(AUTHENTICATE_WEB_FAILURE);

        connBuff->WriteInt16LE(msgLength);
        for(int i = 0; i < msgLength; i++)
            connBuff->WriteChar(message.at(i));

    }

    break;

    default:
        break;

    }

    m_iResult = send(m_connectSocket, connBuff->getBuffer(), packetLength, 0);
    if(m_iResult == SOCKET_ERROR) {
        printf("socket() failed with error: %d\n", m_iResult);
        closesocket(m_connectSocket);
        WSACleanup();

        delete connBuff;

        return;
    } else {
        delete connBuff;
    }

}


void cConnection::closeConnection()
{
    m_iResult = shutdown(m_connectSocket, SD_SEND);
    if(m_iResult == SOCKET_ERROR) {
        printf("shutdown() failed with error: %d\n", m_iResult);
        closesocket(m_connectSocket);
        WSACleanup();
        return;
    }

    closesocket(m_connectSocket);
    WSACleanup();
}

string cConnection::getMessages()
{
    timeval* mTime = new timeval[1];
    mTime->tv_sec = 1;
    mTime->tv_usec = 0;

    fd_set* listOfSockets = new fd_set[1];
    listOfSockets->fd_count = 1;
    listOfSockets->fd_array[0] = m_connectSocket;

    if(select(0, listOfSockets, NULL, NULL, mTime) > 0) {
        ZeroMemory(m_recvBuffer, m_recvBufferLength);

        m_iResult = recv(m_connectSocket, m_recvBuffer, m_recvBufferLength, 0);
        if(m_iResult == SOCKET_ERROR) {
            closesocket(m_connectSocket);
            WSACleanup();
            return "recv failed with error : " + WSAGetLastError() + '\n';
        }

        cBuffer buff(m_recvBufferLength);   
        buff.setBuffer(m_recvBuffer, m_recvBufferLength);
        int packetLength = 0;              
        int bytesInBuffer;                 
        bytesInBuffer = m_recvBufferLength;
        static string controlStr;          
                                           
        string retStr;                     

        if(controlStr != "") 
            retStr = controlStr;

        while(bytesInBuffer != 0) {
            if(bytesInBuffer < 4 && !packetLength) {
                
                controlStr.push_back(buff.ReadChar());
                bytesInBuffer--;
            
            } else {
                packetLength = buff.ReadInt32LE();
                if(packetLength <= bytesInBuffer && packetLength != 0) {
                    
                    for(int i = 0; i < packetLength - 4; i++) {
                        retStr.push_back(buff.ReadChar());
                    }
                    
                    retStr.push_back('\n');
                    bytesInBuffer -= packetLength;
                
                } else if(packetLength == 0) 
                    break;
                
                bytesInBuffer--;
            }
        }

        return retStr;

    }			
    return "";
}
