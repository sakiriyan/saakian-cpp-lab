#include <algorithm>
#include <cctype>
#include <clocale>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif

// Собственный стек на расширяемом массиве.
template <typename T>
class Stack {
    std::unique_ptr<T[]> data;
    int count = 0;
    int capacity = 0;
public:
    bool isEmpty() const { return count == 0; }
    int size() const { return count; }
    void push(const T& value) {
        if (count == capacity) {
            int newCapacity = capacity == 0 ? 4 : capacity * 2;
            auto buffer = std::make_unique<T[]>(newCapacity);
            for (int i = 0; i < count; ++i) {
                buffer[i] = data[i];
            }
            buffer[count] = value;
            data = std::move(buffer);
            capacity = newCapacity;
        } else {
            data[count] = value;
        }
        ++count;
    }
    const T& top() const {
        if (isEmpty()) throw std::underflow_error("Стек пуст");
        return data[count - 1];
    }
    T pop() {
        T value = top();
        --count;
        return value;
    }
};

// Собственный дек на двусвязных узлах.
template <typename T>
class Deque {
    struct Node {
        T value;
        Node* prev;
        Node* next;
    };
    Node* head = nullptr;
    Node* tail = nullptr;
    int count = 0;
public:
    Deque() = default;
    Deque(const Deque&) = delete;
    Deque& operator=(const Deque&) = delete;
    ~Deque() {
        while (!isEmpty()) pop_front();
    }
    bool isEmpty() const { return count == 0; }
    int size() const { return count; }
    const T& front() const {
        if (isEmpty()) throw std::underflow_error("Дек пуст");
        return head->value;
    }
    const T& back() const {
        if (isEmpty()) throw std::underflow_error("Дек пуст");
        return tail->value;
    }
    void push_front(const T& value) {
        Node* node = new Node{value, nullptr, head};
        if (head) head->prev = node;
        else tail = node;
        head = node;
        ++count;
    }
    void push_back(const T& value) {
        Node* node = new Node{value, tail, nullptr};
        if (tail) tail->next = node;
        else head = node;
        tail = node;
        ++count;
    }
    T pop_front() {
        T value = front();
        Node* old = head;
        head = head->next;
        if (head) head->prev = nullptr;
        else tail = nullptr;
        delete old;
        --count;
        return value;
    }
    T pop_back() {
        T value = back();
        Node* old = tail;
        tail = tail->prev;
        if (tail) tail->next = nullptr;
        else head = nullptr;
        delete old;
        --count;
        return value;
    }
};

// Очередь использует узлы нашего дека.
template <typename T>
class Queue {
    Deque<T> data;
public:
    void enqueue(const T& value) { data.push_back(value); }
    T dequeue() { return data.pop_front(); }
    const T& front() const { return data.front(); }
    bool isEmpty() const { return data.isEmpty(); }
    int size() const { return data.size(); }
};

class PriorityQueue {
    std::unique_ptr<int[]> data;
    int count = 0;
    int capacity = 0;
    void heapifyUp(int i) {
        while (i > 0) {
            int parent = (i - 1) / 2;
            if (data[parent] >= data[i]) break;
            std::swap(data[parent], data[i]);
            i = parent;
        }
    }
    void heapifyDown(int i) {
        while (2 * i + 1 < count) {
            int child = 2 * i + 1;
            if (child + 1 < count && data[child + 1] > data[child]) ++child;
            if (data[i] >= data[child]) break;
            std::swap(data[i], data[child]);
            i = child;
        }
    }
public:
    bool isEmpty() const { return count == 0; }
    int size() const { return count; }
    int top() const {
        if (isEmpty()) throw std::underflow_error("Приоритетная очередь пуста");
        return data[0];
    }
    void push(int value) {
        if (count == capacity) {
            int newCapacity = capacity == 0 ? 4 : capacity * 2;
            auto buffer = std::make_unique<int[]>(newCapacity);
            for (int i = 0; i < count; ++i) buffer[i] = data[i];
            data = std::move(buffer);
            capacity = newCapacity;
        }
        data[count] = value;
        heapifyUp(count);
        ++count;
    }
    int pop() {
        int value = top();
        --count;
        if (count > 0) {
            data[0] = data[count];
            heapifyDown(0);
        }
        return value;
    }
    void merge(const PriorityQueue& other) {
        // Самослияние должно удвоить исходные элементы ровно один раз.
        int total = count + other.count;
        auto buffer = std::make_unique<int[]>(total);
        for (int i = 0; i < count; ++i) buffer[i] = data[i];
        for (int i = 0; i < other.count; ++i) buffer[count + i] = other.data[i];
        data = std::move(buffer);
        count = capacity = total;
        for (int i = count / 2 - 1; i >= 0; --i) heapifyDown(i);
    }
};

