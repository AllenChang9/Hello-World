/*----------------------------------------------------------------
// Copyright (C) 2014  �������̨ & �������������޹�˾
// ��Ȩ����.
//
// �ļ����ƣ�XmlRpcServer
// �ļ������������ļ�XmlRpcServer
//
// �����ߣ�Founder 
// �������ڣ�2009��6��1��
// �汾�ţ�  1.0
// 
// �޸ģ� ��
//----------------------------------------------------------------*/
#include "stdafx.h"
#include "XmlRpcServer.h"
#include "XmlRpcServerConnection.h"
#include "XmlRpcServerMethod.h"
#include "XmlRpcSocket.h"
#include "XmlRpcUtil.h"
#include "XmlRpcException.h"
using namespace XmlRpc;
////////wsl added
#include <iostream>
#include <cassert>
#include <process.h>
#pragma comment(lib, "ws2_32.lib" )
#define ASSERT assert
#define THREAD HANDLE
#define EVENT  HANDLE
#define CloseThread CloseHandle
#define CloseEvent  CloseHandle
using std::cin;
using std::cout;
using std::endl;



typedef struct tagServerRecv
{
	SOCKET skAccept;		// �ѽ������ӵ�socket
	CRITICAL_SECTION *pcs;  // ͬ������̨������ٽ���  
	EVENT			  e;	// ��֤�ṹ������ֶ��ڽṹ���ֶθı�֮ǰ���俽�����߳��е��ź���
	THREAD t;				// ��ǰ�̵߳��ں˶���
	DWORD  dwThreadID;		// ��ǰ�̵߳�ID
	XmlRpcServer* server;
	XmlRpcServerConnection* con;

}SERVER_RECV, *PSERVER_RECV;
////////////////////////////////////////////////////////////////////////////////////////////////////
/// \fn	static int ServerRecv(LPVOID lParam)
///
/// \brief	�������뽨�����ӵĿͻ��˽���ͨѶ. 
///
/// \author	Shining100
/// \date	2010-05-18
///
/// \param	lParam	 �̺߳�������, ��ϸ��Ϣ������˵��. 
///
/// \return	���Ƿ���0. 
////////////////////////////////////////////////////////////////////////////////////////////////////
static int ServerRecv(LPVOID lParam);
static const int c_iPort = 10001;
//int flag=0;
int ServerRecv(LPVOID lParam)
{
	// �����ṹ������ֶε��߳���
	PSERVER_RECV psr = (PSERVER_RECV)lParam;
	SERVER_RECV  sr = {
		psr->skAccept,
		psr->pcs,
		psr->e,
		psr->t,
		psr->dwThreadID,
		psr->server,
		psr->con
	};
	// �����ź����� ʹ���߳��ܹ������µ�����
	BOOL bRet = FALSE;
	bRet = SetEvent(sr.e);
	ASSERT(bRet);
	/*const int c_iBufLen = 512;
	char szBuf[c_iBufLen + 1] = {'\0'};
	const char c_szPrefix[] = "Server recv:";
	const int c_iPrefLen = strlen(c_szPrefix);
	char szRely[c_iBufLen + 16 + 1] = {'\0'};
	strcpy(szRely, c_szPrefix);
	int iRet = SOCKET_ERROR;*/
    sr.con->setConnection(true);
	for(;;)
	{
		XmlRpcServerConnection* temp = sr.con;
		if (NULL == temp)
		{
			cout << "Connection " << sr.dwThreadID << " shutdown." << endl;
			//break;
			return 0;

		}
		if(sr.con->_connectionState == sr.con->READ_HEADER)
		{
			sr.con->readHeader();
			if(!sr.con->getConnection())
				break;
		}
		if(sr.con->_connectionState == sr.con->READ_REQUEST)
			sr.con->readRequest();
		if(sr.con->_connectionState == sr.con->WRITE_RESPONSE)
			sr.con->writeResponse();

		if(sr.con->_connectionState != sr.con->READ_HEADER)
		{
			cout << "Connection " << sr.dwThreadID << " shutdown." << endl;
			break;
		}   
		Sleep(100);
		 

	}

	return 0;
	/*return (sr.con->_connectionState == sr.con->WRITE_RESPONSE) 
		? XmlRpcDispatch::WritableEvent : XmlRpcDispatch::ReadableEvent;*/
} 

