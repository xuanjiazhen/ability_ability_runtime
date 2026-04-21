#ifndef MOCK_SINGLETON_H
#define MOCK_SINGLETON_H

#include <memory>
#include <mutex>

template<typename T>
class DelayedSingleton {
public:
    static std::shared_ptr<T> GetInstance()
    {
        static std::once_flag onceFlag;
        static std::shared_ptr<T> instance;
        std::call_once(onceFlag, []() { instance = std::make_shared<T>(); });
        return instance;
    }
};

#define DECLARE_DELAYED_SINGLETON(cls) \
    friend class DelayedSingleton<cls>

#endif // MOCK_SINGLETON_H
