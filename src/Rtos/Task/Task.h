#pragma once

#include "Rtos/CpuCore/CpuCore.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <functional>
#include <string>
#include <memory>

namespace PM
{
    namespace Rtos
    {


        class Task
        {
        public:
            using Code = std::function<void(Task&)>;
            Task(
                const std::string& name,
                uint8_t priority,
                size_t stackSize_B,
                const Code& code,
                CpuCore executionCore = CpuCore::Auto
            );

            static Task getCurrent();

            std::string getName() const;
            void cancel();

        private:

            Task(TaskHandle_t handle);
            static void taskFunction(Task& task) noexcept;
            static void cancelByHandle(TaskHandle_t handle) noexcept;

            Code m_code;
            std::shared_ptr<void> m_handle;
        };
    }
}