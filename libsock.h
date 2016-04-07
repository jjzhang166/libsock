//	SkyFire 2016-04-07	skyfireitdiy@hotmail.com

#ifndef LIBSOCK_H_
#define LIBSOCK_H_

#ifdef LIBSOCK_EXPORTS
#define LIBSOCK_API __declspec(dllexport)
#else
#define LIBSOCK_API __declspec(dllimport)
#endif


#include <windows.h>


enum LIBSOCK_API CONNECTTYPE  {			//����Э������
	TCP,								//TCP����
	UDP									//UDP����
};

 enum LIBSOCK_API SOCKERR  {			//���մ������ͷ���ֵ
	NOERR,								//û�д��󣬽��յ���Ϣ
	NOMESSAGE,							//û����Ϣ���Խ���
	DISCONNECT							//�����ѶϿ�
};


//	TCP����ʱ�ص�����ԭ��
//		ClientSocket��	�ͻ���socket
//			addr		��	�Է���ַ��Ϣ
typedef void(*TCPCONNCALLBACK)(SOCKET clientSock, SOCKADDR_IN addr);

//	TCP/UDP���յ���Ϣʱ�ص�����ԭ��
//		clientSock:		�ͻ���socket
//		addr:			�Է���ַ��Ϣ
//		buffer:			���յ�����Ϣ
//		len:			���յ�����Ϣ����
typedef void(*TCPRECEIVECALLBACK)(SOCKET clientSock, SOCKADDR_IN addr, const char *buffer, int len);

//	TCP�رջص�����ԭ��
//		addr��			�Է���ַ��Ϣ
typedef void(*TCPCLOSECALLBACK)(SOCKADDR_IN addr);


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
//	����ֵ���ɹ�����true��ʧ�ܷ���false
LIBSOCK_API bool ListenTcpSocket(SOCKET sock, TCPCONNCALLBACK callBack, const int maxClientNum);

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
LIBSOCK_API void DEFAULTTCPCONNCALLBACK(SOCKET clientSock, SOCKADDR_IN addr);

//Ĭ�ϵĽ��ջص�������ʲô��������
LIBSOCK_API void DEFAULTTCPRECEIVECALLBACK(SOCKET clientSock, SOCKADDR_IN addr, const char *buffer, int len);

//Ĭ�ϵĹرջص�������ʲô��������
LIBSOCK_API void DEFAULTTCPCLOSECALLBACK(SOCKADDR_IN addr);

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
	//	����ֵ���ɹ�����true��ʧ�ܷ���false
	bool listen(const unsigned short port, int const maxClientNum);

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
	//	����ֵ�����ӳɹ�����true��ʧ�ܷ���false
	bool connect(const char*ip, const unsigned short port, const unsigned short selfPort);

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
	//		port��	���ض˿�
	//	����ֵ���ɹ�����true��ʧ�ܷ���false
	bool recv(const unsigned short port);

	// ��TCP��������
	void close();

	// ��TCP��������
	SOCKET getSock();
private:
	TCPRECEIVECALLBACK receiveCb;
	SOCKET sock;
};


#endif