XmlRpcServer::XmlRpcServer()
{
  _introspectionEnabled = false;
  _listMethods = 0;
  _methodHelp = 0;
}


XmlRpcServer::~XmlRpcServer()
{
  this->shutdown();
  _methods.clear();
  delete _listMethods;
  delete _methodHelp;
}


// Add a command to the RPC server
void 
XmlRpcServer::addMethod(XmlRpcServerMethod* method)
{
  _methods[method->name()] = method;
}

// Remove a command from the RPC server
void 
XmlRpcServer::removeMethod(XmlRpcServerMethod* method)
{
  MethodMap::iterator i = _methods.find(method->name());
  if (i != _methods.end())
    _methods.erase(i);
}

// Remove a command from the RPC server by name
void 
XmlRpcServer::removeMethod(const std::string& methodName)
{
  MethodMap::iterator i = _methods.find(methodName);
  if (i != _methods.end())
    _methods.erase(i);
}


// Look up a method by name
XmlRpcServerMethod* 
XmlRpcServer::findMethod(const std::string& name) const
{
  MethodMap::const_iterator i = _methods.find(name);
  if (i == _methods.end())
    return 0;
  return i->second;
}


// Create a socket, bind to the specified port, and
// set it in listen mode to make it available for clients.
bool 
XmlRpcServer::bindAndListen(int port, int backlog /*= 5*/)
{
  
  int fd = XmlRpcSocket::socket();
  if (fd < 0)
  {
    XmlRpcUtil::error("XmlRpcServer::bindAndListen: Could not create socket (%s).", XmlRpcSocket::getErrorMsg().c_str());
    return false;
  }

  this->setfd(fd);

  // Don't block on reads/writes
  if ( ! XmlRpcSocket::setNonBlocking(fd))
  {
    this->close();
    XmlRpcUtil::error("XmlRpcServer::bindAndListen: Could not set socket to non-blocking input mode (%s).", XmlRpcSocket::getErrorMsg().c_str());
    return false;
  }

  // Allow this port to be re-bound immediately so server re-starts are not delayed
  if ( ! XmlRpcSocket::setReuseAddr(fd))
  {
    this->close();
    XmlRpcUtil::error("XmlRpcServer::bindAndListen: Could not set SO_REUSEADDR socket option (%s).", XmlRpcSocket::getErrorMsg().c_str());
    return false;
  }

  // Bind to the specified port on the default interface
  if ( ! XmlRpcSocket::bind(fd, port))
  {
    this->close();
    XmlRpcUtil::error("XmlRpcServer::bindAndListen: Could not bind to specified port (%s).", XmlRpcSocket::getErrorMsg().c_str());
    return false;
  }

  // Set in listening mode
  if ( ! XmlRpcSocket::listen(fd, backlog))
  {
    this->close();
    XmlRpcUtil::error("XmlRpcServer::bindAndListen: Could not set socket in listening mode (%s).", XmlRpcSocket::getErrorMsg().c_str());
    return false;
  }

  
  XmlRpcUtil::log(2, "XmlRpcServer::bindAndListen: server listening on port %d fd %d", port, fd);

  // Notify the dispatcher to listen on this source when we are in work()
  _disp.addSource(this, XmlRpcDispatch::ReadableEvent);

  return true;
}


// Process client requests for the specified time
void 
XmlRpcServer::work(double msTime)
{
  XmlRpcUtil::log(2, "XmlRpcServer::work: waiting for a connection");
  _disp.work(msTime);
}



// Handle input on the server socket by accepting the connection
// and reading the rpc request.
unsigned
XmlRpcServer::handleEvent(unsigned mask)
{
	
  acceptConnection();
  return XmlRpcDispatch::ReadableEvent;		// Continue to monitor this fd
}


