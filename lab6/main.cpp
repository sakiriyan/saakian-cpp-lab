#include <algorithm>
#include <chrono>
#include <clocale>
#include <iomanip>
#include <iostream>
#include <list>
#include <random>
#include <stdexcept>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif

long long factorial(int n) {
    if (n < 0 || n > 20) throw std::invalid_argument("Для факториала нужно 0 <= n <= 20");
    if (n == 0) return 1;
    return n * factorial(n - 1);
}

long long factorialLoop(int n) {
    if (n < 0 || n > 20) throw std::invalid_argument("Для факториала нужно 0 <= n <= 20");
    long long value = 1;
    for (int i = 2; i <= n; ++i) value *= i;
    return value;
}

long long fibonacci(int n) {
    if (n < 0 || n > 35) throw std::invalid_argument("Для прямой рекурсии нужно 0 <= n <= 35");
    if (n < 2) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

long long fibonacciLoop(int n) {
    if (n < 0 || n > 92) throw std::invalid_argument("Нужно 0 <= n <= 92");
    long long a = 0, b = 1;
    for (int i = 0; i < n; ++i) {
        if (i == n - 1) return b;
        long long next = a + b;
        a = b;
        b = next;
    }
    return a;
}

// 0 — свободная клетка, 1 — стена. path отмечает найденный маршрут.
bool solveMaze(const int maze[5][5], int row, int column, bool visited[5][5], bool path[5][5]) {
    if (row < 0 || row >= 5 || column < 0 || column >= 5 ||
        maze[row][column] == 1 || visited[row][column]) return false;
    visited[row][column] = true;
    path[row][column] = true;
    if (row == 4 && column == 4) return true;
    if (solveMaze(maze, row + 1, column, visited, path) ||
        solveMaze(maze, row, column + 1, visited, path) ||
        solveMaze(maze, row - 1, column, visited, path) ||
        solveMaze(maze, row, column - 1, visited, path)) return true;
    path[row][column] = false;
    return false;
}

bool safeQueen(const int columns[4], int row, int column) {
    for (int previous = 0; previous < row; ++previous) {
        int difference = columns[previous] - column;
        if (difference == 0 || difference == row - previous || difference == previous - row)
            return false;
    }
    return true;
}

int solveQueens(int row, int columns[4], bool show) {
    if (row == 4) {
        if (show) {
            std::cout << "Решение для ферзей (столбцы по строкам): ";
            for (int i = 0; i < 4; ++i) std::cout << columns[i] + 1 << ' ';
            std::cout << '\n';
        }
        return 1;
    }
    int solutions = 0;
    for (int column = 0; column < 4; ++column) {
        if (safeQueen(columns, row, column)) {
            columns[row] = column;
            solutions += solveQueens(row + 1, columns, show);
        }
    }
    return solutions;
}

void bubbleSort(std::vector<int>& values) {
    for (int end = static_cast<int>(values.size()) - 1; end > 0; --end) {
        bool changed = false;
        for (int i = 0; i < end; ++i) {
            if (values[i] > values[i + 1]) {
                std::swap(values[i], values[i + 1]);
                changed = true;
            }
        }
        if (!changed) break;
    }
}

void insertionSort(std::vector<int>& values) {
    for (int i = 1; i < static_cast<int>(values.size()); ++i) {
        int value = values[i];
        int j = i;
        while (j > 0 && values[j - 1] > value) {
            values[j] = values[j - 1];
            --j;
        }
        values[j] = value;
    }
}

void selectionSort(std::vector<int>& values) {
    for (int i = 0; i < static_cast<int>(values.size()); ++i) {
        int smallest = i;
        for (int j = i + 1; j < static_cast<int>(values.size()); ++j) {
            if (values[j] < values[smallest]) smallest = j;
        }
        std::swap(values[i], values[smallest]);
    }
}

void selectionSortList(std::list<int>& values) {
    for (auto current = values.begin(); current != values.end(); ++current) {
        auto smallest = current;
        for (auto candidate = current; candidate != values.end(); ++candidate) {
            if (*candidate < *smallest) smallest = candidate;
        }
        std::iter_swap(current, smallest);
    }
}

void check(bool condition) {
    if (!condition) throw std::logic_error("Проверка не пройдена");
}

double measureSort(void (*sort)(std::vector<int>&), const std::vector<int>& source) {
    double samples[5];
    auto expected = source;
    std::sort(expected.begin(), expected.end());
    for (double& sample : samples) {
        auto values = source;
        auto start = std::chrono::steady_clock::now();
        sort(values);
        auto finish = std::chrono::steady_clock::now();
        check(values == expected);
        sample = std::chrono::duration<double, std::micro>(finish - start).count();
    }
    std::sort(samples, samples + 5);
    return samples[2];
}

int main() {
    std::setlocale(LC_ALL, "Russian");
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    try {
        std::cout << "Лабораторная №6. Вариант 10\n";
        for (int n = 0; n <= 20; ++n) check(factorial(n) == factorialLoop(n));
        for (int n = 0; n <= 20; ++n) check(fibonacci(n) == fibonacciLoop(n));
        std::cout << "Факториал 5: " << factorial(5) << "\nФибоначчи 10: " << fibonacci(10) << '\n';
        int maze[5][5] = {
            {0,1,0,0,0}, {0,1,0,1,0}, {0,0,0,1,0},
            {1,1,0,1,0}, {0,0,0,0,0}
        };
        bool visited[5][5] = {}, path[5][5] = {};
        check(solveMaze(maze, 0, 0, visited, path));
        std::cout << "Маршрут: * — путь, # — стена, . — свободно\n";
        for (int row = 0; row < 5; ++row) {
            for (int column = 0; column < 5; ++column)
                std::cout << (maze[row][column] ? '#' : (path[row][column] ? '*' : '.')) << ' ';
            std::cout << '\n';
        }
        maze[4][4] = 1;
        bool visitedBlocked[5][5] = {}, pathBlocked[5][5] = {};
        check(!solveMaze(maze, 0, 0, visitedBlocked, pathBlocked));
        int columns[4] = {};
        int solutions = solveQueens(0, columns, true);
        check(solutions == 2);
        std::cout << "Всего решений для четырёх ферзей: " << solutions << '\n';

        for (const std::vector<int>& source : std::vector<std::vector<int>>{
                 {}, {1}, {1,2,3}, {3,2,1}, {4,-1,4,0}, {2,2,2}}) {
            auto expected = source;
            std::sort(expected.begin(), expected.end());
            auto first = source, second = source, third = source;
            bubbleSort(first);
            insertionSort(second);
            selectionSort(third);
            std::list<int> linked(source.begin(), source.end());
            selectionSortList(linked);
            check(first == expected && second == expected && third == expected);
            check(std::vector<int>(linked.begin(), linked.end()) == expected);
        }
        std::list<int> linked = {5, 1, 4, 2, 3};
        selectionSortList(linked);
        std::cout << "Список после сортировки выбором: ";
        for (int value : linked) std::cout << value << ' ';
        int errors = 0;
        try { factorial(-1); } catch (const std::invalid_argument&) { ++errors; }
        try { factorial(21); } catch (const std::invalid_argument&) { ++errors; }
        try { fibonacci(-1); } catch (const std::invalid_argument&) { ++errors; }
        check(errors == 3);
        std::cout << "\nВсе проверки пройдены.\n";
        std::cout << "Медиана пяти запусков, мкс\nN | Пузырьком | Вставками | Выбором\n";
        std::mt19937 random(42);
        std::uniform_int_distribution<int> number(-10000, 10000);
        std::cout << std::fixed << std::setprecision(2);
        for (int size : {100, 1000, 3000}) {
            std::vector<int> source(size);
            for (int& value : source) value = number(random);
            double bubble = measureSort(bubbleSort, source);
            double insertion = measureSort(insertionSort, source);
            double selection = measureSort(selectionSort, source);
            std::cout << size << " | " << bubble << " | " << insertion << " | " << selection << '\n';
        }
    } catch (const std::exception& error) {
        std::cerr << "Ошибка: " << error.what() << '\n';
        return 1;
    }
}
