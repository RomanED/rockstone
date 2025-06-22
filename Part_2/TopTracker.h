#pragma once

#include <deque>
#include <chrono>
#include <string>
#include <mutex>
#include <vector>
#include <type_traits>

class TopTracker final
{
public:
    using Clock = std::chrono::system_clock;
    using TimePoint = Clock::time_point;

    template <typename Rep, typename Period>
    explicit TopTracker(
        size_t max_entries = 1000,
        std::chrono::duration<Rep, Period> timeout = std::chrono::seconds(300)
        ) : max_entries(max_entries),
        timeout(std::chrono::duration_cast<std::chrono::milliseconds>(timeout))
    {}

    void add(const std::string& action);
    std::vector<std::string> get_actions(); // Убрали const!
    void cleanup();

private:
    struct Entry
    {
        std::string action;
        TimePoint timestamp;
    };

    void cleanup_impl() const; // Внутренняя реализация очистки

    mutable std::mutex mutex;
    mutable std::deque<Entry> entries; // mutable для очистки
    size_t max_entries;
    std::chrono::milliseconds timeout;
};
