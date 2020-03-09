/*-------------------------------------------------------------------------
    This source file is a part of m8rscript
    For the latest info, see http:www.marrin.org/
    Copyright (c) 2018-2019, Chris Marrin
    All rights reserved.
    Use of this source code is governed by the MIT license that can be
    found in the LICENSE file.
-------------------------------------------------------------------------*/

#pragma once

#include "Containers.h"
#include "SystemTime.h"
#include "Thread.h"
#include <cstdint>

namespace m8r {

class TaskBase;

class TaskManager {
    friend class SystemInterface;
    friend class TaskBase;
    friend class ExecutionUnit;

public:

protected:
    static constexpr uint8_t MaxTasks = 8;

    TaskManager();
    ~TaskManager();
    
    void run(TaskBase*);
    
    void terminate(TaskBase*);
    
    bool executeNextTask();

private:
    void runLoop();

    // Post an event now. When event occurs, call fireEvent
    void readyToExecuteNextTask();
    
    Mutex _mutex;
    Vector<TaskBase*> _list;

    Thread _eventThread;
    Condition _eventCondition;
    bool _terminating = false;
};

}
