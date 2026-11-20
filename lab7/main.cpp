#include <algorithm>
#include <chrono>
#include <clocale>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif

// Графические снимки создаются только для маленькой демонстрации.
std::ofstream picture;
bool drawing = false;
int stepNumber = 0;

void draw(const std::vector<int>& values, const std::string& action) {
    if (!drawing) return;
    picture << "<section><h3>Шаг " << stepNumber++ << ". " << action << "</h3>"
            << "<svg viewBox='0 0 520 90' role='img' aria-label='Состояние массива'>";
    for (int i = 0; i < static_cast<int>(values.size()); ++i) {
        picture << "<rect x='" << 15 + i * 62 << "' y='12'"
                << " width='45' height='55' rx='6' fill='#2671a8'/>"
                << "<text x='" << 37 + i * 62 << "' y='46' fill='white' text-anchor='middle'>"
                << values[i] << "</text>";
    }
    picture << "</svg></section>\n";
}

void quickSort(std::vector<int>& values, int left, int right) {
    if (left >= right) return;
    int i = left, j = right;
    int pivot = values[left + (right - left) / 2];
    while (i <= j) {
        while (values[i] < pivot) ++i;
        while (values[j] > pivot) --j;
        if (i <= j) {
            std::swap(values[i], values[j]);
            ++i;
            --j;
        }
    }
    draw(values, "Разделение по опорному " + std::to_string(pivot));
    if (left < j) quickSort(values, left, j);
    if (i < right) quickSort(values, i, right);
}

void quickSort(std::vector<int>& values) {
    quickSort(values, 0, static_cast<int>(values.size()) - 1);
}

void mergeSort(std::vector<int>& values, std::vector<int>& buffer, int left, int right) {
    if (right - left <= 1) return;
    int middle = left + (right - left) / 2;
    mergeSort(values, buffer, left, middle);
    mergeSort(values, buffer, middle, right);
    int i = left, j = middle, position = left;
    while (i < middle && j < right) {
        if (values[i] <= values[j]) buffer[position++] = values[i++];
        else buffer[position++] = values[j++];
    }
    while (i < middle) buffer[position++] = values[i++];
    while (j < right) buffer[position++] = values[j++];
    for (int k = left; k < right; ++k) values[k] = buffer[k];
    draw(values, "Слияние диапазона [" + std::to_string(left) + ", " + std::to_string(right) + ")");
}

void mergeSort(std::vector<int>& values) {
    std::vector<int> buffer(values.size());
    mergeSort(values, buffer, 0, static_cast<int>(values.size()));
}

void heapify(std::vector<int>& values, int size, int root) {
    while (2 * root + 1 < size) {
        int child = 2 * root + 1;
        if (child + 1 < size && values[child + 1] > values[child]) ++child;
        if (values[root] >= values[child]) break;
        std::swap(values[root], values[child]);
        root = child;
    }
}

void heapSort(std::vector<int>& values) {
    int size = static_cast<int>(values.size());
    for (int i = size / 2 - 1; i >= 0; --i) heapify(values, size, i);
    draw(values, "Построена максимальная куча");
    for (int end = size - 1; end > 0; --end) {
        std::swap(values[0], values[end]);
        heapify(values, end, 0);
        draw(values, "Максимум перенесён в позицию " + std::to_string(end));
    }
}

void check(bool condition) {
    if (!condition) throw std::logic_error("Проверка не пройдена");
}

double measure(void (*sort)(std::vector<int>&), const std::vector<int>& source) {
    auto expected = source;
    std::sort(expected.begin(), expected.end());
    double times[5];
    for (double& time : times) {
        auto values = source;
        auto start = std::chrono::steady_clock::now();
        sort(values);
        auto finish = std::chrono::steady_clock::now();
        check(values == expected);
        time = std::chrono::duration<double, std::micro>(finish - start).count();
    }
    std::sort(times, times + 5);
    return times[2];
}

void createPicture() {
    picture.open("process.html");
    if (!picture) throw std::runtime_error("Не удалось создать process.html");
    picture << "<!doctype html><html lang='ru'><meta charset='utf-8'>"
            << "<title>Шаги сортировки</title><style>"
            << "body{font:18px Arial;max-width:900px;margin:30px auto;padding:0 20px}"
            << "section{border:1px solid #bbb;border-radius:8px;padding:12px;margin:15px 0}"
            << "svg{width:100%;max-width:650px}text{font:16px Arial}</style>"
            << "<h1>Лабораторная №7. Шаги трёх сортировок</h1>";
    const std::vector<int> source = {7, 2, 8, 1, 5, 3, 6, 4};
    drawing = true;
    picture << "<h2>Быстрая сортировка</h2>";
    auto values = source;
    stepNumber = 0;
    draw(values, "Исходный массив");
    quickSort(values);
    check(std::is_sorted(values.begin(), values.end()));
    draw(values, "Готовый массив");
    picture << "<h2>Сортировка слиянием</h2>";
    values = source;
    stepNumber = 0;
    draw(values, "Исходный массив");
    mergeSort(values);
    draw(values, "Готовый массив");
    picture << "<h2>Пирамидальная сортировка</h2>";
    values = source;
    stepNumber = 0;
    draw(values, "Исходный массив");
    heapSort(values);
    draw(values, "Готовый массив");
    picture << "</html>";
    drawing = false;
    picture.close();
    if (!picture) throw std::runtime_error("Ошибка записи process.html");
}

int main() {
    std::setlocale(LC_ALL, "Russian");
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    try {
        std::cout << "Лабораторная №7. Продвинутый уровень. Вариант 10\n";
        for (const std::vector<int>& source : std::vector<std::vector<int>>{
                 {}, {1}, {1,2,3,4}, {4,3,2,1}, {7,-2,7,0}, {5,5,5}}) {
            auto expected = source;
            std::sort(expected.begin(), expected.end());
            auto first = source, second = source, third = source;
            quickSort(first);
            mergeSort(second);
            heapSort(third);
            check(first == expected && second == expected && third == expected);
        }
        createPicture();
        std::cout << "Графические шаги сохранены в process.html. Откройте файл в браузере.\n";
        std::cout << "Медиана пяти запусков, мкс\nN | Быстрая | Слиянием | Пирамидальная\n";
        std::mt19937 random(42);
        std::uniform_int_distribution<int> number(-100000, 100000);
        std::cout << std::fixed << std::setprecision(2);
        for (int size : {1000, 2000, 5000, 10000}) {
            std::vector<int> source(size);
            for (int& value : source) value = number(random);
            double quick = measure(quickSort, source);
            double merge = measure(mergeSort, source);
            double heap = measure(heapSort, source);
            std::cout << size << " | " << quick << " | " << merge << " | " << heap << '\n';
        }
        std::cout << "Все проверки пройдены.\n";
    } catch (const std::exception& error) {
        std::cerr << "Ошибка: " << error.what() << '\n';
        return 1;
    }
}
