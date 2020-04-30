/*----------------------------------------------------------------
// Copyright (C) 2014  中央电视台 & 北大方正电子有限公司
// 版权所有.
//
// 文件名称：XmlRpcSocket
// 文件描述：代码文件XmlRpcSocket
//
// 创建者：Founder 
// 创建日期：2009年6月1日
// 版本号：  1.0
// 
// 修改： 无
//----------------------------------------------------------------*/
#include "stdafx.h"
#include "XmlRpcSocket.h"
#include "XmlRpcUtil.h"

using namespace XmlRpc;


#if defined(_WINDOWS)
  
static void initWinSock()
{
  static bool wsInit = false;
  if (! wsInit)
  {
    WORD wVersionRequested = MAKEWORD( 2, 0 );
    WSADATA wsaData;
    WSAStartup(wVersionRequested, &wsaData);
    wsInit = true;
  }
}

#else

#define initWinSock()

#endif // _WINDOWS


// These errors are not considered fatal for an IO operation; the operation will be re-tried.

static inline bool

nonFatalError()

{

	int err0 = XmlRpcSocket::getError();

    return (err0 == EINPROGRESS || err0 == EAGAIN || err0 == EWOULDBLOCK || err0 == EINTR /*|| err0 == ENXIO*/);

}



int
XmlRpcSocket::socket()
{
  initWinSock();
  return (int) ::socket(AF_INET, SOCK_STREAM, 0);
}


void
XmlRpcSocket::close(int fd)
{
  XmlRpcUtil::log(4, "XmlRpcSocket::close: fd %d.", fd);
#if defined(_WINDOWS)
  closesocket(fd);
#else
  ::close(fd);
#endif // _WINDOWS
}




bool
XmlRpcSocket::setNonBlocking(int fd)
{
#if defined(_WINDOWS)
  unsigned long flag = 1;
  return (ioctlsocket((SOCKET)fd, FIONBIO, &flag) == 0);
#else
  return (fcntl(fd, F_SETFL, O_NONBLOCK) == 0);
#endif // _WINDOWS
}


bool
XmlRpcSocket::setReuseAddr(int fd)
{
  // Allow this port to be re-bound immediately so server re-starts are not delayed
  int sflag = 1;
  return (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&sflag, sizeof(sflag)) == 0);
}


// Bind to a specified port
bool 
XmlRpcSocket::bind(int fd, int port)
{
  struct sockaddr_in saddr;
  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = htonl(INADDR_ANY);
  saddr.sin_port = htons((u_short) port);
  return (::bind(fd, (struct sockaddr *)&saddr, sizeof(saddr)) == 0);
}


// Set socket in listen mode
bool 
XmlRpcSocket::listen(int fd, int backlog)
{
  return (::listen(fd, backlog) == 0);
}


int
XmlRpcSocket::accept(int fd, struct sockaddr_in* pAddr/*=NULL*/)
{
  struct sockaddr_in addr;
#if defined(_WINDOWS)
  int
#else
  socklen_t
#endif
    addrlen = sizeof(addr);
  
  if(pAddr){
      return (int) ::accept(fd, (struct sockaddr*)pAddr, &addrlen);
  }
  return (int) ::accept(fd, (struct sockaddr*)&addr, &addrlen);
}


    
// Connect a socket to a server (from a client)
bool
XmlRpcSocket::connect(int fd, std::string& host, int port)
{
  struct sockaddr_in saddr;
  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;

  struct hostent *hp = gethostbyname(host.c_str());
  if (hp == 0) return false;

  saddr.sin_family = hp->h_addrtype;
  memcpy(&saddr.sin_addr, hp->h_addr, hp->h_length);
  saddr.sin_port = htons((u_short) port);

  // For asynch operation, this will return EWOULDBLOCK (windows) or
  // EINPROGRESS (linux) and we just need to wait for the socket to be writable...
  int result = ::connect(fd, (struct sockaddr *)&saddr, sizeof(saddr));
  return result == 0 || nonFatalError();
}	



