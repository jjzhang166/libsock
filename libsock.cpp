#define LIBSOCK_EXPORTS
#include "libsock.h"
#include <thread>
#include <cstdlib>
#include <climits>
#include <string>
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Comdlg32.lib")


static bool ifContinue[USHRT_MAX];

LIBSOCK_API bool SockInit() {
	static bool isInit = false;
	if(isInit)
		return true;
	WORD myVersionRequest;
	WSADATA wsaData;
	myVersionRequest = MAKEWORD(2, 2);
	int err;
	err = WSAStartup(myVersionRequest, &wsaData);
	if(!err) {
		if(LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
			WSACleanup();
			return FALSE;
		}
		memset(ifContinue, FALSE, USHRT_MAX * sizeof(bool));
		isInit = true;
		return TRUE;
	} else {
		return FALSE;
	}
}

LIBSOCK_API void SockClean() {
	WSACleanup();
}



LIBSOCK_API SOCKET MakeSocket(CONNECTTYPE type) {
	SOCKET sock;
	switch(type) {
	case CONNECTTYPE::TCP:
		sock= socket(AF_INET, SOCK_STREAM, 0);
		break;
	case CONNECTTYPE::UDP:
		sock= socket(AF_INET, SOCK_DGRAM, 0);
		break;
	default:
		sock= -1;
		break;
	}
	return sock;
}


LIBSOCK_API bool BindSocket(
	SOCKET sock,
	const unsigned short port
	) {
	SOCKADDR_IN addr;
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if(0 == ::bind(sock, (sockaddr*)&addr, sizeof(addr))) {
		return TRUE;
	} else {
		return FALSE;
	}
}



LIBSOCK_API bool ListenTcpSocket(
	SOCKET sock,
	TCPCONNCALLBACK callBack,
	const int maxClientNum,
	void *otherParam
	) {
	if(::listen(sock, maxClientNum))
		return FALSE;
	ifContinue[sock] = true;
	std::thread([=]() {
		while(ifContinue[sock]) {
			SOCKADDR_IN clientAddr;
			int len = sizeof(clientAddr);
			SOCKET serConn = accept(sock, (SOCKADDR*)&clientAddr, &len);
			std::thread(callBack, serConn, clientAddr,otherParam).detach();
		}
	}).detach();
	return TRUE;
}




LIBSOCK_API bool TCPSend(SOCKET sock, const char*buffer, const int len) {
	int sz = 0;
	if(len != (sz=::send(sock, buffer, len, 0))) {
		return FALSE;
	} else {
		return TRUE;
	}
}


LIBSOCK_API SOCKERR TCPReceive(SOCKET sock, char*buffer, int *len) {
	memset(buffer, 0, *len);
	if(0 < (*len = ::recv(sock, buffer, *len, 0))) {
		return NOERR;
	} else {
		if((*len < 0) && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
			return NOMESSAGE;
		}
		return DISCONNECT;
	}
}


LIBSOCK_API bool UDPSend(SOCKET sock, const char*buffer, const int len, SOCKADDR *addr, const int addrLen) {
	if(len != ::sendto(sock, buffer, len, 0, addr, addrLen)) {
		return FALSE;
	} else {
		return TRUE;
	}
}


LIBSOCK_API bool UDPReceive(SOCKET sock, char*buffer, int *len, SOCKADDR *addr, int addrLen) {
	memset(buffer, 0, *len);
	if(-1 == (*len = ::recvfrom(sock, buffer, *len, 0, addr, &addrLen))) {
		return false;
	} else {
		return true;
	}
}


LIBSOCK_API bool TCPConnect(SOCKET sock, SOCKADDR *addr, int addrLen) {
	return 0 == ::connect(sock, addr, addrLen);
}

LIBSOCK_API bool CloseSock(SOCKET sock) {
	ifContinue[sock] = FALSE;
	return 0 == closesocket(sock);
}

