/*----------------------------------------------------------------
// Copyright (C) 2014  中央电视台 & 北大方正电子有限公司
// 版权所有.
//
// 文件名称：XmlRpcServerMethod
// 文件描述：代码文件XmlRpcServerMethod
//
// 创建者：Founder 
// 创建日期：2009年6月1日
// 版本号：  1.0
// 
// 修改： 无
//----------------------------------------------------------------*/
#include "stdafx.h"
#include "XmlRpcServerMethod.h"
#include "XmlRpcServer.h"

namespace XmlRpc {


  XmlRpcServerMethod::XmlRpcServerMethod(std::string const& name, XmlRpcServer* server)
  {
    _name = name;
    _server = server;
    if (_server) _server->addMethod(this);
  }

  XmlRpcServerMethod::~XmlRpcServerMethod()
  {
    if (_server) _server->removeMethod(this);
  }


} // namespace XmlRpc
