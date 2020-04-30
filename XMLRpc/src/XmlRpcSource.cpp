/*----------------------------------------------------------------
// Copyright (C) 2014  �������̨ & �������������޹�˾
// ��Ȩ����.
//
// �ļ����ƣ�XmlRpcSource
// �ļ������������ļ�XmlRpcSource
//
// �����ߣ�Founder 
// �������ڣ�2009��6��1��
// �汾�ţ�  1.0
// 
// �޸ģ� ��
//----------------------------------------------------------------*/
#include "stdafx.h"
#include "XmlRpcSource.h"
#include "XmlRpcSocket.h"
#include "XmlRpcUtil.h"

namespace XmlRpc {


  XmlRpcSource::XmlRpcSource(int fd /*= -1*/, bool deleteOnClose /*= false*/) 
    : _fd(fd), _deleteOnClose(deleteOnClose), _keepOpen(false), _lastfd(fd)
  {
  }

  XmlRpcSource::~XmlRpcSource()
  {
  }


  void
  XmlRpcSource::close()
  {
    if (_fd != -1) {
      XmlRpcUtil::log(2,"XmlRpcSource::close: closing socket %d.", _fd);
      XmlRpcSocket::close(_fd);
      XmlRpcUtil::log(2,"XmlRpcSource::close: done closing socket %d.", _fd);
      _fd = -1;
    }
    if (_deleteOnClose) {
      XmlRpcUtil::log(2,"XmlRpcSource::close: deleting this");
      _deleteOnClose = false;
      delete this;
    }
  }

} // namespace XmlRpc
