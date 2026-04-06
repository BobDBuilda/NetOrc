#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

class ThreadPool{
    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;
        std::mutex mtx;
        std::condition_variable cv;
        std::atomic<bool> stop;

    public:
        ThreadPool(size_t num_threads) : stop(false){
            for(size_t i = 0; i < num_threads; i++){
                workers.emplace_back([this]() {
                    while(true){
                        std::function<void()> task;

                        {
                            std::unique_lock<std::mutex> lock(mtx);
                            cv.wait(lock, [this]() {
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

        ~ThreadPool(){
            {
                std::unique_lock<std::mutex> lock(mtx);
                stop = true;
            }
            cv.notify_all();

            for (auto& t: workers){
                t.join();
            }


        }

        template<typename F>
        void enqueue(F&& f){
            {
                std::unique_lock<Std::mutex> lock(mtx);
                tasks.emplace(std::forward<F>(f));
            }

            cv.notify_one();
        }
};