int priority(char operation) {
    if (operation == '~') return 3;
    if (operation == '*' || operation == '/') return 2;
    return 1;
}

void calculateTop(Stack<double>& values, Stack<char>& operations) {
    char operation = operations.pop();
    double right = values.pop();
    if (operation == '~') {
        values.push(-right);
        return;
    }
    double left = values.pop();
    double result = 0;
    if (operation == '+') result = left + right;
    if (operation == '-') result = left - right;
    if (operation == '*') result = left * right;
    if (operation == '/') {
        if (right == 0) throw std::invalid_argument("Деление на ноль");
        result = left / right;
    }
    if (!std::isfinite(result)) throw std::overflow_error("Слишком большой результат");
    values.push(result);
}

double evaluate(const std::string& expression) {
    Stack<double> values;
    Stack<char> operations;
    bool needValue = true;
    for (std::size_t i = 0; i < expression.size();) {
        unsigned char ch = static_cast<unsigned char>(expression[i]);
        if (std::isspace(ch)) {
            ++i;
            continue;
        }
        if (std::isdigit(ch)) {
            if (!needValue) throw std::invalid_argument("Между числами нужен знак");
            double value = 0;
            while (i < expression.size() && std::isdigit(static_cast<unsigned char>(expression[i]))) {
                value = value * 10 + (expression[i++] - '0');
                if (!std::isfinite(value)) throw std::overflow_error("Слишком большое число");
            }
            values.push(value);
            needValue = false;
        } else if (ch == '(') {
            if (!needValue) throw std::invalid_argument("Перед скобкой нужен знак");
            operations.push('(');
            ++i;
        } else if (ch == ')') {
            if (needValue) throw std::invalid_argument("Пустая или незавершённая скобка");
            while (!operations.isEmpty() && operations.top() != '(') calculateTop(values, operations);
            if (operations.isEmpty()) throw std::invalid_argument("Лишняя закрывающая скобка");
            operations.pop();
            ++i;
        } else {
            if (ch != '+' && ch != '-' && ch != '*' && ch != '/')
                throw std::invalid_argument("Неизвестный символ");
            if (needValue) {
                if (ch == '-') operations.push('~');
                else if (ch != '+') throw std::invalid_argument("Ожидалось число");
            } else {
                while (!operations.isEmpty() && operations.top() != '(' &&
                       priority(operations.top()) >= priority(static_cast<char>(ch))) {
                    calculateTop(values, operations);
                }
                operations.push(static_cast<char>(ch));
                needValue = true;
            }
            ++i;
        }
    }
    if (needValue) throw std::invalid_argument("Выражение не завершено");
    while (!operations.isEmpty()) {
        if (operations.top() == '(') throw std::invalid_argument("Скобка не закрыта");
        calculateTop(values, operations);
    }
    if (values.size() != 1) throw std::invalid_argument("Неверное выражение");
    return values.pop();
}

