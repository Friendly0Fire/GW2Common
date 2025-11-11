#pragma once
#include <concepts>
#include <functional>
#include <memory>
#include <stack>
#include <stdexcept>

class BaseSingleton
{
public:
    virtual ~BaseSingleton() = default;

protected:
    static BaseSingleton* Store(std::unique_ptr<BaseSingleton>&& ptr);
    static void Clear(BaseSingleton* ptr);
};

class SingletonManager
{
public:
    SingletonManager() = default;
    void Shutdown() {
        while(!singletons_.empty())
            singletons_.pop();
    }

private:
    std::stack<std::unique_ptr<BaseSingleton>> singletons_;

    friend class BaseSingleton;
};
extern SingletonManager g_singletonManagerInstance;

template<typename T>
class Singleton : public BaseSingleton
{
public:
    template<typename T2 = T>
        requires std::derived_from<T2, T>
    static T2& i() {
        if(!init_) {
            if constexpr(std::is_default_constructible_v<T2>) {
                init_ = true;
                i_ = (T*)Store(std::make_unique<T2>());
            }
            else
                throw std::logic_error("Singleton is not default-constructible but was not explicitly initialized before access.");
        }

        return *(T2*)i_;
    }

    template<typename T2 = T, typename... Args>
        requires std::derived_from<T2, T>
    static T2& init(Args&&... args) {
        init_ = true;
        i_ = (T*)Store(std::make_unique<T2>(std::forward<Args>(args)...));
        return *(T2*)i_;
    }

    template<typename T2>
        requires std::derived_from<T2, T>
    static void f(std::function<void(T2&)> action) {
        if(i_)
            action(static_cast<T2&>(i_));
    }

    static void f(std::function<void(T&)> action) {
        if(i_)
            action(*i_);
    }

    static void reset() {
        if(init_)
            Clear(i_);
    }

    ~Singleton() override {
        i_ = nullptr;
        init_ = false;
    }

private:
    inline static bool init_ = false;
    inline static T* i_ = nullptr;
};
