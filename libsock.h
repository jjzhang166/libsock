//	SkyFire 2016-04-07	skyfireitdiy@hotmail.com
//��飺libsockΪ�˼�socket���



//	������־��
//
//	V1.3(2016-04-08):
//		1.����һ�Զ��ļ�����
//		2.�ı����ݰ���С
//
//	V1.2(2016-04-08):
//		1.�����ļ�������
//		2.������ö�ٸ�Ϊ����ö����
//
//	V1.1(2016-04-08)��
//		1.�ص��������otherParam�������������
//			


//	ʹ�÷�����
//		TCP�����������̣�
//			SockInit()��ʼ��
//			MakeSocket()�����׽���
//			BindSocket()���׽���
//			ʵ��TCP���ӻص����������������������ʵ�֣�
//			ListenSocket()�����׽���
//			ʹ�ý�������CloseSock()
//			SockClean()����
//
//		TCP�ͻ���������̣�
//			SockInit()��ʼ��
//			MakeSocket()�����׽���
//			BindSocket()���׽���
//			TCPConnect()���ӷ�����
//			���ͽ��յȲ���
//			ʹ�ý�������CloseSock()
//			SockClean()����
//
//		UDP������̣�
//			SockInit()��ʼ��
//			MakeSocket()�����׽���
//			BindSocket()���׽���
//			���ͽ��յȲ���
//			ʹ�ý�������CloseSock()
//			SockClean()����
//
//		TCP�������������
//			ʵ��TCP���ӻص�������TCP���ջص�������TCP�رջص�����
//			ʵ����TCPServer����
//			����init()����
//			����listen()����
//			ʹ�ý�������close����
//
//		TCP�ͻ����������
//			ʵ��TCP���ӻص�������TCP���ջص�������TCP�رջص�����
//			ʵ����TCPClient����
//			����init()����
//			����connect()����
//			ʹ�ý�������close����	
//
//		UDP�������
//			ʵ��UDP���ջص�����
//			ʵ����UDPSocket����
//			����init()����
//			����recv()����
//			ʹ�ý�������close����
//
//		�ļ����䷢�Ͷˣ�
//			SockInit()��ʼ��
//			MakeSocket()�����׽���
//			BindSocket()���׽���
//			����TCPFileTrans����
//			SendFile()����
//			ʹ�ý�������CloseSock()
//			SockClean()����
//
//		�ļ�������նˣ�
//			SockInit()��ʼ��
//			MakeSocket()�����׽���
//			BindSocket()���׽��֣���ѡ��
//			����TCPFileTrans����
//			ReceiveFile()����
//			ʹ�ý�������CloseSock()
//			SockClean()����
//
//		һ�Զ��ļ�����
//			SockInit()��ʼ��
//			MakeSocket()�����׽���
//			BindSocket()���׽���
//			����TCPFileTransEx����
//			�����׽��֣���������socket�����б�
//			SendFile()����
//			ʹ�ý�������CloseSock()
//			SockClean()����



#ifndef LIBSOCK_H_
#define LIBSOCK_H_

#ifdef LIBSOCK_EXPORTS
#define LIBSOCK_API __declspec(dllexport)
#else
#define LIBSOCK_API __declspec(dllimport)
#endif


#include <windows.h>
#include <set>

#define MSGBUFFERSIZE 32768 				//��Ϣ��������С
#define MAXPATHLEN 4096						//�ļ�����󳤶�
#define MAXTRYTIME 10000					//�ļ��������Դ���

enum class LIBSOCK_API CONNECTTYPE {			//����Э������
	TCP,								//TCP����
	UDP									//UDP����
};

enum LIBSOCK_API SOCKERR {				//���մ������ͷ���ֵ
	NOERR,								//û�д��󣬽��յ���Ϣ
	NOMESSAGE,							//û����Ϣ���Խ���
	DISCONNECT							//�����ѶϿ�
};


//	TCP����ʱ�ص�����ԭ��
//		ClientSocket��	�ͻ���socket
//		addr��			�Է���ַ��Ϣ
//		otherParam:		��������
typedef void(*TCPCONNCALLBACK)(SOCKET clientSock, SOCKADDR_IN addr, void *otherParam);

//	TCP/UDP���յ���Ϣʱ�ص�����ԭ��
//		clientSock:		�ͻ���socket
//		addr:			�Է���ַ��Ϣ
//		buffer:			���յ�����Ϣ
//		len:			���յ�����Ϣ����
typedef void(*TCPRECEIVECALLBACK)(SOCKET clientSock, SOCKADDR_IN addr, const char *buffer, int len, void *otherParam);

//	TCP�رջص�����ԭ��
//		addr��			�Է���ַ��Ϣ
//		otherParam:		��������
typedef void(*TCPCLOSECALLBACK)(SOCKADDR_IN addr, void *otherParam);


//��ʼ��WinSock
//����ֵ���ɹ�����true��ʧ�ܷ���false
LIBSOCK_API bool SockInit();


