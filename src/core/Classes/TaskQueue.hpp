#ifndef TASK_QUEUE_HPP
#define TASK_QUEUE_HPP

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

/**
 * TaskQueue: Acts as the "Task Queue" in your Event Loop model.
 * Multiple worker threads (from ThreadEnvironment) can push data here,
 * while the Main Thread (the Event Loop) pops data to process it.
 */
template <typename T>
class TaskQueue {
private:
    std::queue<T> queue;
    mutable std::mutex mtx;
    std::condition_variable condition_var;

public:
    TaskQueue() = default;

    // Push an item into the queue (Worker threads use this)
    void push(T value) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            queue.push(std::move(value));
        }
        condition_var.notify_one(); // Notify the Event Loop that a task is ready
    }

    // Blocking Pop: Stops the thread until an item is available
    void wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lock(mtx);
        condition_var.wait(lock, [this] { return !queue.empty(); });
        value = std::move(queue.front());
        queue.pop();
    }

    // Non-Blocking Pop: Returns immediately with the data or an empty optional
    // (The Event Loop typically uses this if it has other work to do)
    std::optional<T> try_pop() {
        std::lock_guard<std::mutex> lock(mtx);
        if (queue.empty()) {
            return std::nullopt;
        }
        T value = std::move(queue.front());
        queue.pop();
        return value;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.size();
    }
};

#endif // TASK_QUEUE_HPP