// Accept a client connection request and create a connection to
// handle method calls from the client.
void
XmlRpcServer::acceptConnection()
{
  struct sockaddr_in stAddr;
  int s = XmlRpcSocket::accept(this->getfd(), &stAddr);
  int o=this->getfd();
  if(s>=0)
  {
	  XmlRpcUtil::log(2, "XmlRpcServer::acceptConnection: socket %d", s);

  }
  
  if (s < 0)
  {
    //this->close();
    XmlRpcUtil::error("XmlRpcServer::acceptConnection: Could not accept connection (%s).", XmlRpcSocket::getErrorMsg().c_str());
  }
  else if ( ! XmlRpcSocket::setNonBlocking(s))
  {
    XmlRpcSocket::close(s);
    XmlRpcUtil::error("XmlRpcServer::acceptConnection: Could not set socket to non-blocking input mode (%s).", XmlRpcSocket::getErrorMsg().c_str());
  }
  else  // Notify the dispatcher to listen for input on this source when we are in work()
  {
    XmlRpcUtil::log(2, "XmlRpcServer::acceptConnection: creating a connection");
	//_disp.addSource(this->createConnection(s), XmlRpcDispatch::ReadableEvent);
	
	//_disp.addSource(server, XmlRpcDispatch::ReadableEvent);
	//XmlRpcServerConnection* con=new XmlRpcServerConnection(s, this, true);
	//addThread(s,o,this->createConnection(s));
	//this->createConnection(s)
	CreateService(createConnection(s, &stAddr), XmlRpcDispatch::ReadableEvent);
  }  
}


// Create a new connection object for processing requests from a specific client.
XmlRpcServerConnection*
XmlRpcServer::createConnection(int s, struct sockaddr_in* pstAddr /*=NULL*/)
{
  // Specify that the connection object be deleted when it is closed
  return new XmlRpcServerConnection(s, this, true, pstAddr);
}


void 
XmlRpcServer::removeConnection(XmlRpcServerConnection* sc)
{
  _disp.removeSource(sc);
  OnReceiveConnectionClose(getlastfd());
}

void XmlRpcServer::OnReceiveConnectionClose(int fd)
{
}

// Stop processing client requests
void 
XmlRpcServer::exit()
{
  _disp.exit();
}


// Close the server socket file descriptor and stop monitoring connections
void 
XmlRpcServer::shutdown()
{
  // This closes and destroys a ll connections as well as closing this socket
  _disp.clear();
}


// Introspection support
static const std::string LIST_METHODS("system.listMethods");
static const std::string METHOD_HELP("system.methodHelp");
static const std::string MULTICALL("system.multicall");


// List all methods available on a server
class ListMethods : public XmlRpcServerMethod
{
public:
  ListMethods(XmlRpcServer* s) : XmlRpcServerMethod(LIST_METHODS, s) {}

  void execute(XmlRpcValue& params, XmlRpcValue& result, XmlRpcSource* pXmlRpcSource)
  {
    _server->listMethods(result);
  }

  std::string help() { return std::string("List all methods available on a server as an array of strings"); }
};


// Retrieve the help string for a named method
class MethodHelp : public XmlRpcServerMethod
{
public:
  MethodHelp(XmlRpcServer* s) : XmlRpcServerMethod(METHOD_HELP, s) {}

  void execute(XmlRpcValue& params, XmlRpcValue& result, XmlRpcSource* pXmlRpcSource)
  {
    if (params[0].getType() != XmlRpcValue::TypeString)
      throw XmlRpcException(METHOD_HELP + ": Invalid argument type");

    XmlRpcServerMethod* m = _server->findMethod(params[0]);
    if ( ! m)
      throw XmlRpcException(METHOD_HELP + ": Unknown method name");

    result = m->help();
  }

  std::string help() { return std::string("Retrieve the help string for a named method"); }
};

    
// Specify whether introspection is enabled or not. Default is enabled.
void 
XmlRpcServer::enableIntrospection(bool enabled)
{
  if (_introspectionEnabled == enabled)
    return;

  _introspectionEnabled = enabled;

  if (enabled)
  {
    if ( ! _listMethods)
    {
      _listMethods = new ListMethods(this);
      _methodHelp = new MethodHelp(this);
    } else {
      addMethod(_listMethods);
      addMethod(_methodHelp);
    }
  }
  else
  {
    removeMethod(LIST_METHODS);
    removeMethod(METHOD_HELP);
  }
}


