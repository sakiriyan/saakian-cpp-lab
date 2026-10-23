#include <algorithm>
#include <clocale>
#include <cstddef>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

#ifdef _WIN32
#include <windows.h>
#endif

template <typename T>
class Vector {
    std::unique_ptr<T[]> data;
    std::size_t count = 0;
    std::size_t allocated = 0;

    void swapStorage(Vector& other) noexcept {
        data.swap(other.data);
        std::swap(count, other.count);
        std::swap(allocated, other.allocated);
    }

public:
    Vector() = default;

    Vector(std::size_t size, const T& value) {
        reserve(size);
        if (size != 0) {
            std::fill_n(data.get(), size, value);
        }
        count = size;
    }

    ~Vector() = default;

    Vector(const Vector& other) {
        reserve(other.count);
        if (!other.empty()) {
            std::copy_n(other.data.get(), other.count, data.get());
        }
        count = other.count;
    }

    Vector& operator=(const Vector& other) {
        if (this != &other) {
            Vector copy(other);
            swapStorage(copy);
        }
        return *this;
    }

    Vector(Vector&& other) noexcept {
        swapStorage(other);
    }

    Vector& operator=(Vector&& other) noexcept {
        if (this != &other) {
            Vector moved(std::move(other));
            swapStorage(moved);
        }
        return *this;
    }

    std::size_t size() const noexcept { return count; }
    std::size_t capacity() const noexcept { return allocated; }
    bool empty() const noexcept { return count == 0; }

    void reserve(std::size_t newCapacity) {
        if (newCapacity <= allocated) {
            return;
        }
        auto newData = std::make_unique<T[]>(newCapacity);
        if (!empty()) {
            std::copy_n(data.get(), count, newData.get());
        }
        // старый буфер освобождается после успешного копирования.
        data = std::move(newData);
        allocated = newCapacity;
    }

    void push_back(const T& value) {
        if (count == allocated) {
            // value может ссылаться на элемент старого буфера.
            T saved = value;
            const std::size_t maximum = static_cast<std::size_t>(-1) / sizeof(T);
            if (allocated >= maximum) {
                throw std::length_error("Массив достиг предельного размера");
            }
            reserve(allocated == 0 ? 1 : (allocated > maximum / 2 ? maximum : allocated * 2));
            data[count] = std::move(saved);
        } else {
            data[count] = value;
        }
        ++count;
    }

    const T& operator[](std::size_t index) const {
        if (index >= count) {
            throw std::out_of_range("Индекс за границами массива");
        }
        return data[index];
    }

    T& operator[](std::size_t index) {
        if (index >= count) {
            throw std::out_of_range("Индекс за границами массива");
        }
        return data[index];
    }

    T* begin() noexcept { return data.get(); }
    const T* begin() const noexcept { return data.get(); }
    T* end() noexcept { return count == 0 ? begin() : begin() + count; }
    const T* end() const noexcept { return count == 0 ? begin() : begin() + count; }

    void unique() {
        std::size_t kept = 0;
        for (std::size_t i = 0; i < count; ++i) {
            std::size_t j = 0;
            while (j < kept && !(data[j] == data[i])) {
                ++j;
            }
            if (j == kept) {
                if (kept != i) {
                    data[kept] = data[i];
                }
                ++kept;
            }
        }
        count = kept;
    }

    std::size_t partition(T pivot) {
        std::size_t boundary = 0;
        for (std::size_t i = 0; i < count; ++i) {
            if (data[i] < pivot) {
                if (i != boundary) {
                    std::swap(data[i], data[boundary]);
                }
                ++boundary;
            }
        }
        return boundary;
    }
};

template <typename T>
void print(const Vector<T>& values) {
    for (const T& value : values) {
        std::cout << value << ' ';
    }
    std::cout << '\n';
}

void check(bool condition) {
    if (!condition) {
        throw std::logic_error("Проверка массива не прошла");
    }
}

int main() {
#ifdef _WIN32
    std::setlocale(LC_ALL, "Russian");
    SetConsoleOutputCP(CP_UTF8);
#else
    std::setlocale(LC_ALL, "");
#endif
    try {
        std::cout << "Лабораторная №3. Продвинутый уровень. Вариант 10\n";
        Vector<int> numbers;
        check(numbers.empty() && numbers.partition(5) == 0);
        for (int value : {7, 2, 5, 1, 5, 3, 8}) {
            numbers.push_back(value);
        }
        std::cout << "Исходный массив: ";
        print(numbers);
        std::cout << "Размер: " << numbers.size() << ", ёмкость: " << numbers.capacity() << '\n';
        const auto boundary = numbers.partition(5);
        check(boundary == 3);
        for (std::size_t i = 0; i < numbers.size(); ++i) {
            check((numbers[i] < 5) == (i < boundary));
        }
        check(numbers.partition(0) == 0);
        check(numbers.partition(10) == numbers.size());
        std::cout << "После разделения по числу 5: ";
        print(numbers);
        std::cout << "Граница частей: " << boundary << '\n';

        Vector<std::string> fruits(2, "яблоко");
        fruits.push_back(fruits[0]);
        fruits.push_back("груша");
        fruits.reserve(12);
        check(fruits.capacity() == 12 && fruits.size() == 4);
        fruits.unique();
        check(fruits.size() == 2 && fruits[0] == "яблоко" && fruits[1] == "груша");
        std::cout << "Удаление дубликатов: ";
        print(fruits);

        Vector<std::string> copy(fruits);
        Vector<std::string> assigned;
        assigned = copy;
        copy[0] = "слива";
        check(fruits[0] == "яблоко" && assigned[0] == "яблоко");
        const auto& same = assigned;
        assigned = same;
        check(assigned.size() == 2);
        Vector<std::string> moved(std::move(copy));
        assigned = std::move(moved);
        check(copy.empty() && moved.empty() && assigned[0] == "слива");
        moved.push_back("абрикос");
        check(moved[0] == "абрикос");

        bool caught = false;
        try {
            numbers[numbers.size()];
        } catch (const std::out_of_range&) {
            caught = true;
            std::cout << "Выход за границы обнаружен.\n";
        }
        check(caught);
        std::cout << "Все проверки пройдены.\n";
    } catch (const std::exception& error) {
        std::cerr << "Ошибка: " << error.what() << '\n';
        return 1;
    }
}