LIBSOCK_API SOCKADDR_IN MakeAddr(const char* ipStr, const unsigned short port) {
	SOCKADDR_IN addr;
	addr.sin_addr.S_un.S_addr = inet_addr(ipStr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	return addr;
}

LIBSOCK_API void DEFAULTTCPCONNCALLBACK(SOCKET clientSock, SOCKADDR_IN addr, void *otherParam) {}

LIBSOCK_API void DEFAULTTCPRECEIVECALLBACK(SOCKET clientSock, SOCKADDR_IN addr, const char * buffer, int len, void *otherParam) {}

LIBSOCK_API void DEFAULTTCPCLOSECALLBACK(SOCKADDR_IN addr, void *otherParam) {}



TCPServer::TCPServer(
	TCPCONNCALLBACK tcpConnCallBack,
	TCPRECEIVECALLBACK tcpReceiveCallBack,
	TCPCLOSECALLBACK tcpCloseCallBack
	) :connCb(tcpConnCallBack), receiveCb(tcpReceiveCallBack), closeCb(tcpCloseCallBack), sock(0) {

}

bool TCPServer::init() {
	return SockInit();
}


bool TCPServer::listen(const unsigned short port, int const  maxClientNum, void *otherParam) {
	sock = MakeSocket(CONNECTTYPE::TCP);
	if(-1 == sock)
		return false;
	if(!BindSocket(sock, port)) {
		return false;
	}
	if(::listen(sock, maxClientNum))
		return FALSE;
	ifContinue[sock] = true;
	std::thread([=]() {
		while(ifContinue[sock]) {
			SOCKADDR_IN clientAddr;
			int len = sizeof(clientAddr);
			SOCKET serConn = accept(sock, (SOCKADDR*)&clientAddr, &len);
			std::thread(connCb, serConn, clientAddr,otherParam).join();
			std::thread([=]() {
				char buffer[MSGBUFFERSIZE];
				int len = 0;
				while(1) {
					len = MSGBUFFERSIZE;
					SOCKERR ret = TCPReceive(serConn, buffer, &len);
					if(ret == NOERR) {
						std::thread(receiveCb, serConn, clientAddr, buffer, len,otherParam).join();
					} else if(ret == DISCONNECT) {
						std::thread(closeCb, clientAddr,otherParam).join();
						CloseSock(serConn);
						break;
					}
				}
			}).detach();
		}
		CloseSock(sock);
	}).detach();
	return true;
}

void TCPServer::close() {
	ifContinue[sock] = false;
}



UDPSocket::UDPSocket(UDPRECEIVECALLBACK UdpReceiveCallBack) : receiveCb(UdpReceiveCallBack), sock(0) {

}

bool UDPSocket::init() {
	return false;
}



bool UDPSocket::recv(const unsigned short port, void *otherParam) {
	sock = MakeSocket(CONNECTTYPE::UDP);
	if(-1 == sock) {
		return false;
	}
	if(!BindSocket(sock, port)) {
		return false;
	}
	ifContinue[sock] = true;
	std::thread([=]() {
		SOCKADDR_IN clientAddr;
		char buffer[MSGBUFFERSIZE];
		int len = 0;
		while(ifContinue[sock]) {
			memset(buffer, 0, MSGBUFFERSIZE);
			len = MSGBUFFERSIZE;
			if(UDPReceive(sock, buffer, &len, (SOCKADDR*)&clientAddr, sizeof(clientAddr))) {
				std::thread(receiveCb, sock, clientAddr, buffer, len,otherParam).join();
			}
		}
		CloseSock(sock);
	}).detach();
	return true;
}

void UDPSocket::close() {
	ifContinue[sock] = false;
}

TCPClient::TCPClient(TCPCONNCALLBACK tcpConnCallBack,
	TCPRECEIVECALLBACK tcpReceiveCallBack,
	TCPCLOSECALLBACK tcpCloseCallBack
	) :connCb(tcpConnCallBack), receiveCb(tcpReceiveCallBack), closeCb(tcpCloseCallBack), sock(0) {

}

bool TCPClient::init() {
	return SockInit();
}

bool TCPClient::connect(const char*ip, const unsigned short port, const unsigned short selfPort,void *otherParam) {
	sock = MakeSocket(CONNECTTYPE::TCP);
	if(-1 == sock)
		return false;
	if(!BindSocket(sock, selfPort)) {
		return false;
	}
	SOCKADDR_IN addr = MakeAddr(ip, port);
	if(!TCPConnect(sock, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN)))
		return FALSE;
	ifContinue[sock] = true;
	std::thread(connCb, sock, addr,otherParam).join();
	std::thread([=]() {
		char buffer[MSGBUFFERSIZE];
		int len = 0;
		while(ifContinue[sock]) {
			len = MSGBUFFERSIZE;
			SOCKERR ret = TCPReceive(sock, buffer, &len);
			if(ret == NOERR) {
				std::thread(receiveCb, sock, addr, buffer, len,otherParam).join();
			} else if(ret == DISCONNECT) {
				std::thread(closeCb, addr,otherParam).join();
				break;
			}
		}
		CloseSock(sock);
	}).detach();
	return true;
}

