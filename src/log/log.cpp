//
// Created on 2022/12/12.
//
#include <mutex>
#include "log.h"

namespace smallkv::log
{
    std::shared_ptr<spdlog::logger> logger = nullptr;
    std::mutex _mutex;
    // 单例模式
    std::shared_ptr<spdlog::logger> get_logger()
    {
        std::lock_guard<std::mutex> lockGuard(_mutex);
        if (logger == nullptr)
        {
            logger = spdlog::stdout_color_mt("console");
            logger->set_level(spdlog::level::debug);
        }
        return logger;
    }
}