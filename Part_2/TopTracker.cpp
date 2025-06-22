#include "TopTracker.h"
#include <algorithm>

void TopTracker::add(const std::string& action)
{
    const auto now = Clock::now();
    std::lock_guard<std::mutex> lock(mutex);

    // Автоматическая очистка при добавлении
    cleanup_impl();
    entries.push_back({action, now});
    while (entries.size() > max_entries)
    {
        entries.pop_front();
    }
}

std::vector<std::string> TopTracker::get_actions()
{
    std::lock_guard<std::mutex> lock(mutex);
    cleanup_impl();

    std::vector<std::string> result;
    result.reserve(entries.size());

    for (const auto& entry : entries)
    {
        result.push_back(entry.action);
    }

    return result;
}

void TopTracker::cleanup()
{
    std::lock_guard<std::mutex> lock(mutex);
    cleanup_impl();
}

// Внутренняя реализация очистки (const благодаря mutable)
void TopTracker::cleanup_impl() const
{
    const auto now = Clock::now();
    const auto expired_threshold = now - timeout;

    while (!entries.empty() && entries.front().timestamp < expired_threshold)
    {
        entries.pop_front();
    }
}
