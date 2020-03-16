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
#include "SystemInterface.h"
#include "TaskManager.h"
#include <cstdint>
#include <functional>

namespace m8r {

class ExecutionUnit;
class String;

class TaskBase {
    friend class TaskManager;
    
public:
    using FinishCallback = std::function<void(TaskBase*)>;
    
    enum class State { Ready, WaitingForEvent, Delaying, Terminated };
    
    virtual ~TaskBase()
    {
        system()->taskManager()->terminate(this);
    }
    
    void run(const FinishCallback& cb = nullptr)
    {
        _finishCB = cb;
        system()->taskManager()->run(this);
    }

    void terminate() { system()->taskManager()->terminate(this); }

    Error error() const { return _error; }
    
    virtual bool readyToRun() const { return state() == State::Ready; }
    
#ifndef NDEBUG
    const String& name() const { return _name; }
#endif

protected:
    TaskBase() { }
    
    State state() const { return _state; }
    void setState(State state) { _state = state; }

    Error _error = Error::Code::OK;

#ifndef NDEBUG
    String _name;
#endif

private:
    void finish() { if (_finishCB) _finishCB(this); }
    
    virtual CallReturnValue execute() = 0;    

    FinishCallback _finishCB;
    
    State _state = State::Ready;
};

class Task : public NativeObject, public TaskBase {
public:
    Task();
    
    virtual ~Task();
    
    bool init(const Stream&);
    bool init(const char* filename);
    
    void receivedData(const String& data, KeyAction action);

    void setConsolePrintFunction(std::function<void(const String&)> f);
    void setConsoleListener(Value func);

    const ExecutionUnit* eu() const { return _eu.get(); }
    
    virtual bool readyToRun() const override;

private:
    virtual CallReturnValue execute() override;

    Mad<ExecutionUnit> _eu;    
};

class TaskProto : public StaticObject {
public:
    TaskProto();

    static CallReturnValue constructor(ExecutionUnit*, Value thisValue, uint32_t nparams);
    static CallReturnValue run(ExecutionUnit*, Value thisValue, uint32_t nparams);
};

}
