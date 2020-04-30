/*----------------------------------------------------------------
// Copyright (C) 2014  �������̨ & �������������޹�˾
// ��Ȩ����.
//
// �ļ����ƣ�XmlRpcServerMethod
// �ļ������������ļ�XmlRpcServerMethod
//
// �����ߣ�Founder 
// �������ڣ�2009��6��1��
// �汾�ţ�  1.0
// 
// �޸ģ� ��
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
