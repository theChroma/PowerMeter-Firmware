#ifdef ESP32

#include "Task.h"
#include "SourceLocation/SourceLocation.h"
#include "Logger/Logger.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include <Arduino.h>
#include <utility>

using namespace Rtos;


Task::Task(
    const char* name,
    uint8_t priority,
    size_t stackSize_B,
    Code code,
    CpuCore executionCore
) : m_code(std::move(code))
{
    BaseType_t status = xTaskCreateUniversal(
        [](void* context){
            taskFunction(static_cast<Task*>(context));
        },
        name,
        stackSize_B,
        this,
        priority,
        &m_handle,
        static_cast<BaseType_t>(executionCore)
    );
    if (status != pdPASS)
    {
        throw std::runtime_error(SOURCE_LOCATION + "Failed to create task \"" + name + "\"");
    }
}


Task::Task(TaskHandle_t handle) noexcept :
    m_handle(handle)
{}


Task Task::getCurrent()
{
    TaskHandle_t handle = xTaskGetCurrentTaskHandle();
    if (!handle)
        throw std::runtime_error(SOURCE_LOCATION + "Couldn't get current task");
    return Task(handle);
}


const char* Task::getName() const
{
    return pcTaskGetName(m_handle);
}


void Task::cancel()
{
    Task::cancelByHandle(m_handle);
}


void Task::taskFunction(Task* task) noexcept
{
    try
    {
        task->m_code(task);
    }
    catch (...)
    {
        Logger[LogLevel::Error]
            << "Exception occurred at "
            << SOURCE_LOCATION
            << "in task \""
            << task->getName()
            << "\"\r\n"
            << ExceptionTrace::what() << std::endl;
    }
    task->cancel();
}


void Task::cancelByHandle(TaskHandle_t handle) noexcept
{
    delay(0);
    // It seems like a bug in FreeRTOS that we have to do "&handle" instead of "handle"
    if (eTaskGetState(&handle) != eTaskState::eDeleted)
        vTaskDelete(handle);
}

#endif