void TCPClient::close() {
	ifContinue[sock] = false;
}


SOCKET TCPServer::getSock() {
	return sock;
}

SOCKET TCPClient::getSock() {
	return sock;
}

SOCKET UDPSocket::getSock() {
	return sock;
}

TCPFileTrans::TCPFileTrans(SOCKET sock_t) :sock(sock_t){
}


void MakeHrader(char *buffer,const char*fileName,const LONGLONG size){
	memset(buffer, 0, FILEHEADERSIZE);
	memcpy(buffer, &size, sizeof(LONGLONG));
	GetFileTitleA(fileName, buffer + sizeof(LONGLONG), FILEHEADERSIZE - sizeof(LONGLONG));
}



FILETRANSERROR TCPFileTrans::SendFile(const char * fileName) {
	HANDLE file = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(file == INVALID_HANDLE_VALUE) {
		return FILETRANSERROR::FILEOPENERROR;
	}
	LARGE_INTEGER fileSize;
	if(!GetFileSizeEx(file, &fileSize)) {
		return FILETRANSERROR::FILESIZEERROR;
	}
	LONGLONG lFileSize = fileSize.QuadPart;
	char headerBuffer[FILEHEADERSIZE];
	MakeHrader(headerBuffer, fileName, lFileSize);
	if(!TCPSend(sock, headerBuffer, FILEHEADERSIZE)) {
		return FILETRANSERROR::SOCKETERROR;
	}
	char buffer[MSGBUFFERSIZE];
	LONGLONG lastSize = lFileSize;
	DWORD readSize=0;
	while(lastSize) {
		if(ReadFile(file, buffer, MSGBUFFERSIZE, &readSize, NULL)) {
			lastSize -= readSize;
			if(!TCPSend(sock,buffer,readSize)) {
				CloseHandle(file);
				return FILETRANSERROR::SOCKETERROR;
			}
		} else {
			CloseHandle(file);
			return FILETRANSERROR::FILEREADERROR;
		}
	}
	CloseHandle(file);
	return FILETRANSERROR::NOERR;
}


std::string getFileNamePart(const std::string fullFileName) {
	int pos = fullFileName.rfind(".");
	if(pos == std::string::npos) {
		return fullFileName;
	} else {
		return std::string(fullFileName.begin(), fullFileName.begin() + pos);
	}
}

std::string getFileNameExt(const std::string fullFileName) {
	int pos = fullFileName.rfind(".");
	if(pos == std::string::npos) {
		return std::string();
	} else {
		return std::string(fullFileName.begin()+pos+1, fullFileName.end());
	}
}