void
XmlRpcServer::listMethods(XmlRpcValue& result)
{
  int i = 0;
  result.setSize(_methods.size()+1);
  for (MethodMap::iterator it=_methods.begin(); it != _methods.end(); ++it)
    result[i++] = it->first;

  // Multicall support is built into XmlRpcServerConnection
  result[i] = MULTICALL;
}

void
XmlRpcServerConnection::generateFaultResponse(std::string const& errorMsg, int errorCode)
{
	const char RESPONSE_1[] = 
		"<?xml version=\"1.0\"?>\r\n"
		"<methodResponse><fault>\r\n\t";
	const char RESPONSE_2[] =
		"\r\n</fault></methodResponse>\r\n";

	XmlRpcValue faultStruct;
	faultStruct[FAULTCODE] = errorCode;
	faultStruct[FAULTSTRING] = errorMsg;
	std::string body = RESPONSE_1 + faultStruct.toXml() + RESPONSE_2;
	std::string header = generateHeader(body);

	_response = header + body;
}

void//wangshuangliu����߳�
XmlRpcServer::addThread(int skAccept,SOCKET skListen,XmlRpcServerConnection* con)
{
	int iRet = SOCKET_ERROR;
	// �������̨�����������ڿ����ж���ͻ��˳������ͬʱ�򻺳�������������Ϣ
	// Ϊ�˱�֤���ʱ�ܹ�һ���������������һ���ͻ��˵�������Ϣ�������������
	// ���������Ϣ������̨ʱ������ʹ���ٽ������������߳�
	CRITICAL_SECTION cs;
	InitializeCriticalSection(&cs);
	// ��֤�ṹ������ֶ��ڽṹ���ֶθı�֮ǰ���俽�����߳��е��ź���
	// ��Ϊ���ýṹ�忽�����߳���֮ǰ, �п������µ����ӵ������ı��˽ṹ���ֵ
	// �������Ǳ����ȱ�ֵ֤���������ٽ�������
	EVENT e = NULL;
	e = CreateEvent(NULL, FALSE, FALSE, NULL);
	ASSERT(NULL != e);
	SERVER_RECV sr;
	//�ɹ��������Ӻ󴴽�һ���������߳�Ӧ��ͻ������Է�ֹӦ�ó�����Ϊ�����޷�Ӧ���µĿͻ�����
	//����Ӧ���Ƚ��̹߳����Ա������ܹ����߳�ִ��֮ǰ��ʼ���߳�����Ҫ�Ľṹ������еĸ����ֶ�
	THREAD hThread   = NULL;
	DWORD dwThreadID = 0;
	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ServerRecv, 
		&sr, CREATE_SUSPENDED, &dwThreadID);
	ASSERT(NULL != hThread);
	// ��ʼ���ṹ���ֶ�
	sr.skAccept   = skAccept;
	sr.pcs 		  = &cs;
	sr.e		  = e;
	sr.t		  = hThread;
	sr.dwThreadID = dwThreadID;
	sr.server = this;
	sr.con = con;
	// �����߳�
	DWORD dwRet = ResumeThread(hThread);
	ASSERT(-1 != dwRet);

	//��֤�ṹ�屻�������߳��к���Ӧ���µ�����
	dwRet = WaitForSingleObject (e, INFINITE);
	ASSERT(WAIT_FAILED != dwRet);



	return;

}

void XmlRpcServer::CreateService(XmlRpcSource* pcsSrc, unsigned uMask){
	if(NULL != pcsSrc){
		CRpcService* pcsSrv = new CRpcService(this, pcsSrc, uMask);
		HANDLE hThrd = (HANDLE)_beginthreadex(NULL, 0, XmlRpcServer::StartService, pcsSrv, 0, NULL);
		if(NULL != hThrd){
           CloseHandle(hThrd);
		}else{
			delete pcsSrv;
			delete pcsSrc;
		}
	}
}

unsigned WINAPI XmlRpcServer::StartService(void* lpParam){
	CRpcService* pcsSrv = (CRpcService*)lpParam;
	pcsSrv->work();

	delete pcsSrv;
	return TRUE;
}