// Read available text from the specified socket. Returns false on error.
bool 
XmlRpcSocket::nbRead(int fd, std::string& s, bool *eof)
{

	if(NULL!=(&s))
	{
		//HANDLE hSynCmdSignal = CreateEvent(NULL, FALSE, FALSE, NULL);

		const int READ_SIZE = 4096;   // Number of bytes to attempt to read at a time
		char readBuf[READ_SIZE];
		memset(readBuf,0,4096);

		bool wouldBlock = false;
		*eof = false;

		bool	bWaited=false;
		bool	bRet=false;
		int nWaitCount=0;
		//////////////////////////////////////////////////////////////////////////
		//先处理缓冲区中未处理完毕的完整数据报  //wangshuangliu
		int nXmlEndIdx=s.find("</methodCall>");
		int nXmlEndIdx2=s.find("</methodResponse>");
		if(nXmlEndIdx>=0||nXmlEndIdx2>=0)
		{
			return true;
		}
		DWORD time=::GetTickCount();
		//////////////////////////////////////////////////////////////////////////
		while ( ! wouldBlock && ! *eof) {

#if defined(_WINDOWS)
			////add by gongxj at 20121010 Begin
			//int nResultIsSet = 0;
			//int nResSelect = 0;
			//int nErrorCode = 0;
			//fd_set fdRead;
			//struct timeval timeout;

			//FD_ZERO(&fdRead);// 清空要监听的句柄队列
			//FD_SET(fd, &fdRead);// 将该SOCKET 加入到监听读队列
			
			//// 超时设定,每次都需要重置,将自动减为0
			//// 使用1秒超时
			//timeout.tv_sec = 0;// 1秒
			//timeout.tv_usec = 1000;// 1000 对应1ms
			//nResSelect = select(fd, &fdRead, NULL, NULL, &timeout);
			//// 处理select返回
			//if (nResSelect > 0)// 有事件被置位
			//{
			//	nResultIsSet = FD_ISSET(fd, &fdRead);// 获取读事件
			//	if (nResultIsSet <= 0)
			//	{
			//		bRet = true;
			//		break;
			//	}
			//}
			////add by gongxj at 20121010 End

			int n = recv(fd, readBuf, READ_SIZE-1, 0);
			int err=GetLastError();
			//add by gongxj at 20121010 Begin
			if (n<=0)
			{
				if (err==EWOULDBLOCK)
				{
					continue;
				}
			}
			//add by gongxj at 20121010 End
#else
			int n = read(fd, readBuf, READ_SIZE-1);
#endif
			DWORD timeend=::GetTickCount();	
			//Sleep(100);
			DWORD timeend2=::GetTickCount();
			//if ((n > 0) )
			//{
			//	XmlRpcUtil::log(5, "XmlRpcSocket::nbRead: read/recv returned %d.", n);
			//}
			if(n>0)
			{
				XmlRpcUtil::log(5, "XmlRpcSocket::nbRead: read/recv returned %d.", n);
				readBuf[n] = 0;
				s.append(readBuf, n);
				//printf(s.c_str());
				int nIdx=s.find("</methodCall>");
				int nIdx2=s.find("</methodResponse>");
				if( nIdx <0&&nIdx2<0)
				{
					if (!bWaited)
					{
						Sleep(500);
						nWaitCount++;
						if (nWaitCount>=10)
						{
							bWaited=true;
						}
					}
					else
					{
						//已经等待了不再等待
						bRet=false;
						break;

					}
				}
				else
				{
					//::SetEvent(hSynCmdSignal);
					bRet=true;
					break;
				}
			} 
			else if (n == 0||((err !=EINPROGRESS)&&err != EAGAIN&&err != EWOULDBLOCK&&err != EINTR ))
			{
				*eof = true;
			} 
			else if (err == EINPROGRESS || err == EAGAIN || err == EWOULDBLOCK || err == EINTR )
			{
				wouldBlock = true;

			} 
			else 
			{

				return false;   // Error
			}
			//int t=WaitForSingleObject(hSynCmdSignal, 6000);
		}
		if(s!="")
		{
			printf(s.c_str());
		}
		return bRet;
	}
	else 
	{
		return false;
	}
    

}

// Write text to the specified socket. Returns false on error.
bool 
XmlRpcSocket::nbWrite(int fd, std::string& s, int *bytesSoFar)
{
  int nToWrite = int(s.length()) - *bytesSoFar;
  char *sp = const_cast<char*>(s.c_str()) + *bytesSoFar;
  bool wouldBlock = false;

  while ( nToWrite > 0 && ! wouldBlock ) {
#if defined(_WINDOWS)
    int n = send(fd, sp, nToWrite, 0);
#else
    int n = write(fd, sp, nToWrite);
#endif
    

    if (n > 0) {
		XmlRpcUtil::log(5, "XmlRpcSocket::nbWrite: send/write returned %d.", n);
      sp += n;
      *bytesSoFar += n;
      nToWrite -= n;
    } else if (nonFatalError()) {
      wouldBlock = true;
    } else {
      return false;   // Error
    }
  }
  return true;
}


// Returns last errno
int 
XmlRpcSocket::getError()
{
#if defined(_WINDOWS)
  return WSAGetLastError();
#else
  return errno;
#endif
}


// Returns message corresponding to last errno
std::string 
XmlRpcSocket::getErrorMsg()
{
  return getErrorMsg(getError());
}

// Returns message corresponding to errno... well, it should anyway
std::string 
XmlRpcSocket::getErrorMsg(int error)
{
  char err0[60];
  snprintf(err0,sizeof(err0),"error %d", error);
  return std::string(err0);

}


