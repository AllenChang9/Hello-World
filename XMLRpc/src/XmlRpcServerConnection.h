/*----------------------------------------------------------------
// Copyright (C) 2014  中央电视台 & 北大方正电子有限公司
// 版权所有.
//
// 文件名称：XmlRpcServerConnection
// 文件描述：头文件XmlRpcServerConnection
//
// 创建者：Founder 
// 创建日期：2009年6月1日
// 版本号：  1.0
// 
// 修改： 无
//----------------------------------------------------------------*/
#ifndef _XMLRPCSERVERCONNECTION_H_
#define _XMLRPCSERVERCONNECTION_H_
//
// XmlRpc++ Copyright (c) 2002-2003 by Chris Morley
//
#if defined(_MSC_VER)
# pragma warning(disable:4786)    // identifier was truncated in debug info
#endif

#ifndef MAKEDEPEND
# include <string>
#endif

#include "XmlRpcValue.h"
#include "XmlRpcSource.h"
#include "xmlrpcsocket.h"

namespace XmlRpc {


  // The server waits for client connections and provides methods
  class XmlRpcServer;
  class XmlRpcServerMethod;

  //! A class to handle XML RPC requests from a particular client
  class XmlRpcServerConnection : public XmlRpcSource {

  public:
    // Static data
    static const char METHODNAME_TAG[];
    static const char PARAMS_TAG[];
    static const char PARAMS_ETAG[];
    static const char PARAM_TAG[];
    static const char PARAM_ETAG[];

    static const std::string SYSTEM_MULTICALL;
    static const std::string METHODNAME;
    static const std::string PARAMS;

    static const std::string FAULTCODE;
    static const std::string FAULTSTRING;
 
    //! Constructor
    XmlRpcServerConnection(int fd, XmlRpcServer* server, bool deleteOnClose = false, struct sockaddr_in* pstAddr=NULL);
    //! Destructor
    virtual ~XmlRpcServerConnection();

    // XmlRpcSource interface implementation
    //! Handle IO on the client connection socket.
    //!   @param eventType Type of IO event that occurred. @see XmlRpcDispatch::EventType.
    virtual unsigned handleEvent(unsigned eventType);

	// 验证合法
	void SignIn(bool bSignIn) { _bSignIn = bSignIn; }
	bool IsSignIn() { return _bSignIn; }
	bool readHeader();
	bool readRequest();
	bool writeResponse();
	// Possible IO states for the connection
	enum ServerConnectionState { READ_HEADER, READ_REQUEST, WRITE_RESPONSE };
	ServerConnectionState _connectionState;
	//ServerConnectionState getState(){return _connectionState;}
	bool connection;
	void setConnection(bool b){connection = b;}
	bool getConnection(){return connection;}

	/*client sockaddr*/
	struct sockaddr_in* getSockaddr(){
		return &m_stAddr;
	};
  protected:

    

    // Parses the request, runs the method, generates the response xml.
    virtual void executeRequest();

    // Parse the methodName and parameters from the request.
    std::string parseRequest(XmlRpcValue& params);

    // Execute a named method with the specified params.
    bool executeMethod(const std::string& methodName, XmlRpcValue& params, XmlRpcValue& result);

    // Execute multiple calls and return the results in an array.
    bool executeMulticall(const std::string& methodName, XmlRpcValue& params, XmlRpcValue& result);

    // Construct a response from the result XML.
    void generateResponse(std::string const& resultXml);
    void generateFaultResponse(std::string const& msg, int errorCode = -1);
    std::string generateHeader(std::string const& body);


    // The XmlRpc server that accepted this connection
    XmlRpcServer* _server;

    //// Possible IO states for the connection
    //enum ServerConnectionState { READ_HEADER, READ_REQUEST, WRITE_RESPONSE };
    //ServerConnectionState _connectionState;

    // Request headers
    std::string _header;

    // Number of bytes expected in the request body (parsed from header)
    int _contentLength;

    // Request body
    std::string _request;

    // Response
    std::string _response;

    // Number of bytes of the response written so far
    int _bytesWritten;

    // Whether to keep the current client connection open for further requests
    bool _keepAlive;

	// 是否通过验证
	bool _bSignIn;

	/*sockaddr*/
	struct sockaddr_in m_stAddr;
  };
} // namespace XmlRpc

#endif // _XMLRPCSERVERCONNECTION_H_
