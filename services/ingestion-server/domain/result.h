#pragma once

// This header is the ONE exception to "stub everything" in this service's
// skeleton: it is a fully working, header-only, inline-implemented utility.
// Every stub elsewhere returns Result<T>::Err(...) through this type.

#include <optional>
#include <string>
#include <utility>
#include <variant>

enum class ErrorCode {
    Unknown,
    NotImplemented,
    NotFound,
    InvalidArgument,
    AlreadyExists,
    IoError,
    Unavailable,
};

struct Error {
    ErrorCode code;
    std::string message;
};

template <typename T> class Result {
public:
    static Result Ok(T value) {
        return Result(std::variant<T, Error>(std::in_place_index<0>, std::move(value)));
    }

    static Result Err(Error error) {
        return Result(std::variant<T, Error>(std::in_place_index<1>, std::move(error)));
    }

    bool IsOk() const {
        return storage_.index() == 0;
    }
    bool IsErr() const {
        return storage_.index() == 1;
    }

    const T& Value() const {
        return std::get<0>(storage_);
    }
    T& Value() {
        return std::get<0>(storage_);
    }

    const Error& GetError() const {
        return std::get<1>(storage_);
    }

private:
    explicit Result(std::variant<T, Error> storage) : storage_(std::move(storage)) {}

    std::variant<T, Error> storage_;
};

template <> class Result<void> {
public:
    static Result Ok() {
        return Result(std::nullopt);
    }
    static Result Err(Error error) {
        return Result(std::move(error));
    }

    bool IsOk() const {
        return !error_.has_value();
    }
    bool IsErr() const {
        return error_.has_value();
    }

    const Error& GetError() const {
        return *error_;
    }

private:
    explicit Result(std::optional<Error> error) : error_(std::move(error)) {}

    std::optional<Error> error_;
};
