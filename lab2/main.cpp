#include <algorithm>
#include <chrono>
#include <clocale>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif

template <typename T>
class SortedList {
    std::vector<T> data;
    int lowerBound(const T& value) const {
        int left = 0, right = static_cast<int>(data.size());
        while (left < right) {
            int middle = left + (right - left) / 2;
            if (data[middle] < value) left = middle + 1;
            else right = middle;
        }
        return left;
    }
public:
    void insert(const T& value) {
        int position = lowerBound(value);
        data.insert(data.begin() + position, value);
    }
    bool contains(const T& value) const {
        int position = lowerBound(value);
        return position < static_cast<int>(data.size()) &&
               !(value < data[position]);
    }
    bool remove(const T& value) {
        int position = lowerBound(value);
        if (position == static_cast<int>(data.size()) || value < data[position]) return false;
        data.erase(data.begin() + position);
        return true;
    }
    const T& getByIndex(int index) const {
        if (index < 0 || index >= static_cast<int>(data.size()))
            throw std::out_of_range("Индекс за границами списка");
        return data[index];
    }
    int size() const { return static_cast<int>(data.size()); }
};

template <typename T>
bool isEqual(T first, T second) {
    return first == second;
}

// Таблица соответствий. Повторный ключ обновляет значение.
template <typename Tin, typename Tout>
class Mapper {
    std::map<Tin, Tout> data;
public:
    Mapper() = default;
    void addMapping(const Tin& key, const Tout& value) { data.insert_or_assign(key, value); }
    const Tout& apply(const Tin& key) const { return data.at(key); }
};

void check(bool condition) {
    if (!condition) throw std::logic_error("Проверка не пройдена");
}

using Clock = std::chrono::steady_clock;

double elapsed(Clock::time_point start) {
    return std::chrono::duration<double, std::micro>(Clock::now() - start).count();
}

void benchmark(int size) {
    constexpr int batch = 256;
    double own[4] = {}, standard[4] = {};
    std::mt19937 random(42);
    std::uniform_int_distribution<int> position(0, size - 1);
    std::vector<int> keys(batch), indices(batch);
    for (int i = 0; i < batch; ++i) {
        indices[i] = position(random);
        keys[i] = indices[i] * 2 + 1;
    }
    for (int repeat = 0; repeat < 5; ++repeat) {
        SortedList<int> list;
        std::multiset<int> tree;
        for (int i = 0; i < size; ++i) {
            list.insert(2 * i);
            tree.insert(2 * i);
        }
        auto start = Clock::now();
        for (int key : keys) list.insert(key);
        own[0] += elapsed(start);
        start = Clock::now();
        for (int key : keys) tree.insert(key);
        standard[0] += elapsed(start);
        check(list.size() == static_cast<int>(tree.size()));
        int index = 0;
        for (int value : tree) check(list.getByIndex(index++) == value);

        int foundOwn = 0, foundStandard = 0;
        start = Clock::now();
        for (int i = 0; i < batch; ++i) foundOwn += list.contains(i % 2 == 0 ? keys[i] - 1 : -1);
        own[1] += elapsed(start);
        start = Clock::now();
        for (int i = 0; i < batch; ++i)
            foundStandard += tree.find(i % 2 == 0 ? keys[i] - 1 : -1) != tree.end();
        standard[1] += elapsed(start);
        check(foundOwn == batch / 2 && foundOwn == foundStandard);

        long long sumOwn = 0, sumStandard = 0;
        start = Clock::now();
        for (int i : indices) sumOwn += list.getByIndex(i);
        own[3] += elapsed(start);
        start = Clock::now();
        for (int i : indices) sumStandard += *std::next(tree.begin(), i);
        standard[3] += elapsed(start);
        check(sumOwn == sumStandard);

        int removedOwn = 0, removedStandard = 0;
        start = Clock::now();
        for (int key : keys) removedOwn += list.remove(key);
        own[2] += elapsed(start);
        start = Clock::now();
        for (int key : keys) {
            auto found = tree.find(key);
            if (found != tree.end()) {
                tree.erase(found);
                ++removedStandard;
            }
        }
        standard[2] += elapsed(start);
        check(removedOwn == batch && removedOwn == removedStandard);
        check(list.size() == size && tree.size() == static_cast<std::size_t>(size));
        index = 0;
        for (int value : tree) check(list.getByIndex(index++) == value);
    }
    const char* names[] = {"insert", "contains", "remove", "getByIndex"};
    for (int i = 0; i < 4; ++i) {
        double first = own[i] / 5, second = standard[i] / 5;
        std::cout << size << " | " << names[i] << " | " << first << " | " << second << '\n';
    }
}

