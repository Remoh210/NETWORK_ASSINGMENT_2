#define UNICODE
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <vector>
#include <cBuffer.h>
#include <MessageIDs.h>
#include <authentication.pb.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "5000"
#define DEFAULT_BUFFER_LENGTH 512

using namespace std;


typedef struct _SOCKET_INFORMATION
{
	_SOCKET_INFORMATION()
		: m_isAuthServer(false)
		, m_isAuthenticated(false)
		, m_hasNewData(false)
		, m_requestedID(false)
		, m_userID(false)
	{

	}

	char m_buffer[DEFAULT_BUFFER_LENGTH];
	WSABUF m_dataBuffer;
	SOCKET m_socket;
	DWORD m_bytesSent;
	DWORD m_bytesReceived;
	vector<string> m_rooms;
	string m_userName;
	bool m_isAuthServer;

	bool m_isAuthenticated;

	bool m_hasNewData;

	long long m_requestedID;

	long long m_userID;

} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;


BOOL createSocketInformation(SOCKET);
void freeSocketInformation(DWORD Index);
void readSocket(LPSOCKET_INFORMATION);
void sendMsg(LPSOCKET_INFORMATION, string msg, string userName);
void sendMsg(LPSOCKET_INFORMATION, string msg);
void treatMessage(LPSOCKET_INFORMATION, string msg);
void msgToRoom(LPSOCKET_INFORMATION, string room, string msg, string sender);


LPSOCKET_INFORMATION g_socketArray[FD_SETSIZE];


DWORD g_totalSockets = 0;
int g_result;
FD_SET g_writeSet;
FD_SET g_readSet;
long long g_requestedID = 0;


