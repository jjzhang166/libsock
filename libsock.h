//	SkyFire 2016-04-07	skyfireitdiy@hotmail.com
//简介：libsock为了简化socket编程



//	更新日志：
//	V1.4(2017-03-22)
//		1.增加本地ip传入，应对多网卡情况
//		2.增加参数输入输出标识
//
//	V1.3(2016-04-08):
//		1.新增一对多文件传输
//		2.改变数据包大小
//
//	V1.2(2016-04-08):
//		1.新增文件传输类
//		2.将错误枚举改为错误枚举类
//
//	V1.1(2016-04-08)：
//		1.回调函数添加otherParam，方便参数传递
//			


//	使用方法：
//		TCP服务端面向过程：
//			SockInit()初始化
//			MakeSocket()创建套接字
//			BindSocket()绑定套接字
//			实现TCP连接回调函数（发送与接收在这里实现）
//			ListenSocket()监听套接字
//			使用结束调用CloseSock()
//			SockClean()清理
//
//		TCP客户端面向过程：
//			SockInit()初始化
//			MakeSocket()创建套接字
//			BindSocket()绑定套接字
//			TCPConnect()连接服务器
//			发送接收等操作
//			使用结束调用CloseSock()
//			SockClean()清理
//
//		UDP面向过程：
//			SockInit()初始化
//			MakeSocket()创建套接字
//			BindSocket()绑定套接字
//			发送接收等操作
//			使用结束调用CloseSock()
//			SockClean()清理
//
//		TCP服务器面向对象：
//			实现TCP连接回调函数、TCP接收回调函数、TCP关闭回调函数
//			实例化TCPServer对象
//			调用init()方法
//			调用listen()方法
//			使用结束调用close方法
//
//		TCP客户端面向对象：
//			实现TCP连接回调函数、TCP接收回调函数、TCP关闭回调函数
//			实例化TCPClient对象
//			调用init()方法
//			调用connect()方法
//			使用结束调用close方法	
//
//		UDP面向对象：
//			实现UDP接收回调函数
//			实例化UDPSocket对象
//			调用init()方法
//			调用recv()方法
//			使用结束调用close方法
//
//		文件传输发送端：
//			SockInit()初始化
//			MakeSocket()创建套接字
//			BindSocket()绑定套接字
//			创建TCPFileTrans对象
//			SendFile()发送
//			使用结束调用CloseSock()
//			SockClean()清理
//
//		文件传输接收端：
//			SockInit()初始化
//			MakeSocket()创建套接字
//			BindSocket()绑定套接字（可选）
//			创建TCPFileTrans对象
//			ReceiveFile()接收
//			使用结束调用CloseSock()
//			SockClean()清理
//
//		一对多文件传输
//			SockInit()初始化
//			MakeSocket()创建套接字
//			BindSocket()绑定套接字
//			创建TCPFileTransEx对象
//			监听套接字，将新来的socket加入列表
//			SendFile()发送
//			使用结束调用CloseSock()
//			SockClean()清理



#ifndef LIBSOCK_H_
#define LIBSOCK_H_

#ifdef LIBSOCK_EXPORTS
#define LIBSOCK_API __declspec(dllexport)
#else
#define LIBSOCK_API __declspec(dllimport)
#endif


#include <windows.h>
#include <set>

#define MSGBUFFERSIZE 32768 				//消息缓冲区大小
#define MAXPATHLEN 4096						//文件名最大长度
#define MAXTRYTIME 10000					//文件重名尝试次数

enum class LIBSOCK_API CONNECTTYPE {			//连接协议类型
	TCP,								//TCP连接
	UDP									//UDP连接
};

enum LIBSOCK_API SOCKERR {				//接收错误类型返回值
	NOERR,								//没有错误，接收到消息
	NOMESSAGE,							//没有消息可以接收
	DISCONNECT							//连接已断开
};


//	TCP连接时回调函数原型
//		ClientSocket：	客户端socket
//		addr：			对方地址信息
//		otherParam:		其他参数
typedef void(*TCPCONNCALLBACK)(__in SOCKET clientSock, __in SOCKADDR_IN  addr, __inout void *otherParam);

//	TCP/UDP接收到消息时回调函数原型
//		clientSock:		客户端socket
//		addr:			对方地址信息
//		buffer:			接收到的消息
//		len:			接收到的消息长度
typedef void(*TCPRECEIVECALLBACK)(__in SOCKET clientSock, __in SOCKADDR_IN addr, __in const char *buffer, __in int len, __inout void *otherParam);

//	TCP关闭回调函数原型
//		addr：			对方地址信息
//		otherParam:		其他参数
typedef void(*TCPCLOSECALLBACK)(__in SOCKADDR_IN addr,__inout void *otherParam);


//初始化WinSock
//返回值：成功返回true，失败返回false
LIBSOCK_API bool SockInit();


//清理WinSock
LIBSOCK_API void SockClean();