void compareMappers() {
    constexpr int size = 2000, queries = 1000;
    Mapper<int, int> mapper;
    std::vector<std::pair<int, int>> linear;
    for (int i = 0; i < size; ++i) {
        mapper.addMapping(i, i * 2);
        linear.push_back({i, i * 2});
    }
    long long treeSum = 0, linearSum = 0;
    auto start = Clock::now();
    for (int i = 0; i < queries; ++i) treeSum += mapper.apply((i * 17) % size);
    double treeTime = elapsed(start);
    start = Clock::now();
    for (int i = 0; i < queries; ++i) {
        int key = (i * 17) % size;
        for (const auto& item : linear) {
            if (item.first == key) {
                linearSum += item.second;
                break;
            }
        }
    }
    double linearTime = elapsed(start);
    check(treeSum == linearSum && treeSum > 0);
    std::cout << "1000 запросов соответствий, мкс: дерево = " << treeTime
              << ", линейный просмотр = " << linearTime << '\n';
}

int main() {
    std::setlocale(LC_ALL, "Russian");
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    try {
        std::cout << "Лабораторная №2. Вариант 10\n";
        SortedList<int> list;
        check(!list.contains(5) && !list.remove(5));
        for (int value : {7, -2, 7, 3}) list.insert(value);
        std::cout << "Числа по порядку: ";
        for (int i = 0; i < list.size(); ++i) std::cout << list.getByIndex(i) << ' ';
        check(list.getByIndex(0) == -2 && list.getByIndex(1) == 3);
        check(list.remove(7) && list.contains(7) && list.remove(7) && !list.contains(7));
        check(!list.remove(99));
        SortedList<std::string> fruits;
        for (const std::string word : {"яблоко", "абрикос", "груша"}) fruits.insert(word);
        std::cout << "\nФрукты по порядку: ";
        for (int i = 0; i < fruits.size(); ++i) std::cout << fruits.getByIndex(i) << ' ';
        check(fruits.getByIndex(0) == "абрикос" && fruits.getByIndex(2) == "яблоко");
        int errors = 0;
        try { list.getByIndex(-1); } catch (const std::out_of_range&) { ++errors; }
        try { list.getByIndex(list.size()); } catch (const std::out_of_range&) { ++errors; }
        check(errors == 2);
        check(isEqual(5, 5) && !isEqual(2.0, 3.0));
        check(isEqual(std::string("яблоко"), std::string("яблоко")));
        Mapper<std::string, std::string> mapper;
        mapper.addMapping("яблоко", "красное");
        mapper.addMapping("яблоко", "зелёное");
        check(mapper.apply("яблоко") == "зелёное");
        bool missing = false;
        try { mapper.apply("слива"); } catch (const std::out_of_range&) { missing = true; }
        check(missing);
        std::cout << "\nТаблица соответствий: яблоко -> " << mapper.apply("яблоко") << '\n';
        compareMappers();
        std::cout << "Среднее время серии из 256 операций, мкс\nN | Операция | SortedList | multiset\n";
        std::cout << std::fixed << std::setprecision(3);
        for (int size : {1000, 5000, 20000}) benchmark(size);
        std::cout << "Все проверки пройдены.\n";
    } catch (const std::exception& error) {
        std::cerr << "Ошибка: " << error.what() << '\n';
        return 1;
    }
}
