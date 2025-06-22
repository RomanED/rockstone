#include "TopTracker.h"
#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <algorithm>

class TopTrackerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        tracker = std::make_unique<TopTracker>(3, std::chrono::milliseconds(100));
    }

    std::unique_ptr<TopTracker> tracker;
};

TEST_F(TopTrackerTest, InitialStateIsEmpty)
{
    auto actions = tracker->get_actions();
    EXPECT_TRUE(actions.empty());
}

TEST_F(TopTrackerTest, AddSingleAction)
{
    tracker->add("Action1");
    auto actions = tracker->get_actions();
    ASSERT_EQ(actions.size(), 1);
    EXPECT_EQ(actions[0], "Action1");
}

TEST_F(TopTrackerTest, RespectsMaxEntries)
{
    tracker->add("A1");
    tracker->add("A2");
    tracker->add("A3");
    tracker->add("A4");
    
    auto actions = tracker->get_actions();
    ASSERT_EQ(actions.size(), 3);
    EXPECT_EQ(actions[0], "A2");
    EXPECT_EQ(actions[1], "A3");
    EXPECT_EQ(actions[2], "A4");
}

TEST_F(TopTrackerTest, ManualCleanup)
{
    tracker->add("TempAction");
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    tracker->cleanup();
    
    auto actions = tracker->get_actions();
    EXPECT_TRUE(actions.empty());
}

TEST_F(TopTrackerTest, MaintainsOrder)
{
    tracker->add("First");
    tracker->add("Second");
    tracker->add("Third");
    
    auto actions = tracker->get_actions();
    ASSERT_EQ(actions.size(), 3);
    EXPECT_EQ(actions[0], "First");
    EXPECT_EQ(actions[1], "Second");
    EXPECT_EQ(actions[2], "Third");
}

TEST_F(TopTrackerTest, AutoRemovesExpiredEntries)
{
    tracker->add("TempAction");
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    auto actions = tracker->get_actions(); // Теперь правильно!
    EXPECT_TRUE(actions.empty());
}

TEST_F(TopTrackerTest, ThreadSafetyStressTest)
{
    TopTracker stress_tracker(100000, std::chrono::seconds(10));

    constexpr size_t kThreadCount = 8;
    constexpr size_t kIterations = 10000;

    std::vector<std::thread> threads;
    for (size_t i = 0; i < kThreadCount; ++i)
    {
        threads.emplace_back([&]
        {
            for (size_t j = 0; j < kIterations; ++j)
            {
                stress_tracker.add("Action_" + std::to_string(j));
            }
        });
    }

    // Добавляем потоки для чтения
    std::atomic<bool> running(true);
    std::thread reader([&]
    {
        while (running)
        {
            auto actions = stress_tracker.get_actions();
            EXPECT_LE(actions.size(), kThreadCount * kIterations);
        }
    });

    for (auto& thread : threads)
    {
        thread.join();
    }
    running = false;
    reader.join();

    EXPECT_EQ(stress_tracker.get_actions().size(), kThreadCount * kIterations);
}



int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
