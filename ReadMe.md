#	SkyFire 2016-04-07	skyfireitdiy@hotmail.com
#简介：libsock为了简化socket编程



#	更新日志：

##	V1.5(2017-03-22)
###		1.加入组播文件传输

##	V1.4(2017-03-22)
###		1.增加本地ip传入，应对多网卡情况
###		2.增加参数输入输出标识

##	V1.3(2016-04-08):
###		1.新增一对多文件传输
###		2.改变数据包大小

##	V1.2(2016-04-08):
###		1.新增文件传输类
###		2.将错误枚举改为错误枚举类

##	V1.1(2016-04-08)：
###		1.回调函数添加otherParam，方便参数传递



#	使用方法：
##		TCP服务端面向过程：
###			SockInit()初始化
###			MakeSocket()创建套接字
###			BindSocket()绑定套接字
###			实现TCP连接回调函数（发送与接收在这里实现）
###			ListenSocket()监听套接字
###			使用结束调用CloseSock()
###			SockClean()清理

##		TCP客户端面向过程：
###			SockInit()初始化
###			MakeSocket()创建套接字
###			BindSocket()绑定套接字
###			TCPConnect()连接服务器
###			发送接收等操作
###			使用结束调用CloseSock()
###			SockClean()清理

##		UDP面向过程：
###			SockInit()初始化
###			MakeSocket()创建套接字
###			BindSocket()绑定套接字
###			发送接收等操作
###			使用结束调用CloseSock()
###			SockClean()清理
###			UDPMulticastJoinGroup() 加入UDP组播
###			UDPMulticastLeaveGroup() 离开UDP组播


##		TCP服务器面向对象：
###			实现TCP连接回调函数、TCP接收回调函数、TCP关闭回调函数
###			实例化TCPServer对象
###			调用init()方法
###			调用listen()方法
###			使用结束调用close方法

##		TCP客户端面向对象：
###			实现TCP连接回调函数、TCP接收回调函数、TCP关闭回调函数
###			实例化TCPClient对象
###			调用init()方法
###			调用connect()方法
###			使用结束调用close方法	

##		UDP面向对象：
###			实现UDP接收回调函数
###			实例化UDPSocket对象
###			调用init()方法
###			调用recv()方法
###			使用结束调用close方法

##		文件传输发送端：
###			SockInit()初始化
###			MakeSocket()创建套接字
###			BindSocket()绑定套接字
###			创建TCPFileTrans对象
###			SendFile()发送
###			使用结束调用CloseSock()
###			SockClean()清理

##		文件传输接收端：
###			SockInit()初始化
###			MakeSocket()创建套接字
###			BindSocket()绑定套接字
###			创建TCPFileTrans对象
###			ReceiveFile()接收
###			使用结束调用CloseSock()
###			SockClean()清理

##		一对多文件传输
###			SockInit()初始化
###			MakeSocket()创建套接字
###			BindSocket()绑定套接字
###			创建TCPFileTransEx对象
###			监听套接字，将新来的socket加入列表
###			SendFile()发送
###			使用结束调用CloseSock()
###			SockClean()清理