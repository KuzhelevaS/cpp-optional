/* Разместите здесь код класса Optional */
#include <stdexcept>
#include <utility>

class BadOptionalAccess : public std::exception {
public:
    using exception::exception;

    virtual const char* what() const noexcept override {
        return "Bad optional access";
    }
};

template <typename T>
class Optional {
public:
    Optional() = default;
    Optional(const T& value) : is_initialized_(true), value_(new (&data_[0]) T(value)){}
    Optional(T&& value) : is_initialized_(true), value_(new (&data_[0]) T(std::move(value))) {}
    Optional(const Optional& other) {
        if(other.is_initialized_) {
            is_initialized_ = true;
            value_ = new (&data_[0]) T(*other.value_);
        }
    }
    Optional(Optional&& other) {
        if(other.is_initialized_) {
            is_initialized_ = true;
            value_ = new (&data_[0]) T(std::move(*other.value_));
        }
    }

    Optional& operator=(const T& value) {
        if (is_initialized_) {
            *value_ = value;
        } else {
            value_ = new (&data_[0]) T(value);
            is_initialized_ = true;
        }
        return *this;
    }
    Optional& operator=(T&& rhs) {
        if (is_initialized_) {
            *value_ = std::move(rhs);
        } else {
            value_ = new (&data_[0]) T(std::move(rhs));
            is_initialized_ = true;
        }
        return *this;
    }
    Optional& operator=(const Optional& rhs) {
        if (rhs.is_initialized_) {
            if (!is_initialized_) {
                is_initialized_ = true;
                value_ = new (&data_[0]) T(*rhs.value_);
            } else {
                *value_ = *rhs.value_; 
            }
        } else {
            if (is_initialized_) {
                is_initialized_ = false;
                value_->~T();
            } 
        }
        return *this;
    }
    Optional& operator=(Optional&& rhs) {
        if (rhs.is_initialized_) {
            if (!is_initialized_) {
                is_initialized_ = true;
                value_ = new (&data_[0]) T(std::move(*rhs.value_));
            } else {
                *value_ = std::move(*rhs.value_); 
            }
        } else {
            if (is_initialized_) {
                is_initialized_ = false;
                value_->~T();
            } 
        }
        return *this;
    }

    ~Optional() {
        if (is_initialized_) {
            value_->~T();
        }
    }
    
    template <typename... Type>
    void Emplace(Type&&... value) {
        if (is_initialized_) {
            value_->~T();
        } 
        value_ = new (&data_[0]) T(std::forward<Type>(value)...);
        is_initialized_ = true;
    }

    bool HasValue() const {
        return is_initialized_;
    }

    // Операторы * и -> не должны делать никаких проверок на пустоту Optional.
    // Эти проверки остаются на совести программиста
    T& operator*() & {
        return *value_;
    }
    const T& operator*() const & {
        return *value_;
    }
    T&& operator*() && {
        return std::move(*value_);
    }
    T* operator->() {
        return value_;
    }
    const T* operator->() const{
        return value_;
    }

    // Метод Value() генерирует исключение BadOptionalAccess, если Optional пуст
    T& Value() & {
        if (!is_initialized_) {
            throw BadOptionalAccess();
        }
        return *value_;
    }
    const T& Value() const & {
        if (!is_initialized_) {
            throw BadOptionalAccess();
        }
        return *value_;
    }
    T&& Value() && {
        if (!is_initialized_) {
            throw BadOptionalAccess();
        }
        return std::move(*value_);
    }
    
    void Reset() {
        is_initialized_ = false;
        if (value_) {
            value_->~T();
        }
    }

private:
    // alignas нужен для правильного выравнивания блока памяти
    alignas(T) char data_[sizeof(T)];
    bool is_initialized_ = false;
    T* value_ = nullptr;
};