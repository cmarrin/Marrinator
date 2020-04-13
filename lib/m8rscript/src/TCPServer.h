/*-------------------------------------------------------------------------
    This source file is a part of m8rscript
    For the latest info, see http:www.marrin.org/
    Copyright (c) 2018-2019, Chris Marrin
    All rights reserved.
    Use of this source code is governed by the MIT license that can be
    found in the LICENSE file.
-------------------------------------------------------------------------*/

#pragma once

#include "Task.h"

namespace m8r {

class TCPServer {
public:
    using CreateTaskFunction = std::function<Mad<TaskBase>()>;
    
    TCPServer(uint16_t port, CreateTaskFunction, TCP::EventFunction);
    ~TCPServer();

protected:
    Mad<TCP> _socket;

    struct Connection
    {
        Connection() { }
        Mad<TaskBase> task;
    };
    
    Connection _connections[TCP::MaxConnections];
    
    CreateTaskFunction _createTaskFunction;
    TCP::EventFunction _eventFunction;
};
    
}
