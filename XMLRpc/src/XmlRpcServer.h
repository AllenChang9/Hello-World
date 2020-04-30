/*----------------------------------------------------------------
// Copyright (C) 2014  �������̨ & �������������޹�˾
// ��Ȩ����.
//
// �ļ����ƣ�XmlRpcServer
// �ļ�������ͷ�ļ�XmlRpcServer
//
// �����ߣ�Founder 
// �������ڣ�2009��6��1��
// �汾�ţ�  1.0
// 
// �޸ģ� ��
//----------------------------------------------------------------*/

#ifndef _XMLRPCSERVER_H_
#define _XMLRPCSERVER_H_
//
// XmlRpc++ Copyright (c) 2002-2003 by Chris Morley
//
#if defined(_MSC_VER)
# pragma warning(disable:4786)    // identifier was truncated in debug info
#endif

#ifndef MAKEDEPEND
# include <map>
# include <string>
#endif


//#include <wtypes.h>
#include "XmlRpcDispatch.h"
#include "XmlRpcSource.h"
#include "winsock2.h"
namespace XmlRpc {


  // An abstract class supporting XML RPC methods
  class XmlRpcServerMethod;

  // Class representing connections to specific clients
  class XmlRpcServerConnection;

  // Class representing argument and result values
  class XmlRpcValue;


  //! A class to handle XML RPC requests
  class XmlRpcServer : public XmlRpcSource {
  public:
    //! Create a server object.
    XmlRpcServer();
    //! Destructor.
    virtual ~XmlRpcServer();

    //! Specify whether introspection is enabled or not. Default is not enabled.
    void enableIntrospection(bool enabled=true);

    //! Add a command to the RPC server
    void addMethod(XmlRpcServerMethod* method);

    //! Remove a command from the RPC server
    void removeMethod(XmlRpcServerMethod* method);

    //! Remove a command from the RPC server by name
    void removeMethod(const std::string& methodName);

    //! Look up a method by name
    XmlRpcServerMethod* findMethod(const std::string& name) const;

    //! Create a socket, bind to the specified port, and
    //! set it in listen mode to make it available for clients.
    bool bindAndListen(int port, int backlog = 5);

    //! Process client requests for the specified time
    void work(double msTime);

    //! Temporarily stop processing client requests and exit the work() method.
    void exit();

    //! Close all connections with clients and the socket file descriptor
    void shutdown();

    //! Introspection support
    void listMethods(XmlRpcValue& result);


    // XmlRpcSource interface implementation

    //! Handle client connection requests
    virtual unsigned handleEvent(unsigned eventType);

    //! Remove a connection from the dispatcher
    virtual void removeConnection(XmlRpcServerConnection*);

	// ���ӽӿ�
	virtual void OnReceiveConnectionClose(int fd);

	// Event dispatcher
	//static XmlRpcDispatch _disp;
	//���߳�wsl
	void addThread(int,SOCKET,XmlRpcServerConnection*);

  protected:

    //! Accept a client connection request
    virtual void acceptConnection();

    //! Create a new connection object for processing requests from a specific client.
    virtual XmlRpcServerConnection* createConnection(int socket, struct sockaddr_in* pstAddr=NULL);

    // Whether the introspection API is supported by this server
    bool _introspectionEnabled;

    // Event dispatcher
    XmlRpcDispatch _disp;

    // Collection of methods. This could be a set keyed on method name if we wanted...
    typedef std::map< std::string, XmlRpcServerMethod* > MethodMap;
    MethodMap _methods;

    // system methods
    XmlRpcServerMethod* _listMethods;
    XmlRpcServerMethod* _methodHelp;

	/*֧�ֲ������� Added by Chaolong 2012-1115*/
	/*--------------Begin-----------*/
	void CreateService(XmlRpcSource* pcsSrc, unsigned uMask);
   
	/*Embeded class supported Service*/
	class CRpcService{
	public:
		CRpcService(XmlRpcServer* pcsSrv, XmlRpcSource* pcsSrc, unsigned uMask){
			m_pcsSrv = pcsSrv;
			m_csDisp.addSource(pcsSrc, uMask);
		}

		void work(){
           m_csDisp.work(-1);
		}

	protected:
		XmlRpcServer* m_pcsSrv;
		XmlRpcDispatch m_csDisp;
	};

public:
    static unsigned WINAPI StartService(void* lpParam);
	/*--------------End ------------*/

  };
} // namespace XmlRpc

#endif //_XMLRPCSERVER_H_
