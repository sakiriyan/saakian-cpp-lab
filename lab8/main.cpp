#include <algorithm>
#include <chrono>
#include <clocale>
#include <climits>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif

template <typename T>
int binarySearch(const std::vector<T>& values, const T& key) {
    int left = 0, right = static_cast<int>(values.size()) - 1;
    while (left <= right) {
        int middle = left + (right - left) / 2;
        if (values[middle] == key) return middle;
        if (values[middle] < key) left = middle + 1;
        else right = middle - 1;
    }
    return -1;
}

int nearestIndex(const std::vector<int>& values, int key) {
    if (values.empty()) throw std::invalid_argument("Массив пуст");
    int left = 0, right = static_cast<int>(values.size());
    while (left < right) {
        int middle = left + (right - left) / 2;
        if (values[middle] < key) left = middle + 1;
        else right = middle;
    }
    if (left == 0) return 0;
    if (left == static_cast<int>(values.size())) return left - 1;
    // Вычисления в long long защищают разность крайних int от переполнения.
    long long before = static_cast<long long>(key) - values[left - 1];
    long long after = static_cast<long long>(values[left]) - key;
    return before <= after ? left - 1 : left;
}

class HashTable {
    enum State { Empty, Occupied, Deleted };
    struct Slot {
        std::string key, value;
        State state = Empty;
    };
    std::vector<Slot> table;
    int count = 0;
    std::size_t hash(const std::string& key) const {
        // FNV-1a: смешиваем каждый байт ключа с накопленным значением.
        std::uint64_t value = 14695981039346656037ULL;
        for (unsigned char ch : key) {
            value ^= ch;
            value *= 1099511628211ULL;
        }
        return value % table.size();
    }
public:
    explicit HashTable(int capacity) {
        if (capacity <= 0) throw std::invalid_argument("Ёмкость должна быть положительной");
        table.resize(capacity);
    }
    int size() const { return count; }
    const std::string* find(const std::string& key) const {
        std::size_t position = hash(key);
        for (std::size_t step = 0; step < table.size(); ++step) {
            const Slot& slot = table[position];
            if (slot.state == Empty) return nullptr;
            if (slot.state == Occupied && slot.key == key) return &slot.value;
            position = (position + 1) % table.size();
        }
        return nullptr;
    }
    void insert(const std::string& key, const std::string& value) {
        std::size_t position = hash(key);
        std::size_t freePosition = table.size();
        for (std::size_t step = 0; step < table.size(); ++step) {
            Slot& slot = table[position];
            if (slot.state == Occupied && slot.key == key) {
                slot.value = value;
                return;
            }
            if (slot.state != Occupied && freePosition == table.size()) freePosition = position;
            if (slot.state == Empty) break;
            position = (position + 1) % table.size();
        }
        if (freePosition == table.size()) throw std::overflow_error("Хеш-таблица заполнена");
        // Запись состояния выполняется после успешного копирования строк.
        table[freePosition].key = key;
        table[freePosition].value = value;
        table[freePosition].state = Occupied;
        ++count;
    }
    bool remove(const std::string& key) {
        std::size_t position = hash(key);
        for (std::size_t step = 0; step < table.size(); ++step) {
            Slot& slot = table[position];
            if (slot.state == Empty) return false;
            if (slot.state == Occupied && slot.key == key) {
                slot.state = Deleted;
                slot.key.clear();
                slot.value.clear();
                --count;
                return true;
            }
            position = (position + 1) % table.size();
        }
        return false;
    }
};

void check(bool condition) {
    if (!condition) throw std::logic_error("Проверка не пройдена");
}

void benchmark(int size) {
    std::vector<std::string> keys;
    HashTable table(size * 2 + 1);
    for (int i = 0; i < size; ++i) {
        std::string key = "фрукт" + std::to_string(1000000 + i);
        keys.push_back(key);
        table.insert(key, "значение");
    }
    std::sort(keys.begin(), keys.end());
    constexpr int queriesCount = 5000;
    std::mt19937 random(42);
    std::uniform_int_distribution<int> index(0, size - 1);
    std::vector<std::string> queries;
    for (int i = 0; i < queriesCount; ++i) {
        queries.push_back(i % 2 == 0 ? keys[index(random)] : "нет" + std::to_string(i));
    }
    double binaryTimes[5], hashTimes[5];
    for (int repeat = 0; repeat < 5; ++repeat) {
        int binaryFound = 0, hashFound = 0;
        auto start = std::chrono::steady_clock::now();
        for (const auto& key : queries) binaryFound += binarySearch(keys, key) >= 0;
        auto finish = std::chrono::steady_clock::now();
        binaryTimes[repeat] = std::chrono::duration<double, std::micro>(finish - start).count();
        start = std::chrono::steady_clock::now();
        for (const auto& key : queries) hashFound += table.find(key) != nullptr;
        finish = std::chrono::steady_clock::now();
        hashTimes[repeat] = std::chrono::duration<double, std::micro>(finish - start).count();
        check(binaryFound == queriesCount / 2 && binaryFound == hashFound);
    }
    std::sort(binaryTimes, binaryTimes + 5);
    std::sort(hashTimes, hashTimes + 5);
    std::cout << size << " | " << binaryTimes[2] << " | " << hashTimes[2] << '\n';
}

int main() {
    std::setlocale(LC_ALL, "Russian");
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    try {
        std::cout << "Лабораторная №8. Вариант 10\n";
        std::vector<int> values = {1, 4, 8, 12};
        check(binarySearch(values, 8) == 2 && binarySearch(values, 7) == -1);
        check(binarySearch(std::vector<int>{}, 5) == -1);
        check(nearestIndex(values, 6) == 1);
        check(nearestIndex(values, -10) == 0 && nearestIndex(values, 99) == 3);
        check(nearestIndex({INT_MIN, INT_MAX}, 0) == 1);
        check(nearestIndex({5}, 0) == 0);
        std::cout << "Ближайшее к 6: " << values[nearestIndex(values, 6)] << '\n';
        HashTable fruits(7);
        fruits.insert("яблоко", "зелёное");
        fruits.insert("груша", "жёлтая");
        fruits.insert("яблоко", "красное");
        check(fruits.size() == 2 && fruits.find("яблоко") && *fruits.find("яблоко") == "красное");
        std::cout << "яблоко -> " << *fruits.find("яблоко") << '\n';
        check(fruits.remove("груша") && !fruits.remove("груша") && !fruits.find("груша"));
        HashTable collisions(3);
        collisions.insert("a", "1");
        collisions.insert("d", "2");
        collisions.insert("h", "3");
        check(collisions.remove("a") && collisions.find("h"));
        collisions.insert("h", "новое");
        check(collisions.size() == 2 && *collisions.find("h") == "новое");
        collisions.insert("o", "4");
        bool full = false, empty = false;
        try { collisions.insert("m", "5"); } catch (const std::overflow_error&) { full = true; }
        try { nearestIndex({}, 5); } catch (const std::invalid_argument&) { empty = true; }
        check(full && empty);
        std::cout << "Коллизии, удалённые ячейки и переполнение проверены.\n";
        std::cout << "Медиана пяти серий по 5000 запросов, мкс\n";
        std::cout << "N | Двоичный поиск | Хеш-таблица\n" << std::fixed << std::setprecision(2);
        for (int size : {1000, 10000, 100000}) benchmark(size);
        std::cout << "Все проверки пройдены.\n";
    } catch (const std::exception& error) {
        std::cerr << "Ошибка: " << error.what() << '\n';
        return 1;
    }
}