//����WinSock
LIBSOCK_API void SockClean();

//	����SOCKET
//		type��	Ҫ������SOCKET���ͣ�TCP��UDP
//	����ֵ��-1��ʾʧ�ܣ�������ʾ�ɹ�
LIBSOCK_API SOCKET MakeSocket(CONNECTTYPE type);

//	���׽��ֵ��˿�
//		sock��	Ҫ�󶨵��׽���
//		port��	Ҫ�󶨵Ķ˿�
//	����ֵ���ɹ�����true��ʧ�ܷ���false
LIBSOCK_API bool BindSocket(SOCKET sock, const unsigned short port);

//	����TCP�׽���
//		sock��			Ҫ�������׽���
//		callBack:		�����ӵ����ص�����
//		maxClientNum��	���������
//		otherParam��		��������
//	����ֵ���ɹ�����true��ʧ�ܷ���false
LIBSOCK_API bool ListenTcpSocket(SOCKET sock, TCPCONNCALLBACK callBack, const int maxClientNum, void *otherParam = 0);

//	����TCP��Ϣ
//		sock��		ʹ�õ��׽���
//		buffer��		��Ϣ����
//		len��		��Ϣ����
//	����ֵ���ɹ�����true��ʧ�ܷ���false
LIBSOCK_API bool TCPSend(SOCKET sock, const char*buffer, const int len);

//	����TCP��Ϣ
//		sock��		ʹ�õ��׽���
//		buffer��		��Ϣ����
//		len��		��Ϣ���ȣ�ͬʱҲ�ǻ��������ȣ�ʹ��ǰ��Ҫ��ʼ����
//	����ֵ�����մ�������
LIBSOCK_API SOCKERR TCPReceive(SOCKET sock, char*buffer, int *len);

//	����UDP��Ϣ
//		sock��		ʹ�õ��׽���
//		buffer��		��Ϣ������
//		len��		��Ϣ����
//		addr��		�Է���ַ��Ϣ
//		addrLen��	��ַ��Ϣ����
//	����ֵ���ɹ�����true��ʧ�ܷ���false
LIBSOCK_API bool UDPSend(SOCKET sock, const char*buffer, const int len, SOCKADDR *addr, const int addrLen);

//	����UDP��Ϣ
//		sock��		Ҫʹ�õ��׽���
//		buffer��		��Ϣ������
//		len��		��Ϣ���ȣ�ͬʱҲ�ǻ��������ȣ�ʹ��ǰ��Ҫ��ʼ����
//		addr��		�Է���ַ��Ϣ
//		addrLen��	��ַ��Ϣ����
//	����ֵ���ɹ�����true��ʧ�ܷ���false
LIBSOCK_API bool UDPReceive(SOCKET sock, char*buffer, int *len, SOCKADDR *addr, int addrLen);

//	TCP����
//		sock��		ʹ�õ��׽���
//		addr��		�Է���ַ��Ϣ
//		addrLen��	��ַ��Ϣ����
//	����ֵ�����ӳɹ�����true��ʧ�ܷ���false
LIBSOCK_API bool TCPConnect(SOCKET sock, SOCKADDR *addr, int addrLen);

//	�ر��׽���
//		sock��	Ҫ�رյ��׽���
//	�ɹ��رշ���true��ʧ�ܷ���false
LIBSOCK_API bool CloseSock(SOCKET sock);

//	���ɵ�ַ��Ϣ
//		ipStr��	ip�ַ���
//		port��	�˿�
// ����ֵ�����ص�ַ��Ϣ
LIBSOCK_API SOCKADDR_IN MakeAddr(const char* ipStr, const unsigned short port);

//Ĭ�ϵ����ӻص�������ʲô��������
LIBSOCK_API void DEFAULTTCPCONNCALLBACK(SOCKET clientSock, SOCKADDR_IN addr, void *otherParam = 0);

//Ĭ�ϵĽ��ջص�������ʲô��������
LIBSOCK_API void DEFAULTTCPRECEIVECALLBACK(SOCKET clientSock, SOCKADDR_IN addr, const char *buffer, int len, void *otherParam = 0);

//Ĭ�ϵĹرջص�������ʲô��������
LIBSOCK_API void DEFAULTTCPCLOSECALLBACK(SOCKADDR_IN addr, void *otherParam = 0);

#define UDPRECEIVECALLBACK TCPRECEIVECALLBACK
#define DEFAULTUDPRECEIVECALLBACK DEFAULTTCPRECEIVECALLBACK

//	TCP��������
class LIBSOCK_API TCPServer {
public:
	//	ʹ�ûص�������ʼ��
	TCPServer(TCPCONNCALLBACK tcpConnCallBack = DEFAULTTCPCONNCALLBACK,
		TCPRECEIVECALLBACK tcpReceiveCallBack = DEFAULTTCPRECEIVECALLBACK,
		TCPCLOSECALLBACK tcpCloseCallBack = DEFAULTTCPCLOSECALLBACK);

