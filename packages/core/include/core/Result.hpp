#pragma once
#include <optional>
#include <string>
#include <variant>

namespace homecctv {

template <typename T, typename E = std::string>
class Result {
public:
    static Result Ok(T value) {
        Result r;
        r.data_ = std::move(value);
        return r;
    }

    static Result Err(E error) {
        Result r;
        r.data_ = std::move(error);
        return r;
    }

    bool is_ok() const noexcept { return std::holds_alternative<T>(data_); }
    bool is_err() const noexcept { return !is_ok(); }

    T& value() { return std::get<T>(data_); }
    const T& value() const { return std::get<T>(data_); }

    E& error() { return std::get<E>(data_); }
    const E& error() const { return std::get<E>(data_); }

    template <typename F>
    auto map(F&& f) {
        using U = std::invoke_result_t<F, T>;
        if (is_ok()) return Result<U, E>::Ok(f(value()));
        return Result<U, E>::Err(error());
    }

private:
    std::variant<T, E> data_;
};

template <typename E>
class Result<void, E> {
public:
    static Result Ok() {
        Result r;
        r.ok_ = true;
        return r;
    }

    static Result Err(E error) {
        Result r;
        r.ok_ = false;
        r.error_ = std::move(error);
        return r;
    }

    bool is_ok() const noexcept { return ok_; }
    bool is_err() const noexcept { return !ok_; }

    E& error() { return error_.value(); }
    const E& error() const { return error_.value(); }

private:
    bool ok_ = false;
    std::optional<E> error_;
};

}  // namespace homecctv
