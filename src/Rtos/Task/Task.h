#pragma once

#include "Rtos/CpuCore/CpuCore.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <functional>
#include <string>
#include <unique_resource.hpp>
#include <tl/optional.hpp>

namespace Rtos
{
    class Task
    {
    public:
        using Code = std::function<void(Task&)>;
        Task(
            const char* name,
            uint8_t priority,
            size_t stackSize_B,
            Code code,
            CpuCore executionCore = CpuCore::Auto
        );

        static Task getCurrent();

        const char* getName() const;
        void cancel();

    private:
        Task(TaskHandle_t handle) noexcept;
        static void taskFunction(Task& task) noexcept;
        static void cancelByHandle(TaskHandle_t handle) noexcept;

        Code m_code;

        TaskHandle_t m_handle;
    };
}