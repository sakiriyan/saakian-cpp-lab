#include <algorithm>
#include <chrono>
#include <cstddef>
#include <exception>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <iterator>
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
    std::vector<T> values;

public:
    void insert(const T& value) {
        // lower_bound указывает на первый элемент, который не меньше value.
        const auto position = std::lower_bound(values.begin(), values.end(), value);
        values.insert(position, value);
    }

    bool contains(const T& value) const {
        return std::binary_search(values.begin(), values.end(), value);
    }

    bool remove(const T& value) {
        if (values.empty()) {
            return false;
        }
        const auto position = std::lower_bound(values.begin(), values.end(), value);
        if (position == values.end() || value < *position) {
            return false;
        }
        values.erase(position);
        return true;
    }

    T getByIndex(std::size_t index) const {
        return values.at(index);
    }
};

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::logic_error(message);
    }
}

template <typename T>
void checkOrder(const SortedList<T>& list, const std::multiset<T>& expected) {
    std::size_t index = 0;
    for (const T& value : expected) {
        require(list.getByIndex(index) == value, "Неверный элемент по индексу");
        ++index;
    }
    try {
        list.getByIndex(index);
    } catch (const std::out_of_range&) {
        return;
    }
    throw std::logic_error("Ожидалась ошибка выхода за границы списка");
}

void runChecks() {
    SortedList<int> numbers;
    checkOrder(numbers, {});
    require(!numbers.contains(3) && !numbers.remove(3), "Список должен быть пустым");
    std::cout << "[ОК] Пустой список и выход за границы\n";

    for (int value : {7, -2, 7, 3}) {
        numbers.insert(value);
    }
    checkOrder(numbers, {-2, 3, 7, 7});
    require(numbers.contains(-2) && numbers.contains(7) && !numbers.contains(4),
            "Неверный результат поиска");
    std::cout << "[ОК] Числа по порядку: -2 3 7 7\n";

    require(numbers.remove(7) && numbers.contains(7), "Должна остаться одна семёрка");
    checkOrder(numbers, {-2, 3, 7});
    require(numbers.remove(7) && !numbers.remove(7) && !numbers.remove(4),
            "Неверный результат удаления");
    checkOrder(numbers, {-2, 3});
    std::cout << "[ОК] Удаление дубликата и поиск отсутствующего числа\n";

    SortedList<std::string> words;
    const std::string fruits[] = {"яблоко", "абрикос", "груша"};
    for (const std::string& word : fruits) {
        words.insert(word);
    }
    checkOrder(words, {"абрикос", "груша", "яблоко"});
    require(words.contains("груша") && words.remove("груша") && !words.contains("груша"),
            "Неверный результат проверки строк");
    checkOrder(words, {"абрикос", "яблоко"});
    std::cout << "[ОК] Строки по порядку: абрикос, груша, яблоко\n";
}

template <typename Operation>
double measure(Operation operation, std::vector<int>& results) {
    const auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < results.size(); ++i) {
        results[i] = operation(i);
    }
    const auto finish = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::nano>(finish - start).count()
           / results.size();
}

double median(std::vector<double>& samples) {
    std::sort(samples.begin(), samples.end());
    return samples[samples.size() / 2];
}

void compareContainers(int size) {
    constexpr int batchSize = 512;
    constexpr int repeats = 9;
    SortedList<int> list;
    std::multiset<int> tree;
    for (int i = 0; i < size; ++i) {
        list.insert(2 * i);
        tree.insert(2 * i);
    }

    std::mt19937 random(42);
    std::uniform_int_distribution<int> position(0, size - 1);
    const char* operations[] = {"insert", "contains", "remove", "getByIndex"};
    std::vector<double> listTimes[4];
    std::vector<double> treeTimes[4];
    std::vector<int> values(batchSize), indices(batchSize);
    std::vector<int> listResults(batchSize), treeResults(batchSize);

    for (int repeat = 0; repeat < repeats; ++repeat) {
        // Нечётные значения отсутствуют в исходных контейнерах.
        for (int i = 0; i < batchSize; ++i) {
            indices[i] = position(random);
            values[i] = 2 * indices[i] + 1;
        }
        auto compare = [&](int operation, auto listOperation, auto treeOperation) {
            double listTime;
            double treeTime;
            // Чередуем очередность контейнеров между повторами.
            if (repeat % 2 == 0) {
                listTime = measure(listOperation, listResults);
                treeTime = measure(treeOperation, treeResults);
            } else {
                treeTime = measure(treeOperation, treeResults);
                listTime = measure(listOperation, listResults);
            }
            require(listResults == treeResults, "Результаты операций различаются");
            listTimes[operation].push_back(listTime);
            treeTimes[operation].push_back(treeTime);
        };

        compare(0, [&](std::size_t i) { list.insert(values[i]); return 0; },
                   [&](std::size_t i) { tree.insert(values[i]); return 0; });
        checkOrder(list, tree);

        const auto query = [&](std::size_t i) {
            // Половина запросов ищет чётный ключ, половина — заведомо отсутствующий.
            return i % 2 == 0 ? 2 * indices[i] : -1;
        };
        compare(1, [&](std::size_t i) { return list.contains(query(i)); },
                   [&](std::size_t i) { return tree.find(query(i)) != tree.end(); });
        compare(3, [&](std::size_t i) { return list.getByIndex(indices[i]); },
                   [&](std::size_t i) { return *std::next(tree.cbegin(), indices[i]); });
        compare(2, [&](std::size_t i) { return list.remove(values[i]); },
                   [&](std::size_t i) {
                       const auto found = tree.find(values[i]);
                       if (found == tree.end()) {
                           return false;
                       }
                       tree.erase(found);
                       return true;
                   });
        checkOrder(list, tree);
        require(tree.size() == static_cast<std::size_t>(size), "Неверный размер после удаления");
    }

    for (int i = 0; i < 4; ++i) {
        std::cout << std::left << std::setw(9) << size << std::setw(14) << operations[i]
                  << std::right << std::setw(13) << median(listTimes[i])
                  << std::setw(17) << median(treeTimes[i]) << '\n';
    }
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    try {
        std::cout << "Лабораторная работа №2. Продвинутый уровень. Вариант 10\n";
        runChecks();
        std::cout << "\nМедианное время (нс на операцию), 9 серий по 512 операций\n";
        std::cout << std::left << std::setw(9) << "N" << "Операция      "
                  << std::right << std::setw(13) << "SortedList" << std::setw(17)
                  << "std::multiset" << '\n' << std::fixed << std::setprecision(2);
        for (int size : {1000, 5000, 20000}) {
            compareContainers(size);
        }
        std::cout << "\n[ОК] Результаты контейнеров совпали во всех сериях\n";
    } catch (const std::exception& error) {
        std::cerr << "[ОШИБКА] " << error.what() << '\n';
        return 1;
    }
}