FILETRANSERROR TCPFileTrans::RecvFile(const char * dir,char*fullPath,int fullPathLen) {
	char buffer[MSGBUFFERSIZE];
	char fileHeader[FILEHEADERSIZE];
	LONGLONG totalRecvSize = 0;
	int recvSize = 0;
	SOCKERR sockErr;
	while(totalRecvSize < FILEHEADERSIZE) {
		recvSize = MSGBUFFERSIZE;
		if((sockErr=TCPReceive(sock, buffer, &recvSize))==SOCKERR::NOERR) {
			if(totalRecvSize + recvSize>=FILEHEADERSIZE) {
				memcpy(fileHeader + totalRecvSize, buffer, FILEHEADERSIZE-totalRecvSize);
			} else {
				memcpy(fileHeader + totalRecvSize, buffer, recvSize);
			}
			totalRecvSize += recvSize;
		} else if(sockErr==SOCKERR::DISCONNECT){
			return FILETRANSERROR::SOCKETERROR;
		} 
	}
	LONGLONG fileSize = 0;
	memcpy(&fileSize, fileHeader, sizeof(LONGLONG));
	char fileName[MAXPATHLEN];
	char dirF[FILEHEADERSIZE];
	if(dir == 0) {
		dir = ".\\";
	}
	strcpy_s(dirF, FILEHEADERSIZE, dir);
	if(strlen(dirF) != 0) {
		if(dirF[strlen(dirF) - 1] != '\\') {
			strcat_s(dirF,FILEHEADERSIZE, "\\");
		}
	}
	strcpy_s(fileName, MAXPATHLEN, dirF);
	strcpy_s(fileName+strlen(dirF), MAXPATHLEN-strlen(dirF), fileHeader + sizeof(LONGLONG));
	HANDLE file = CreateFileA(fileName, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if(file == INVALID_HANDLE_VALUE) {
		std::string fileNameExt = getFileNameExt(fileName);
		std::string fileNamePart = getFileNamePart(fileName);
		for(int i = 0;i < MAXTRYTIME;++i) {
			strcpy_s(fileName, MAXPATHLEN, (fileNamePart+"("+std::to_string(i)+")." + fileNameExt).c_str());
			file = CreateFileA(fileName, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
			if(file != INVALID_HANDLE_VALUE) {
				goto openFileSucc;
			}
		}
		return FILETRANSERROR::FILEOPENERROR;
	}
openFileSucc:
	DWORD writeSize=0;
	if(!WriteFile(file, buffer, totalRecvSize - FILEHEADERSIZE, &writeSize, NULL)) {
		CloseHandle(file);
		return FILETRANSERROR::FILEWRITEERROR;
	}
	if(writeSize != totalRecvSize - FILEHEADERSIZE) {
		CloseHandle(file);
		return FILETRANSERROR::FILEWRITEERROR;
	}
	totalRecvSize -= FILEHEADERSIZE;
	while(totalRecvSize < fileSize) {
		recvSize = MSGBUFFERSIZE;
		if((sockErr=TCPReceive(sock, buffer, &recvSize))==SOCKERR::DISCONNECT) {
			CloseHandle(file);
			return FILETRANSERROR::SOCKETERROR;
		} else if(sockErr==SOCKERR::NOMESSAGE){
			continue;
		}
		if(!WriteFile(file, buffer, recvSize, &writeSize, NULL)) {
			CloseHandle(file);
			return FILETRANSERROR::FILEWRITEERROR;
		}
		if(writeSize != recvSize) {
			CloseHandle(file);
			return FILETRANSERROR::FILEWRITEERROR;
		}
		totalRecvSize += recvSize;
	}
	CloseHandle(file);
	if(fullPathLen != 0) {
		strcpy_s(fullPath, fullPathLen, fileName);
	}
	return FILETRANSERROR::NOERR;
}



TCPFileTransEx::TCPFileTransEx( std::set<SOCKET> sockList_t):sockList(sockList_t) {

}

FILETRANSERROR TCPFileTransEx::SendFile(const char * fileName) {
	HANDLE file = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(file == INVALID_HANDLE_VALUE) {
		return FILETRANSERROR::FILEOPENERROR;
	}
	LARGE_INTEGER fileSize;
	if(!GetFileSizeEx(file, &fileSize)) {
		return FILETRANSERROR::FILESIZEERROR;
	}
	LONGLONG lFileSize = fileSize.QuadPart;
	if(sockList.empty()) {
		CloseHandle(file);
		return FILETRANSERROR::EMPTYLIST;
	}
	errorSockList.clear();
	std::set<SOCKET> sockList = this->sockList;
	char headerBuffer[FILEHEADERSIZE];
	MakeHrader(headerBuffer, fileName, lFileSize);
	for(auto p : sockList) {
		if(!TCPSend(p, headerBuffer, FILEHEADERSIZE)) {
			errorSockList.insert(p);
		}
	}
	for(auto p : errorSockList) {
		sockList.erase(p);
	}
	if(sockList.empty()) {
		CloseHandle(file);
		return FILETRANSERROR::ALLSOCKETERROR;
	}
	char buffer[MSGBUFFERSIZE];
	LONGLONG lastSize = lFileSize;
	DWORD readSize = 0;
	while(lastSize) {
		if(ReadFile(file, buffer, MSGBUFFERSIZE, &readSize, NULL)) {
			lastSize -= readSize;
			for(auto p : sockList) {
				if(!TCPSend(p, buffer, readSize)) {
					errorSockList.insert(p);
				}
			}
			for(auto p : errorSockList) {
				sockList.erase(p);
			}
			if(sockList.empty()) {
				CloseHandle(file);
				return FILETRANSERROR::ALLSOCKETERROR;
			}
		} else {
			CloseHandle(file);
			return FILETRANSERROR::FILEREADERROR;
		}
	}
	CloseHandle(file);
	return FILETRANSERROR::NOERR;
}


void TCPFileTransEx::AddSock(SOCKET sock) {
	sockList.insert(sock);
}

void TCPFileTransEx::RemoveSock(SOCKET sock) {
	sockList.erase(sock);
}

std::set<SOCKET> TCPFileTransEx::GetErrSockList() {
	return errorSockList;
}