	//	��ʼ���׽���
	//	����ֵ����ʼ���ɹ�����true��ʧ�ܷ���false
	static bool init();

	//	�����׽���
	//		port��			�����Ķ˿�
	//		maxClientNum��	�����������
	//		otherParam:		��������
	//	����ֵ���ɹ�����true��ʧ�ܷ���false
	bool listen(const unsigned short port, int const maxClientNum, void *otherParam = 0);

	//	�ر��׽���
	void close();

	//	��ȡ�׽���
	//	����ֵ��ʵ�����׽���
	SOCKET getSock();
private:
	TCPCONNCALLBACK connCb;
	TCPRECEIVECALLBACK receiveCb;
	TCPCLOSECALLBACK closeCb;

	SOCKET sock;
};

//	TCP�ͻ�����
class LIBSOCK_API TCPClient {
public:
	// ��TCP��������
	TCPClient(
		TCPCONNCALLBACK tcpConnCallBack = DEFAULTTCPCONNCALLBACK,
		TCPRECEIVECALLBACK tcpReceiveCallBack = DEFAULTTCPRECEIVECALLBACK,
		TCPCLOSECALLBACK tcpCloseCallBack = DEFAULTTCPCLOSECALLBACK);

	// ��TCP��������
	static bool init();

	//	���ӷ�����
	//		ip��			ip�ַ���
	//		port��		Զ�̶˿�
	//		selfPort��	���ض˿�
	//		otherParam:	��������
	//	����ֵ�����ӳɹ�����true��ʧ�ܷ���false
	bool connect(const char*ip, const unsigned short port, const unsigned short selfPort, void *otherParam = 0);

	// ��TCP��������
	void close();

	// ��TCP��������
	SOCKET getSock();
private:
	TCPCONNCALLBACK connCb;
	TCPRECEIVECALLBACK receiveCb;
	TCPCLOSECALLBACK closeCb;
	SOCKET sock;
};

//	UDP�׽�����
class LIBSOCK_API UDPSocket {
public:

	// ��TCP��������
	UDPSocket(UDPRECEIVECALLBACK UdpReceiveCallBack = DEFAULTUDPRECEIVECALLBACK);

	// ��TCP��������
	static bool init();

	//	������Ϣ
	//		port��			���ض˿�
	//		otherParam:		��������
	//	����ֵ���ɹ�����true��ʧ�ܷ���false
	bool recv(const unsigned short port, void*otherParam = 0);

	// ��TCP��������
	void close();

	// ��TCP��������
	SOCKET getSock();
private:
	TCPRECEIVECALLBACK receiveCb;
	SOCKET sock;
};




#define FILEHEADERSIZE 1024			//�ļ�ͷ��С

enum class LIBSOCK_API FILETRANSERROR {			//�ļ��������
	NOERR,										//�޴���
	FILEOPENERROR,								//�ļ��򿪴���
	FILESIZEERROR,								//��ȡ�ļ���С����
	FILEREADERROR,								//��ȡ�ļ�����
	FILEWRITEERROR,								//д���ļ�����
	SOCKETERROR,								//�׽��ִ���
	EMPTYLIST,									//���б�
	ALLSOCKETERROR								//�����׽���ȫ��ʧ��
};

//TCP�ļ�������
class LIBSOCK_API TCPFileTrans {
public:

	//	����һ���ļ�������
	//		sock��		ʹ�õ��׽���
	TCPFileTrans(SOCKET sock);

	//	�����ļ�
	//		fileName��	Ҫ���͵��ļ�·��
	//	����ֵ������FILETRANSERROR::NOERR��ʾ�ɹ�������Ϊʧ��
	FILETRANSERROR SendFile(const char *fileName);

	//	�����ļ�
	//		dir��		�����ļ����·��
	//	����ֵ������FILETRANSERROR::NOERR��ʾ�ɹ�������Ϊʧ��
	FILETRANSERROR RecvFile(const char*dir,char *fullPath=0,int fullPathLen=0);
private:
	SOCKET sock;
};


class LIBSOCK_API TCPFileTransEx {
public:
	TCPFileTransEx(std::set<SOCKET> sockList=std::set<SOCKET>());

	//	�����ļ�
	//		fileName��	Ҫ���͵��ļ�·��
	//	����ֵ������FILETRANSERROR::NOERR��ʾ�ɹ�������Ϊʧ��
	FILETRANSERROR SendFile(const char *fileName);

	//	���socket
	//		sock��	Ҫ��ӵ�socket
	void AddSock(SOCKET sock);

	//	ɾ��Socket
	//		sock��	Ҫɾ����socket
	void RemoveSock(SOCKET sock);

	//	��ȡ����Socket�б�
	//	����ֵ�����س����socket����
	std::set<SOCKET> GetErrSockList();
private:
	std::set<SOCKET> sockList;
	std::set<SOCKET> errorSockList;
};


#endif