void removeOdd(Queue<int>& queue) {
    int size = queue.size();
    for (int i = 0; i < size; ++i) {
        int value = queue.dequeue();
        if (value % 2 == 0) queue.enqueue(value);
    }
}

std::vector<int> windowMaximum(const std::vector<int>& values, int k) {
    if (k <= 0 || k > static_cast<int>(values.size()))
        throw std::invalid_argument("Неверная длина окна");
    Deque<int> indices;
    std::vector<int> result;
    for (int i = 0; i < static_cast<int>(values.size()); ++i) {
        while (!indices.isEmpty() && indices.front() <= i - k) indices.pop_front();
        while (!indices.isEmpty() && values[indices.back()] <= values[i]) indices.pop_back();
        indices.push_back(i);
        if (i >= k - 1) result.push_back(values[indices.front()]);
    }
    return result;
}

void check(bool condition) {
    if (!condition) throw std::logic_error("Проверка не пройдена");
}

int main() {
    std::setlocale(LC_ALL, "Russian");
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    try {
        std::cout << "Лабораторная №5. Вариант 10\n";
        for (const std::string expression : {"-5 + 3", "2 * -(3 + 4)", "--5", "8 / 2 - -3"}) {
            std::cout << expression << " = " << evaluate(expression) << '\n';
        }
        check(evaluate("-5 + 3") == -2 && evaluate("2 * -(3 + 4)") == -14);
        check(evaluate("--5") == 5 && evaluate("8 / 2 - -3") == 7);
        for (const std::string expression : {"", "2 +", "1/0", "(2+3", "2 3", "a"}) {
            bool caught = false;
            try { evaluate(expression); }
            catch (const std::exception&) { caught = true; }
            check(caught);
        }
        Queue<int> queue;
        for (int x : {-3, -2, 0, 1, 4, 7}) queue.enqueue(x);
        removeOdd(queue);
        std::cout << "Очередь без нечётных: ";
        for (int expected : {-2, 0, 4}) {
            int value = queue.dequeue();
            check(value == expected);
            std::cout << value << ' ';
        }
        check(queue.isEmpty());
        removeOdd(queue);
        std::cout << "\nМаксимумы окон длины 3: ";
        auto maxima = windowMaximum({1, 3, -1, -3, 5, 3, 6, 7}, 3);
        check(maxima == std::vector<int>({3, 3, 5, 5, 6, 7}));
        for (int x : maxima) std::cout << x << ' ';
        check(windowMaximum({4, 4}, 1) == std::vector<int>({4, 4}));
        Deque<std::string> fruits;
        fruits.push_front("яблоко");
        fruits.push_back("груша");
        check(fruits.pop_front() == "яблоко" && fruits.pop_back() == "груша");
        PriorityQueue first, second;
        for (int x : {5, 1, 9}) first.push(x);
        for (int x : {8, 2, 9}) second.push(x);
        first.merge(second);
        std::cout << "\nСлияние приоритетных очередей: ";
        for (int expected : {9, 9, 8, 5, 2, 1}) {
            int value = first.pop();
            check(value == expected);
            std::cout << value << ' ';
        }
        check(second.size() == 3);
        second.merge(second);
        check(second.size() == 6);
        for (int expected : {9, 9, 8, 8, 2, 2}) check(second.pop() == expected);
        int errors = 0;
        try { Stack<int> empty; empty.pop(); } catch (const std::underflow_error&) { ++errors; }
        try { queue.dequeue(); } catch (const std::underflow_error&) { ++errors; }
        try { fruits.pop_back(); } catch (const std::underflow_error&) { ++errors; }
        try { first.pop(); } catch (const std::underflow_error&) { ++errors; }
        check(errors == 4);
        std::cout << "\nПустые структуры: четыре исключения перехвачены.\nВсе проверки пройдены.\n";
    } catch (const std::exception& error) {
        std::cerr << "Ошибка: " << error.what() << '\n';
        return 1;
    }
}
