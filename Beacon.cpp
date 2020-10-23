#include <stdio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string.h>

#pragma comment(lib, "Ws2_32.lib")

#define PORT "1337"
#define BUFLEN 1024

int beacon_to_c2(struct addrinfo* result, struct addrinfo* ptr, const char* sendbuf, char recvbuffer[BUFLEN]) {
	SOCKET ConnectSocket = INVALID_SOCKET;

	ptr = result;

	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET)
	{
		printf("socket() failed with code %ld\n", WSAGetLastError());
		return 1;
	}

	int iResult;
	// Try to connect on the socket
	iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
	}

	if (ConnectSocket == INVALID_SOCKET)
	{
		printf("Can't connect to c2\n");
		return 1;
	}

	// Send initial buffer
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR)
	{
		printf("Send failed with code %d\n", WSAGetLastError());
		return 1;
	}

	printf("Bytes sent: %ld\n", iResult);
	printf("Message sent: %s\n", sendbuf);
	int bytesReceived = 0;
	do {
		ZeroMemory(recvbuffer, BUFLEN);
		printf("Size of buffer: %ld\n", BUFLEN);
		iResult = recv(ConnectSocket, recvbuffer, BUFLEN, 0);
		if (iResult > 0)
		{
			printf("Bytes recieved: %d\n", iResult);
			printf("String recieved: %s\n", recvbuffer);
			if (strcmp(recvbuffer, "quit") == 0)
				return 2;
			/*bytesReceived += iResult;
			if (bytesReceived == BUFLEN)
				printf("-------\nFinal String: %s\n", recvbuffer);*/
		}
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed with code %d\n", WSAGetLastError());
	} while (iResult > 0);

	// Close the socket
	closesocket(ConnectSocket);
	return 0;
}

int main(int argc, char const* argv[]) 
{
	WSADATA wsaData;

	int iResult;

	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed with code: %d\n", iResult);
		return 1;
	}

	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo(argv[1], PORT, &hints, &result);
	if (iResult != 0)
	{
		printf("getaddrinfo failed with code %d\n", iResult);
		return 1;
	}

	ptr = result;

	const char* sendbuf = "this is a test";
	char recvbuffer[BUFLEN];

	/*iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed with code %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}*/
	int beaconResult;
	do {
		Sleep(5000);
		beaconResult = beacon_to_c2(result, ptr, sendbuf, recvbuffer);
		if (beaconResult == 1)
			printf("WSAGetlastError: %d\n", WSAGetLastError());
	} while (beaconResult != 2);

	freeaddrinfo(result);
	WSACleanup();
	return 0;
}