int main()
{
	SOCKET listenSocket;
	SOCKET acceptedSocket;
	WSADATA wsaData;
	DWORD socketsIndex;
	DWORD totalOfSockets;
	ULONG nonBlockMode;
	DWORD Flags;
	DWORD sentBytes;
	DWORD receivedBytes;

	struct addrinfo* addrInfoResult;
	struct addrinfo hints;

	addrInfoResult = nullptr;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	g_result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (g_result != 0)
	{
		printf("WSAStartup failed: %d\n", g_result);
		return 1;
	}

	listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET)
	{
		printf("socket() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	getaddrinfo(NULL, DEFAULT_PORT, &hints, &addrInfoResult);
	g_result = bind(listenSocket,
		addrInfoResult->ai_addr,
		(int)addrInfoResult->ai_addrlen);

	if (g_result == SOCKET_ERROR)
	{
		printf("bind() failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(addrInfoResult);
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	if (listen(listenSocket, SOMAXCONN))
	{
		printf("listen() failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	nonBlockMode = 1;
	if (ioctlsocket(listenSocket, FIONBIO, &nonBlockMode) == SOCKET_ERROR)
	{
		printf("ioctlsocket() failed with error %d\n", WSAGetLastError());
		return 1;
	}
	else
	{
		printf("ioctlsocket() is OK!\n");
	}


	while (true)
	{

		FD_ZERO(&g_readSet);
		FD_ZERO(&g_writeSet);

		FD_SET(listenSocket, &g_readSet);

		for (socketsIndex = 0; socketsIndex < g_totalSockets; socketsIndex++)
		{
			if (g_socketArray[socketsIndex]->m_hasNewData)
			{
				FD_SET(g_socketArray[socketsIndex]->m_socket, &g_writeSet);
			}
			else
			{
				FD_SET(g_socketArray[socketsIndex]->m_socket, &g_readSet);
			}
		}



		if ((totalOfSockets = select(0, &g_readSet, &g_writeSet, 0, 0))
			== SOCKET_ERROR)
		{
			printf("select() returned with error %d\n", WSAGetLastError());
			return 1;
		}
		else
		{
			printf("select() is OK!\n");
		}


		if (FD_ISSET(listenSocket, &g_readSet))
		{
			totalOfSockets--;

			if ((acceptedSocket = accept(listenSocket, 0, 0))
				!= INVALID_SOCKET)
			{

				nonBlockMode = 1;
				if (ioctlsocket(acceptedSocket, FIONBIO, &nonBlockMode)
					== SOCKET_ERROR)
				{
					printf("ioctlsocket(FIONBIO) failed with error %d\n",
						WSAGetLastError());
					return 1;
				}
				else
				{
					printf("ioctlsocket(FIONBIO) is OK!\n");
				}


				if (createSocketInformation(acceptedSocket) == FALSE)
				{
					printf("createSocketInformation(acceptedSocket)"
						" failed!\n");
					return 1;
				}
				else
				{
					printf("createSocketInformation() is OK!\n");
				}


			}
			else
			{
				if (WSAGetLastError() != WSAEWOULDBLOCK)
				{
					printf("accept() failed with error %d\n",
						WSAGetLastError());
					return 1;
				}
				else
				{
					printf("accept() is fine!\n");
				}

			}

		}

		for (socketsIndex = 0; totalOfSockets > 0
			&& socketsIndex < g_totalSockets;
			socketsIndex++)
		{

			LPSOCKET_INFORMATION SocketInfo = g_socketArray[socketsIndex];

			if (FD_ISSET(SocketInfo->m_socket, &g_readSet))
			{
				totalOfSockets--;

				SocketInfo->m_dataBuffer.buf = SocketInfo->m_buffer;
				SocketInfo->m_dataBuffer.len = DEFAULT_BUFFER_LENGTH;

				Flags = 0;
				if (WSARecv(SocketInfo->m_socket,
					&(SocketInfo->m_dataBuffer), 1,
					&receivedBytes, &Flags, NULL, NULL)
					== SOCKET_ERROR)
				{

					if (WSAGetLastError() != WSAEWOULDBLOCK)
					{
						printf("WSARecv() failed with error %d\n",
							WSAGetLastError());
						freeSocketInformation(socketsIndex);
					}
					else
					{
						printf("WSARecv() is OK!\n");
					}

					continue;

				}
				else
				{

					SocketInfo->m_bytesReceived = receivedBytes;

					if (receivedBytes == 0)
					{
						freeSocketInformation(socketsIndex);
						continue;
					}
					else
					{
						readSocket(g_socketArray[socketsIndex]);
					}

				}
			}


			if (FD_ISSET(SocketInfo->m_socket, &g_writeSet))
			{
				totalOfSockets--;

				if (WSASend(SocketInfo->m_socket,
					&(SocketInfo->m_dataBuffer),
					1, &sentBytes, 0, NULL, NULL)
					== SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSAEWOULDBLOCK)
					{
						printf("WSASend() failed with error %d\n",
							WSAGetLastError());
						freeSocketInformation(socketsIndex);
					}
					else
					{
						printf("WSASend() is OK!\n");
					}

					continue;

				}
				else
				{
					SocketInfo->m_bytesSent += sentBytes;

					if (SocketInfo->m_bytesSent == SocketInfo->m_bytesReceived)
					{
						SocketInfo->m_bytesSent = false;
						SocketInfo->m_bytesReceived = false;
					}
				}

				SocketInfo->m_hasNewData = false;

			}
		}
	}
}


BOOL createSocketInformation(SOCKET s)
{
	LPSOCKET_INFORMATION SI;

	printf("Accepted socket number %d\n", s);

	if ((SI = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION)))
		== NULL)
	{
		printf("GlobalAlloc() failed with error %d\n", GetLastError());
		return FALSE;
	}
	else
	{
		printf("GlobalAlloc() for SOCKET_INFORMATION is OK!\n");
	}

	SI->m_socket = s;
	SI->m_bytesSent = 0;
	SI->m_bytesReceived = 0;

	g_socketArray[g_totalSockets] = SI;
	g_totalSockets++;
	return(TRUE);
}


void freeSocketInformation(DWORD Index)
{
	LPSOCKET_INFORMATION SI = g_socketArray[Index];
	DWORD i;

	closesocket(SI->m_socket);
	printf("Closing socket number %d\n", SI->m_socket);
	GlobalFree(SI);

	for (i = Index; i < g_totalSockets; i++)
	{
		g_socketArray[i] = g_socketArray[i + 1];
	}

	g_totalSockets--;
}

void readSocket(LPSOCKET_INFORMATION sa)
{
	bool isMsgIncomplete = true;
	int currMsgLength = 0;
	string currBuffer;

	for (int index1 = 0; index1 <= sa->m_bytesReceived; index1++)
	{

		if (isMsgIncomplete)
		{
			if (!currMsgLength)
			{
				currBuffer.push_back(sa->m_dataBuffer.buf[index1]);


				if (currBuffer.size() == 4)
				{
					cBuffer buff(4);
					for (int index2 = 0; index2 < 4; index2++)
					{
						buff.WriteChar(currBuffer.at(index2));
					}
					currMsgLength = buff.ReadInt32LE();

				}

			}
			else
			{
				if (currMsgLength != currBuffer.size())
				{
					currBuffer.push_back(sa->m_dataBuffer.buf[index1]);
				}
				else
				{
					treatMessage(sa, currBuffer);
					currBuffer = "";
					!isMsgIncomplete;
					!currMsgLength;
				}

			}

		}
		else
		{
			currMsgLength = 0;
			!isMsgIncomplete;
			currBuffer.push_back(sa->m_dataBuffer.buf[index1]);
		}
	}
}


void treatMessage(LPSOCKET_INFORMATION sa, string msg)
{
	cBuffer buff(msg.size());

	for (int index = 0; index < msg.size(); index++)
	{
		buff.WriteChar(msg.at(index));
	}


	int packetLength = buff.ReadInt32LE();

	if (packetLength > 65542)
	{
		return;
	}


	char msgID = buff.ReadChar();

	switch (msgID)
	{

	case JOIN_ROOM:
	{
		if (!sa->m_isAuthenticated)
		{
			sendMsg(sa, "You have to authenticate before joing a room!",
				"Chat Server");
		}
		else
		{

			string roomName;
			char roomNameLength = buff.ReadChar();
			for (short index = 0; index < roomNameLength; index++)
			{
				roomName.push_back(buff.ReadChar());
			}


			sa->m_rooms.push_back(roomName);

			string userName;
			char userNameLength = buff.ReadChar();

			for (short index = 0; index < userNameLength; index++)
			{
				userName.push_back(buff.ReadChar());
			}


			sa->m_userName = userName;
			sa->m_userName = userName;

			msgToRoom(sa, roomName, " has connected to ", "Chat Server");

		}

	}
	break;

	case LEAVE_ROOM:
	{

		string roomName;
		short roomNameLength = buff.ReadChar();
		for (short index = 0; index < roomNameLength; index++)
		{
			roomName.push_back(buff.ReadChar());
		}


		for (int index = 0; index < sa->m_rooms.size(); index++)
		{
			if (roomName == sa->m_rooms[index])
			{
				sa->m_rooms[index].erase();
			}

		}


		msgToRoom(sa, roomName, " has disconnected from ", "Chat Server");

	}
	break;

	case SEND_TEXT:
	{

		string msg;
		short msgLength = buff.ReadInt16LE();
		for (short index = 0; index < msgLength; index++)
		{
			msg.push_back(buff.ReadChar());
		}


		for (int index1 = 0; index1 < sa->m_rooms.size(); index1++)
		{
			for (int index2 = 0; index2 < g_totalSockets; index2++)
			{
				for (int index3 = 0; index3 < g_socketArray[index2]->m_rooms.size(); index3++)
				{
					if (sa->m_rooms.at(index1) == g_socketArray[index2]->m_rooms.at(index3)
						&& sa->m_rooms.at(index1) != "")
					{
						sendMsg(g_socketArray[index2], msg, sa->m_userName);
					}
				}
			}
		}

	}
	break;

	case CREATE_ACCOUNT:
	{

		string email;
		char emailLength = buff.ReadChar();
		for (short index = 0; index < emailLength; index++)
		{
			email.push_back(buff.ReadChar());
		}


		string password;
		char passwordLength = buff.ReadChar();
		for (short index = 0; index < passwordLength; index++)
		{
			password.push_back(buff.ReadChar());
		}


		authentication::CreateAccountWeb caw;
		g_requestedID++;
		sa->m_requestedID = g_requestedID;
		caw.set_requestid(g_requestedID);
		caw.set_email(email);
		caw.set_plaintextpassword(password);

		string pbStr;
		pbStr = caw.SerializeAsString();

		int messageLength = 0;

		messageLength = sizeof(char) + pbStr.size();
		cBuffer authBuff(messageLength);
		authBuff.WriteChar(CREATE_ACCOUNT_WEB);
		for (int index = 0; index < pbStr.size(); index++)
		{
			authBuff.WriteChar(pbStr.at(index));
		}


		for (int index = 0; index < g_totalSockets; index++)
		{
			if (g_socketArray[index]->m_isAuthServer)
			{
				sendMsg(g_socketArray[index], authBuff.getBuffer());
			}

		}


	}
	break;

	case AUTHENTICATE:
	{

		string email;
		char emailLength = buff.ReadChar();
		for (short index = 0; index < emailLength; index++)
		{
			email.push_back(buff.ReadChar());
		}


		string password;
		char passwordLength = buff.ReadChar();
		for (short index = 0; index < passwordLength; index++)
		{
			password.push_back(buff.ReadChar());
		}


		authentication::AuthenticateWeb aw;
		g_requestedID++;
		sa->m_requestedID = g_requestedID;
		aw.set_requestid(g_requestedID);
		aw.set_email(email);
		aw.set_plaintextpassword(password);

		string pbStr;
		pbStr = aw.SerializeAsString();

		int messageLength = 0;

		messageLength = sizeof(char) + pbStr.size();
		cBuffer authBuff(messageLength);
		authBuff.WriteChar(AUTHENTICATE_WEB);
		for (int index = 0; index < pbStr.size(); index++)
		{
			authBuff.WriteChar(pbStr.at(index));
		}


		for (int index = 0; index < g_totalSockets; index++)
		{
			if (g_socketArray[index]->m_isAuthServer)
			{
				sendMsg(g_socketArray[index], authBuff.getBuffer());
			}

		}


	}
	break;

	case VALIDATE_SERVER:
	{

		string serverName;
		char serverNameLength = buff.ReadChar();
		for (int index = 0; index < serverNameLength; index++)
		{
			serverName.push_back(buff.ReadChar());
		}


		string hash;
		char hashLength = buff.ReadChar();
		for (int index = 0; index < hashLength; index++)
		{
			hash.push_back(buff.ReadChar());
		}


		if (hash != "TEMP_HASH")
		{
			cout << "Authentication Error!\n";
			return;
		}

		sa->m_isAuthServer = true;

		sa->m_userName = serverName;
		sa->m_userName = serverName;

		cout << "Authentication Server validated!\n";
		sendMsg(sa, "Authentication Server validated!\n", "Chat Server");

	}
	break;

	case CREATE_ACCOUNT_WEB_SUCCESS:
	{
		if (sa->m_isAuthServer)
		{
			string receivedStr;
			short msgLength = buff.ReadInt16LE();
			for (int index = 0; index < msgLength; index++)
			{
				receivedStr.push_back(buff.ReadChar());
			}


			authentication::CreateAccountWebSuccess caws;
			caws.ParseFromString(receivedStr);

			for (int index = 0; index < g_totalSockets; index++)
			{
				if (g_socketArray[index]->m_requestedID == caws.requestid())
				{
					g_socketArray[index]->m_userID = caws.userid();
					sendMsg(g_socketArray[index],
						"The user was created successfully!",
						"Authentication Server");
				}
			}
		}
	}
	break;

	case CREATE_ACCOUNT_WEB_FAILURE:
	{

		if (sa->m_isAuthServer)
		{
			string receivedStr;
			short msgLength = buff.ReadInt16LE();
			for (int index = 0; index < msgLength; index++)
			{
				receivedStr.push_back(buff.ReadChar());
			}


			authentication::CreateAccountWebFailure cawf;
			cawf.ParseFromString(receivedStr);

			for (int index = 0; index < g_totalSockets; index++)
			{
				if (g_socketArray[index]->m_requestedID == cawf.requestid())
				{
					char reason = cawf.thereaseon();

					switch (reason)
					{
						case authentication
						::CreateAccountWebFailure_reason_ACCOUNT_ALREADY_EXISTS:
							sendMsg(g_socketArray[index],
								"The account already exists!",
								"Authentication Server");
							break;

							case authentication
							::CreateAccountWebFailure_reason_INVALID_PASSWORD:
								sendMsg(g_socketArray[index],
									"The Authentication Server did "
									"not accept your password!",
									"Authentication Server");
								break;

								case authentication
								::CreateAccountWebFailure_reason_INTERNAL_SERVER_ERROR:
									sendMsg(g_socketArray[index],
										"There was an Internal Authentication "
										"Server error!",
										"Authentication Server");
									break;

								default:
									sendMsg(g_socketArray[index],
										"There was an unknown error!",
										"Authentication Server");

					}
				}
			}
		}
	}
	break;

	case AUTHENTICATE_WEB_SUCCESS:
	{

		if (sa->m_isAuthServer)
		{

			string receivedStr;
			short msgLength = buff.ReadInt16LE();
			for (int index = 0; index < msgLength; index++)
			{
				receivedStr.push_back(buff.ReadChar());
			}

			authentication::AuthenticateWebSuccess aws;
			aws.ParseFromString(receivedStr);

			for (int index = 0; index < g_totalSockets; index++)
			{
				if (g_socketArray[index]->m_requestedID == aws.requestid())
				{
					g_socketArray[index]->m_isAuthenticated = true;
					sendMsg(g_socketArray[index],
						"Authentication successful, account created on "
						+ aws.creationdate(),
						"Authentication Server");
				}
			}
		}
	}
	break;

	case AUTHENTICATE_WEB_FAILURE:
	{

		if (sa->m_isAuthServer)
		{

			string receivedStr;
			short msgLength = buff.ReadInt16LE();
			for (int index = 0; index < msgLength; index++)
			{
				receivedStr.push_back(buff.ReadChar());
			}

			authentication::AuthenticateWebFailure awf;
			awf.ParseFromString(receivedStr);

			for (int index = 0; index < g_totalSockets; index++)
			{
				if (g_socketArray[index]->m_requestedID == awf.requestid())
				{

					char reason = awf.thereaseon();

					switch (reason)
					{
						case authentication
						::AuthenticateWebFailure_reason_INVALID_CREDENTIALS:
							sendMsg(g_socketArray[index],
								"Invalid credentials!",
								"Authentication Server");
							break;

							case authentication
							::AuthenticateWebFailure_reason_INTERNAL_SERVER_ERROR:
								sendMsg(g_socketArray[index],
									"There was an internal error",
									"Authentication Server");
								break;

							default:
								sendMsg(g_socketArray[index],
									"There was an unknown error!",
									"Authentication Server");

					}
				}
			}
		}
	}
	break;

	default:
		break;

	}
}


void sendMsg(LPSOCKET_INFORMATION sa, string msg, string userName)
{
	string formatedMsg = userName + "->" + msg;

	if (formatedMsg.size() > 65535)
	{
		return;
	}


	int packetLength = sizeof(INT32) + formatedMsg.size();

	cBuffer buff(packetLength);
	buff.WriteInt32LE(packetLength);

	for (int index = 0; index < formatedMsg.size(); index++)
	{
		buff.WriteChar(formatedMsg.at(index));
	}

	buff.WriteChar('\0');

	sa->m_dataBuffer.buf = buff.getBuffer();
	sa->m_dataBuffer.len = packetLength;
	sa->m_hasNewData = true;
}

void sendMsg(LPSOCKET_INFORMATION sa, string msg)
{
	if (msg.size() > 65535)
	{
		return;
	}

	int packetLength = sizeof(INT32) + msg.size();

	cBuffer buff(packetLength);
	buff.WriteInt32LE(packetLength);

	for (int index = 0; index < msg.size(); index++)
	{
		buff.WriteChar(msg.at(index));
	}

	sa->m_dataBuffer.buf = buff.getBuffer();
	sa->m_dataBuffer.len = packetLength;
	sa->m_hasNewData = true;
}

void msgToRoom(LPSOCKET_INFORMATION sa, string room, string msg, string sender)
{
	for (int index1 = 0; index1 < g_totalSockets; index1++)
	{
		for (int index2 = 0; index2 < g_socketArray[index1]->m_rooms.size(); index2++)
		{
			if (g_socketArray[index1]->m_rooms.at(index2) == room)
			{
				sendMsg(g_socketArray[index1], sa->m_userName + msg + room, sender);
			}
		}
	}
}