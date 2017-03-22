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
	const unsigned short port,
	const char *local_ip
	) {
	SOCKADDR_IN addr;
	if (local_ip == 0) {
		addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	}
	else {
		addr.sin_addr.S_un.S_addr = htonl(inet_addr(local_ip));
	}
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


bool TCPServer::listen(const unsigned short port, int const  maxClientNum, void *otherParam,const char *local_ip) {
	sock = MakeSocket(CONNECTTYPE::TCP);
	if(-1 == sock)
		return false;
	if(!BindSocket(sock, port, local_ip)) {
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



bool UDPSocket::recv(const unsigned short port, void *otherParam,const char *local_ip) {
	sock = MakeSocket(CONNECTTYPE::UDP);
	if(-1 == sock) {
		return false;
	}
	if (!BindSocket(sock, port, local_ip)) {
		return false;
	}
	return recv(sock, otherParam);
}


bool UDPSocket::recv(__in SOCKET recv_sock, __inout void*otherParam /* = 0 */) {
	sock = recv_sock;
	ifContinue[sock] = true;
	std::thread([=]() {
		SOCKADDR_IN clientAddr;
		char buffer[MSGBUFFERSIZE];
		int len = 0;
		while (ifContinue[sock]) {
			memset(buffer, 0, MSGBUFFERSIZE);
			len = MSGBUFFERSIZE;
			if (UDPReceive(sock, buffer, &len, (SOCKADDR*)&clientAddr, sizeof(clientAddr))) {
				std::thread(receiveCb, sock, clientAddr, buffer, len, otherParam).join();
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

bool TCPClient::connect(const char*ip, const unsigned short port, const unsigned short selfPort,void *otherParam,const char *local_ip) {
	sock = MakeSocket(CONNECTTYPE::TCP);
	if(-1 == sock)
		return false;
	if(!BindSocket(sock, selfPort,ip)) {
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


void MakeHeader(FilePkgHead &buffer,const char*fileName,const LONGLONG size){
	memset(&buffer, 0, sizeof(FilePkgHead));
	buffer.size = size;
	GetFileTitleA(fileName, buffer.file_name, MAX_PATH);
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
	FilePkgHead headerBuffer;
	MakeHeader(headerBuffer, fileName, lFileSize);
	if(!TCPSend(sock, (const char*)&headerBuffer, sizeof(FilePkgHead))) {
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
	FilePkgHead fileHeader;
	LONGLONG totalRecvSize = 0;
	int recvSize = 0;
	SOCKERR sockErr;
	while(totalRecvSize < sizeof(FilePkgHead)) {
		recvSize = MSGBUFFERSIZE;
		if((sockErr=TCPReceive(sock, buffer, &recvSize))==SOCKERR::NOERR) {
			if(totalRecvSize + recvSize>= sizeof(FilePkgHead)) {
				memcpy((char*)&fileHeader + totalRecvSize, buffer, sizeof(FilePkgHead)-totalRecvSize);
			} else {
				memcpy((char*)&fileHeader + totalRecvSize, buffer, recvSize);
			}
			totalRecvSize += recvSize;
		} else if(sockErr==SOCKERR::DISCONNECT){
			return FILETRANSERROR::SOCKETERROR;
		} 
	}
	LONGLONG fileSize = 0;
	fileSize = fileHeader.size;
	char fileName[MAXPATHLEN];
	char dirF[MAX_PATH];
	if(dir == 0) {
		dir = ".\\";
	}
	strcpy_s(dirF, MAX_PATH, dir);
	if(strlen(dirF) != 0) {
		if(dirF[strlen(dirF) - 1] != '\\') {
			strcat_s(dirF,MAX_PATH, "\\");
		}
	}
	strcpy_s(fileName, MAXPATHLEN, dirF);
	strcpy_s(fileName+strlen(dirF), MAXPATHLEN-strlen(dirF), fileHeader.file_name);
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
	if(!WriteFile(file, buffer, totalRecvSize - sizeof(FilePkgHead), &writeSize, NULL)) {
		CloseHandle(file);
		return FILETRANSERROR::FILEWRITEERROR;
	}
	if(writeSize != totalRecvSize - sizeof(FilePkgHead)) {
		CloseHandle(file);
		return FILETRANSERROR::FILEWRITEERROR;
	}
	totalRecvSize -= sizeof(FilePkgHead);
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
		CloseHandle(file);
		return FILETRANSERROR::FILESIZEERROR;
	}
	LONGLONG lFileSize = fileSize.QuadPart;
	if(sockList.empty()) {
		CloseHandle(file);
		return FILETRANSERROR::EMPTYLIST;
	}
	errorSockList.clear();
	std::set<SOCKET> sockList = this->sockList;
	FilePkgHead headerBuffer;
	MakeHeader(headerBuffer, fileName, lFileSize);
	for(auto p : sockList) {
		if(!TCPSend(p, (const char*)&headerBuffer, sizeof(FilePkgHead))) {
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



bool UDPMulticastJoinGroup(SOCKET sock, const char* group_ip, const char *local_ip){
	BOOL bMultipleApps = TRUE;
	if (::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&bMultipleApps, sizeof(BOOL)) == SOCKET_ERROR) {
		return false;
	}

	ip_mreq m_mreq;
	m_mreq.imr_multiaddr.s_addr = inet_addr(group_ip);
	if (local_ip != 0) {
		m_mreq.imr_interface.s_addr = inet_addr(local_ip);
	}
	else {
		m_mreq.imr_interface.s_addr = htons(INADDR_ANY);
	}
	return ::setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&m_mreq, sizeof(m_mreq)) != SOCKET_ERROR;
}


bool UDPMulticastLeaveGroup(SOCKET sock, const char* group_ip, const char *local_ip){
	ip_mreq m_mreq;
	m_mreq.imr_multiaddr.s_addr = inet_addr(group_ip);
	if (local_ip != 0) {
		m_mreq.imr_interface.s_addr = inet_addr(local_ip);
	}
	else {
		m_mreq.imr_interface.s_addr = htons(INADDR_ANY);
	}
	return ::setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&m_mreq, sizeof(m_mreq)) != SOCKET_ERROR;
}

UDPMulticastFileTrans::UDPMulticastFileTrans(SOCKET conn_sock):sock(conn_sock)
{
	time_stamp = time(0);
}

FILETRANSERROR UDPMulticastFileTrans::SendFile(const char * fileName, SOCKADDR * addr, const int addrLen)
{
	buf.clear();
	HANDLE file = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE) {
		return FILETRANSERROR::FILEOPENERROR;
	}
	LARGE_INTEGER fileSize;
	if (!GetFileSizeEx(file, &fileSize)) {
		CloseHandle(file);
		return FILETRANSERROR::FILESIZEERROR;
	}
	LONGLONG lFileSize = fileSize.QuadPart;

	FilePkgHead headerBuffer;
	MakeHeader(headerBuffer, fileName, lFileSize);
	
	file_length = headerBuffer.size;

	UDPFilePkg pkg;
	memset(&pkg, 0, sizeof(UDPFilePkg));
	pkg.total_pkg_num = (file_length + PKG_LENGHT - 1) / PKG_LENGHT;
	pkg.curr_pkg_num = 0;
	memcpy_s(&pkg.data, PKG_LENGHT, &headerBuffer, sizeof(FilePkgHead));
	pkg.flag = FLAG_FILE_INFO;
	pkg.time_stamp = time_stamp;
	pkg.curr_pkg_length = sizeof(FilePkgHead);
	if (!UDPSend(sock, (const char*)&pkg, sizeof(UDPFilePkg), addr, addrLen)) {
		CloseHandle(file);
		return FILETRANSERROR::SOCKETERROR;
	}

	std::thread recv_thread(
		[this]() {
		UDPFilePkg pkg;
		int len = sizeof(UDPFilePkg);
		SOCKADDR sockaddr;
		while (UDPReceive(sock, (char*)&pkg, &len, &sockaddr, sizeof(SOCKADDR))) {
			if (pkg.flag == FLAG_LOSS_PKG) {
				this->mu.lock();
				if (buf.find(pkg.curr_pkg_num) != buf.end()) {
					UDPSend(sock, (const char*)&this->buf[pkg.curr_pkg_num], sizeof(UDPFilePkg), &sockaddr, sizeof(sockaddr));
					this->mu.unlock();
				}
				else {
					this->mu.unlock();
					pkg.flag = FLAG_DISCONNECT;
					UDPSend(sock, (const char*)&pkg, sizeof(UDPFilePkg), &sockaddr, sizeof(sockaddr));
				}
				
			}
			else if (pkg.flag == FLAG_COMPLETE) {
				break;
			}
			len = sizeof(UDPFilePkg);
		}
	}
	
	);

	LONGLONG lastSize = lFileSize;
	DWORD readSize = 0;

	unsigned long long curr_pkg = 0;
	
	while (lastSize) {
		if (ReadFile(file, pkg.data, PKG_LENGHT, &readSize, NULL)) {
			lastSize -= readSize;
			pkg.flag = FLAG_FILE_DATA;
			pkg.curr_pkg_num = ++curr_pkg;
			pkg.curr_pkg_length = readSize;

			if (buf.size() == JUMP_PKG) {
				mu.lock();
				buf.erase(buf.begin());
				buf[pkg.curr_pkg_num] = pkg;
				mu.unlock();
			}else{
				mu.lock();
				buf[pkg.curr_pkg_num] = pkg;
				mu.unlock();
			}
			
			if (!UDPSend(sock, (const char*)&pkg, sizeof(UDPFilePkg), addr, addrLen)) {
				CloseHandle(file);
				return FILETRANSERROR::SOCKETERROR;
			}
		}
		else {
			CloseHandle(file);
			return FILETRANSERROR::FILEREADERROR;
		}
	}
	CloseHandle(file);
	Sleep(1000);
	pkg.flag = FLAG_COMPLETE;
	UDPSend(sock, (const char*)&pkg, sizeof(UDPFilePkg), addr, addrLen);
	recv_thread.join();
	return FILETRANSERROR::NOERR;
}



FILETRANSERROR UDPMulticastFileTrans::RecvFile(SOCKET sock, const char * dir, char * fullPath, int fullPathLen)
{
	bool recv_head = false;
	UDPFilePkg pkg;
	int len = sizeof(pkg);
	SOCKADDR sockaddr;
	int curr_pkg = 0;
	HANDLE file = INVALID_HANDLE_VALUE;
	while (UDPReceive(sock, (char*)&pkg, &len, &sockaddr, sizeof(SOCKADDR))) {
		if (pkg.flag == FLAG_FILE_INFO) {
			char fileName[MAXPATHLEN];
			char dirF[MAX_PATH];
			if (dir == 0) {
				dir = ".\\";
			}
			strcpy_s(dirF, MAX_PATH, dir);
			if (strlen(dirF) != 0) {
				if (dirF[strlen(dirF) - 1] != '\\') {
					strcat_s(dirF, MAX_PATH, "\\");
				}
			}
			strcpy_s(fileName, MAXPATHLEN, dirF);
			strcpy_s(fileName + strlen(dirF), MAXPATHLEN - strlen(dirF), ((FilePkgHead*)pkg.data)->file_name);
			file = CreateFileA(fileName, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
			if (file == INVALID_HANDLE_VALUE) {
				std::string fileNameExt = getFileNameExt(fileName);
				std::string fileNamePart = getFileNamePart(fileName);
				for (int i = 0; i < MAXTRYTIME; ++i) {
					strcpy_s(fileName, MAXPATHLEN, (fileNamePart + "(" + std::to_string(i) + ")." + fileNameExt).c_str());
					file = CreateFileA(fileName, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
					if (file != INVALID_HANDLE_VALUE) {
						goto openFileSucc;
					}
				}
				return FILETRANSERROR::FILEOPENERROR;
			openFileSucc:
				++curr_pkg;
			}
		}
		else if (pkg.flag == FLAG_FILE_DATA) {
			if (pkg.curr_pkg_num == curr_pkg) {
				DWORD writeSize = 0;
				if (!WriteFile(file, pkg.data, pkg.curr_pkg_length, &writeSize, NULL)) {
					CloseHandle(file);
					return FILETRANSERROR::FILEWRITEERROR;
				}
				if (writeSize != pkg.curr_pkg_length) {
					CloseHandle(file);
					return FILETRANSERROR::FILEWRITEERROR;
				}
				if (buf[curr_pkg].curr_pkg_num == buf[curr_pkg].total_pkg_num) {
					CloseHandle(file);
					return FILETRANSERROR::NOERR;
				}
				++curr_pkg;
				while (buf.find(curr_pkg)!=buf.end()) {
					if (!WriteFile(file, buf[curr_pkg].data, buf[curr_pkg].curr_pkg_length, &writeSize, NULL)) {
						CloseHandle(file);
						return FILETRANSERROR::FILEWRITEERROR;
					}
					if (writeSize != buf[curr_pkg].curr_pkg_length) {
						CloseHandle(file);
						return FILETRANSERROR::FILEWRITEERROR;
					}
					if (buf[curr_pkg].curr_pkg_num == buf[curr_pkg].total_pkg_num) {
						CloseHandle(file);
						return FILETRANSERROR::NOERR;
					}
					buf.erase(curr_pkg);
					++curr_pkg;
				}
			}
			else if (pkg.curr_pkg_num > curr_pkg) {
				buf[pkg.curr_pkg_num] = pkg;
				pkg.flag = FLAG_LOSS_PKG;
				pkg.curr_pkg_num = curr_pkg;
				UDPSend(sock, (const char*)&pkg, sizeof(UDPFilePkg), &sockaddr, sizeof(sockaddr));
			}
		}
		else if (pkg.flag==FLAG_DISCONNECT) {
			if (file != INVALID_HANDLE_VALUE) {
				CloseHandle(file);
			}
			return FILETRANSERROR::SOCKETERROR;
		}
		len = sizeof(UDPFilePkg);
	}
	return FILETRANSERROR::ALLSOCKETERROR;
}