//	产生SOCKET
//		type：	要产生的SOCKET类型，TCP或UDP
//	返回值：-1表示失败，其他表示成功
LIBSOCK_API SOCKET MakeSocket(__in CONNECTTYPE type);

//	绑定套接字到端口
//		sock：	要绑定的套接字
//		port：	要绑定的端口
//		local_ip 本机IP
//	返回值：成功返回true，失败返回false
LIBSOCK_API bool BindSocket(__in SOCKET sock,__in const unsigned short port,__in const char *local_ip=0);

//	监听TCP套接字
//		sock：			要监听的套接字
//		callBack:		新连接到来回调函数
//		maxClientNum：	最大连接数
//		otherParam：		其他参数
//	返回值：成功返回true，失败返回false
LIBSOCK_API bool ListenTcpSocket(__in SOCKET sock,__in TCPCONNCALLBACK callBack,__in const int maxClientNum,__inout void *otherParam = 0);

//	发送TCP消息
//		sock：		使用的套接字
//		buffer：		消息缓冲
//		len：		消息长度
//	返回值：成功返回true，失败返回false
LIBSOCK_API bool TCPSend(__in SOCKET sock,__in const char*buffer,__in const int len);

//	接收TCP消息
//		sock：		使用的套接字
//		buffer：		消息缓冲
//		len：		消息长度（同时也是缓冲区长度，使用前需要初始化）
//	返回值：接收错误类型
LIBSOCK_API SOCKERR TCPReceive(__in SOCKET sock,__out char*buffer,__out int *len);

//	发送UDP信息
//		sock：		使用的套接字
//		buffer：		消息缓冲区
//		len：		消息长度
//		addr：		对方地址信息
//		addrLen：	地址信息长度
//	返回值：成功返回true，失败返回false
LIBSOCK_API bool UDPSend(__in SOCKET sock,__in const char*buffer,__in const int len,__in SOCKADDR *addr,__in const int addrLen);

//	接收UDP消息
//		sock：		要使用的套接字
//		buffer：		消息缓冲区
//		len：		消息长度（同时也是缓冲区长度，使用前需要初始化）
//		addr：		对方地址信息
//		addrLen：	地址信息长度
//	返回值：成功返回true，失败返回false
LIBSOCK_API bool UDPReceive(__in SOCKET sock, __out char*buffer,__out int *len,__out SOCKADDR *addr,__in int addrLen);



//	加入组播
//		sock：		套接字
//		group_ip:	组播地址
//		local_ip:	本地ip
//	返回值：加入成功返回true，失败返回false
LIBSOCK_API bool UDPMulticastJoinGroup(__in SOCKET sock,__in const char* group_ip,__in const char *local_ip=0);

//	离开组播
//		sock：		套接字
//		group_ip:	组播地址
//		local_ip:	本地ip
//	返回值：离开成功返回true，失败返回false
LIBSOCK_API bool UDPMulticastLeaveGroup(SOCKET sock, const char* group_ip, const char *local_ip=0);


//	TCP连接
//		sock：		使用的套接字
//		addr：		对方地址信息
//		addrLen：	地址信息长度
//	返回值：连接成功返回true，失败返回false
LIBSOCK_API bool TCPConnect(__in SOCKET sock, __out SOCKADDR *addr, __in int addrLen);

//	关闭套接字
//		sock：	要关闭的套接字
//	成功关闭返回true，失败返回false
LIBSOCK_API bool CloseSock(__in SOCKET sock);

//	生成地址信息
//		ipStr：	ip字符串
//		port：	端口
// 返回值：返回地址信息
LIBSOCK_API SOCKADDR_IN MakeAddr(__in const char* ipStr, __in  const unsigned short port);

//默认的连接回调函数（什么都不做）
LIBSOCK_API void DEFAULTTCPCONNCALLBACK(SOCKET clientSock, SOCKADDR_IN addr, void *otherParam = 0);

//默认的接收回调函数（什么都不做）
LIBSOCK_API void DEFAULTTCPRECEIVECALLBACK(SOCKET clientSock, SOCKADDR_IN addr, const char *buffer, int len, void *otherParam = 0);

//默认的关闭回调函数（什么都不做）
LIBSOCK_API void DEFAULTTCPCLOSECALLBACK(SOCKADDR_IN addr, void *otherParam = 0);

#define UDPRECEIVECALLBACK TCPRECEIVECALLBACK
#define DEFAULTUDPRECEIVECALLBACK DEFAULTTCPRECEIVECALLBACK

//	TCP服务器类
class LIBSOCK_API TCPServer {
public:
	//	使用回调函数初始化
	TCPServer(__in TCPCONNCALLBACK tcpConnCallBack = DEFAULTTCPCONNCALLBACK,
		__in TCPRECEIVECALLBACK tcpReceiveCallBack = DEFAULTTCPRECEIVECALLBACK,
		__in TCPCLOSECALLBACK tcpCloseCallBack = DEFAULTTCPCLOSECALLBACK);

