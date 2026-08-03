#pragma once
#include <memory>

template <typename T, typename... Args>
concept Bindable = requires(T const& instance, Args&&... args) {
    { instance.bind(args...) } -> std::same_as<std::unique_ptr<typename T::memo_type>>;
};

template <typename T>
// requires Bindable<T, BindArgs...>
class BindingHandle {
    public:
        template<typename... Args>
        requires Bindable<T, Args...>
        explicit BindingHandle(std::shared_ptr<T> handle, Args&&... args) : m_instance(std::move(handle)) {
            m_memo = m_instance->bind(args...);
        }

        ~BindingHandle() =default;

    protected:


        std::shared_ptr<T> m_instance;
        std::unique_ptr<typename T::memo_type> m_memo;
};
//
// template<typename T, typename... Args>
// requires Bindable<T, Args...>
// BindingHandle(std::shared_ptr<T>, Args&&...)
//     -> BindingHandle<T, Args...>;