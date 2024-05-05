#ifdef ESP32

#include "Task.h"
#include "SourceLocation/SourceLocation.h"
#include "Logger/Logger.h"
#include "ExceptionTrace/ExceptionTrace.h"
#include <Arduino.h>
#include <utility>

using namespace Rtos;
using namespace std_experimental;


Task::Task(
    const char* name,
    uint8_t priority,
    size_t stackSize_B,
    Code code,
    CpuCore executionCore
) : m_code(std::move(code))
{
    TaskHandle_t handle = nullptr;
    BaseType_t status = xTaskCreateUniversal(
        [](void* context){
            taskFunction(*static_cast<Task*>(context));
        },
        name,
        stackSize_B,
        this,
        priority,
        &handle,
        static_cast<BaseType_t>(executionCore)
    );
    if (status != pdPASS)
    {
        throw std::runtime_error(SOURCE_LOCATION + "Failed to create task \"" + name + "\"");
    }
    m_handle = unique_resource<TaskHandle_t, std::function<void(TaskHandle_t)>>(std::move(handle), cancelByHandle);
}

Task::Task(TaskHandle_t handle) noexcept :
    m_handle(unique_resource<TaskHandle_t, std::function<void(TaskHandle_t)>>(std::move(handle), cancelByHandle))
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
    return pcTaskGetName(m_handle.value().get());
}


void Task::cancel()
{
    Task::cancelByHandle(m_handle.value().get());
}


void Task::taskFunction(Task& task) noexcept
{
    try
    {
        task.m_code(task);
        throw std::runtime_error(SOURCE_LOCATION + "Task code returned. Task must delete itself or run indefinitely.");
    }
    catch (...)
    {
        Logger[LogLevel::Error]
            << "Exception occurred at "
            << SOURCE_LOCATION
            << "in task \""
            << task.getName()
            << "\"\r\n"
            << ExceptionTrace::what() << std::endl;
        task.cancel();
    }
}


void Task::cancelByHandle(TaskHandle_t handle) noexcept
{
    delay(0);
    // It seems like a bug in FreeRTOS that we have to do "&handle" instead of "handle"
    if (eTaskGetState(&handle) != eTaskState::eDeleted)
        vTaskDelete(handle);
}

#endif