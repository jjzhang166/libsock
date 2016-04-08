#	SkyFire 2016-04-07	skyfireitdiy@hotmail.com
#简介：libsock为了简化socket编程



#	更新日志：
##	V1.1：
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