	//	初始化套接字
	//	返回值：初始化成功返回true，失败返回false
	static bool init();

	//	监听套接字
	//		port：			监听的端口
	//		maxClientNum：	最大连接数量
	//		otherParam:		其他参数
	//	返回值：成功返回true，失败返回false
	bool listen(__in const unsigned short port, __in  int const maxClientNum, __in  void *otherParam = 0, __in const char *local_ip = 0);

	//	关闭套接字
	void close();

	//	获取套接字
	//	返回值：实例的套接字
	SOCKET getSock();
private:
	TCPCONNCALLBACK connCb;
	TCPRECEIVECALLBACK receiveCb;
	TCPCLOSECALLBACK closeCb;

	SOCKET sock;
};

//	TCP客户端类
class LIBSOCK_API TCPClient {
public:
	// 见TCP客户端类
	TCPClient(
		__in TCPCONNCALLBACK tcpConnCallBack = DEFAULTTCPCONNCALLBACK,
		__in TCPRECEIVECALLBACK tcpReceiveCallBack = DEFAULTTCPRECEIVECALLBACK,
		__in TCPCLOSECALLBACK tcpCloseCallBack = DEFAULTTCPCLOSECALLBACK);

	// 见TCP服务器类
	static bool init();

	//	连接服务器
	//		ip：			ip字符串
	//		port：		远程端口
	//		selfPort：	本地端口
	//		otherParam:	其他参数
	//	返回值：连接成功返回true，失败返回false
	bool connect(__in const char*ip, __in  const unsigned short port, __in  const unsigned short selfPort, __inout void *otherParam = 0,const char *local_ip=0);

	// 见TCP服务器类
	void close();

	// 见TCP服务器类
	SOCKET getSock();
private:
	TCPCONNCALLBACK connCb;
	TCPRECEIVECALLBACK receiveCb;
	TCPCLOSECALLBACK closeCb;
	SOCKET sock;
};

//	UDP套接字类
class LIBSOCK_API UDPSocket {
public:

	// 见TCP服务器类
	UDPSocket(__in UDPRECEIVECALLBACK UdpReceiveCallBack = DEFAULTUDPRECEIVECALLBACK);

	// 见TCP服务器类
	static bool init();

	//	接收消息
	//		port：			本地端口
	//		otherParam:		其他参数
	//		local_ip		本机IP
	//	返回值：成功返回true，失败返回false
	bool recv(__in const unsigned short port, __inout void*otherParam = 0, const char *local_ip = 0);

	//	接收消息
	//		recv_sock		用于接收的套接字
	//		otherParam:		其他参数
	//	返回值：成功返回true，失败返回false
	bool recv(__in SOCKET recv_sock, __inout void*otherParam = 0);

	// 见TCP服务器类
	void close();

	// 见TCP服务器类
	SOCKET getSock();
private:
	TCPRECEIVECALLBACK receiveCb;
	SOCKET sock;
};




#define FILEHEADERSIZE 1024			//文件头大小

enum class LIBSOCK_API FILETRANSERROR {			//文件传输错误
	NOERR,										//无错误
	FILEOPENERROR,								//文件打开错误
	FILESIZEERROR,								//获取文件大小错误
	FILEREADERROR,								//读取文件错误
	FILEWRITEERROR,								//写入文件错误
	SOCKETERROR,								//套接字错误
	EMPTYLIST,									//空列表
	ALLSOCKETERROR								//所有套接字全部失败
};

//TCP文件传输类
class LIBSOCK_API TCPFileTrans {
public:

	//	构造一个文件传输类
	//		sock：		使用的套接字
	TCPFileTrans(__in SOCKET sock);

	//	发送文件
	//		fileName：	要发送的文件路径
	//	返回值：返回FILETRANSERROR::NOERR表示成功，其他为失败
	FILETRANSERROR SendFile(__in const char *fileName);

	//	接收文件
	//		dir：		接收文件存放路径
	//	返回值：返回FILETRANSERROR::NOERR表示成功，其他为失败
	FILETRANSERROR RecvFile(__in const char*dir, __out char *fullPath=0, __in int fullPathLen=0);
private:
	SOCKET sock;
};


class LIBSOCK_API TCPFileTransEx {
public:
	TCPFileTransEx(std::set<SOCKET> sockList=std::set<SOCKET>());

	//	发送文件
	//		fileName：	要发送的文件路径
	//	返回值：返回FILETRANSERROR::NOERR表示成功，其他为失败
	FILETRANSERROR SendFile(__in const char *fileName);

	//	添加socket
	//		sock：	要添加的socket
	void AddSock(__in SOCKET sock);

	//	删除Socket
	//		sock：	要删除的socket
	void RemoveSock(__in SOCKET sock);

	//	获取错误Socket列表
	//	返回值：返回出错的socket集合
	std::set<SOCKET> GetErrSockList();
private:
	std::set<SOCKET> sockList;
	std::set<SOCKET> errorSockList;
};


#endif