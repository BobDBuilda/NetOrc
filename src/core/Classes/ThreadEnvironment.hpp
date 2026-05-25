#ifndef THREAD_ENVIRONMENT_HPP
#define THREAD_ENVIRONMENT_HPP

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

class ThreadEnvironment{
    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;
        std::mutex mtx;
        std::condition_variable condition_var;
        std::atomic<bool> stop;

    public:
        ThreadEnvironment(size_t num_threads) : stop(false){
            for(size_t i = 0; i < num_threads; i++){
                workers.emplace_back([this]() {
                    while(true){
                        std::function<void()> task;

                        {
                            std::unique_lock<std::mutex> lock(mtx);
                            condition_var.wait(lock, [this]() {
                                return stop || !tasks.empty();
                            });

                            if (stop && tasks.empty()){
                                return;
                            }

                            task = std::move(tasks.front());
                            tasks.pop();
                        }

                        task();
                    }
                });
            }
        }

        ~ThreadEnvironment(){
            {
                std::unique_lock<std::mutex> lock(mtx);
                stop = true;
            }
            condition_var.notify_all();

            for (auto& t: workers){
                t.join();
            }


        }

        template<typename F>
        void enqueue(F&& f){
            {
                std::unique_lock<std::mutex> lock(mtx);
                tasks.emplace(std::forward<F>(f));
            }

            condition_var.notify_one();
        }
};

#endif // THREAD_ENVIRONMENT_HPP
