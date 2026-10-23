  #include <algorithm>
  #include <chrono>
  #include <clocale>
  #include <cstddef>
  #include <iomanip>
  #include <iostream>
  #include <iterator>
  #include <stdexcept>
  #include <string>
  #include <utility>
  #include <vector>

  #ifdef _MSC_VER
  #pragma execution_character_set("utf-8")
  #endif

  #ifdef _WIN32
  #include <windows.h>
  #endif

  template <typename Node, bool Doubly>
  struct PreviousLink {};

  template <typename Node>
  struct PreviousLink<Node, true> {
      Node* prev = nullptr;
  };

  template <typename T, bool Doubly>
  class LinkedList {
      struct Node : PreviousLink<Node, Doubly> {
          T value;
          Node* next = nullptr;
          explicit Node(const T& item) : value(item) {}
      };

      Node* head = nullptr;
      Node* tail = nullptr;
      std::size_t count = 0;

      // для двусвязного списка предшественник хранится в самом узле.
      Node* previous(Node* node) const noexcept {
          if constexpr (Doubly) {
              return node->prev;
          } else {
              Node* current = head;
              while (current->next != node) {
                  current = current->next;
              }
              return current;
          }
      }

      void linkAfter(Node* left, Node* node) noexcept {
          Node* right = left ? left->next : head;
          node->next = right;
          if (left) {
              left->next = node;
          } else {
              head = node;
          }
          if constexpr (Doubly) {
              node->prev = left;
              if (right) {
                  right->prev = node;
              }
          }
          if (!right) {
              tail = node;
          }
          ++count;
      }

      void unlink(Node* left, Node* node) noexcept {
          Node* right = node->next;
          if (left) {
              left->next = right;
          } else {
              head = right;
          }
          if constexpr (Doubly) {
              if (right) {
                  right->prev = left;
              }
          }
          if (!right) {
              tail = left;
          }
          delete node;
          --count;
      }

  public:
      template <bool Reverse>
      class Iterator {
          const Node* current;
          friend class LinkedList;
          explicit Iterator(const Node* node) : current(node) {}

      public:
          using iterator_category = std::forward_iterator_tag;
          using value_type = T;
          using difference_type = std::ptrdiff_t;
          using pointer = const T*;
          using reference = const T&;

          Iterator() : current(nullptr) {}
          reference operator*() const { return current->value; }
          pointer operator->() const { return &current->value; }

          Iterator& operator++() {
              if constexpr (Reverse) {
                  current = current->prev;
              } else {
                  current = current->next;
              }
              return *this;
          }

          Iterator operator++(int) {
              Iterator old = *this;
              ++*this;
              return old;
          }

          bool operator==(const Iterator& other) const { return current == other.current; }
          bool operator!=(const Iterator& other) const { return !(*this == other); }
      };

      using const_iterator = Iterator<false>;
      using reverse_iterator = Iterator<true>;

      LinkedList() = default;
      ~LinkedList() { clear(); }
      LinkedList(const LinkedList&) = delete;
      LinkedList& operator=(const LinkedList&) = delete;

      const_iterator begin() const noexcept { return const_iterator(head); }
      const_iterator end() const noexcept { return const_iterator(nullptr); }

      reverse_iterator rbegin() const noexcept {
          static_assert(Doubly, "Обратный обход доступен для двусвязного списка");
          return reverse_iterator(tail);
      }

      reverse_iterator rend() const noexcept {
          static_assert(Doubly, "Обратный обход доступен для двусвязного списка");
          return reverse_iterator(nullptr);
      }

      bool empty() const noexcept { return count == 0; }
      std::size_t size() const noexcept { return count; }

      void push_front(const T& value) { linkAfter(nullptr, new Node(value)); }
      void push_back(const T& value) { linkAfter(tail, new Node(value)); }

      void pop_front() {
          if (empty()) {
              throw std::underflow_error("Список пуст");
          }
          unlink(nullptr, head);
      }

      void pop_back() {
          if (empty()) {
              throw std::underflow_error("Список пуст");
          }
          unlink(head == tail ? nullptr : previous(tail), tail);
      }

      const_iterator find(const T& value) const {
          auto it = begin();
          while (it != end() && !(*it == value)) {
              ++it;
          }
          return it;
      }

      bool remove(const T& value) {
          Node* left = nullptr;
          Node* node = head;
          while (node && !(node->value == value)) {
              left = node;
              node = node->next;
          }
          if (!node) {
              return false;
          }
          unlink(left, node);
          return true;
      }

      // позиция должна указывать на существующий узел этого списка.
      void insert_after(const_iterator position, const T& value) {
          if (position == end()) {
              throw std::out_of_range("Нет узла для вставки");
          }
          linkAfter(const_cast<Node*>(position.current), new Node(value));
      }

      // позиция должна указывать на существующий узел этого списка.
      void erase(const_iterator position) {
          if (position == end()) {
              throw std::out_of_range("Нет узла для удаления");
          }
          Node* node = const_cast<Node*>(position.current);
          unlink(node == head ? nullptr : previous(node), node);
      }

      void removeEverySecond() noexcept {
          for (Node* node = head; node && node->next; node = node->next) {
              unlink(node, node->next);
          }
      }

      void clear() noexcept {
          while (head) {
              unlink(nullptr, head);
          }
      }

      bool operator==(const LinkedList& other) const {
          return size() == other.size() && std::equal(begin(), end(), other.begin());
      }

      bool operator!=(const LinkedList& other) const { return !(*this == other); }
  };

  template <typename T>
  using SinglyLinkedList = LinkedList<T, false>;

  template <typename T>
  using DoublyLinkedList = LinkedList<T, true>;

  template <typename Iterator>
  void print(Iterator first, Iterator last) {
      for (; first != last; ++first) {
          std::cout << *first << ' ';
      }
      std::cout << '\n';
  }

  void check(bool condition) {
      if (!condition) {
          throw std::logic_error("Проверка списка не прошла");
      }
  }

  template <typename List>
  void checkOperations() {
      List list;
      check(list.empty() && list.find(10) == list.end() && !list.remove(10));
      bool caught = false;
      try {
          list.pop_back();
      } catch (const std::underflow_error&) {
          caught = true;
      }
      check(caught);
      list.push_back(20);
      list.push_front(10);
      list.insert_after(list.find(10), 15);
      check(std::vector<int>(list.begin(), list.end()) == std::vector<int>{10, 15, 20});
      list.erase(list.find(15));
      check(list.size() == 2 && list.find(15) == list.end());
      list.pop_front();
      list.pop_back();
      check(list.empty());
      list.push_back(30);
      check(list.remove(30) && list.empty());
      list.push_front(40);
      list.clear();
      check(list.size() == 0 && list.begin() == list.end());
      List other;
      check(list == other);
      other.push_back(7);
      check(list != other);
  }

  template <typename List>
  double measurePopBack(int size) {
      using Clock = std::chrono::steady_clock;
      constexpr int operations = 512;
      double samples[9];
      List list;
      for (int i = 0; i < size; ++i) {
          list.push_back(i);
      }
      for (double& sample : samples) {
          for (int i = 0; i < operations; ++i) {
              list.push_back(size + i);
          }
          const auto start = Clock::now();
          for (int i = 0; i < operations; ++i) {
              list.pop_back();
          }
          const auto finish = Clock::now();
          check(list.size() == static_cast<std::size_t>(size));
          sample = std::chrono::duration<double, std::nano>(finish - start).count() / operations;
      }
      std::sort(std::begin(samples), std::end(samples));
      return samples[4];
  }

  int main() {
  #ifdef _WIN32
      std::setlocale(LC_ALL, "Russian");
      // русские строки программы записаны в UTF-8.
      SetConsoleOutputCP(CP_UTF8);
  #else
      std::setlocale(LC_ALL, "");
  #endif
      try {
          std::cout << "Лабораторная №4. Продвинутый уровень. Вариант 10\n";
          checkOperations<SinglyLinkedList<int>>();
          checkOperations<DoublyLinkedList<int>>();
          SinglyLinkedList<int> numbers;
          numbers.push_front(20);
          numbers.push_front(10);
          numbers.push_back(30);
          std::cout << "Односвязный список: ";
          print(numbers.begin(), numbers.end());
          numbers.clear();
          std::cout << "Размер после очистки: " << numbers.size() << '\n';

          DoublyLinkedList<std::string> fruits;
          check(fruits.rbegin() == fruits.rend());
          fruits.push_back("яблоко");
          check(std::vector<std::string>(fruits.rbegin(), fruits.rend()) ==
                std::vector<std::string>{"яблоко"});
          fruits.push_back("груша");
          fruits.push_back("слива");
          fruits.push_back("абрикос");
          std::cout << "Прямой обход: ";
          print(fruits.begin(), fruits.end());
          check(std::vector<std::string>(fruits.rbegin(), fruits.rend()) ==
                std::vector<std::string>{"абрикос", "слива", "груша", "яблоко"});
          std::cout << "Обратный обход: ";
          print(fruits.rbegin(), fruits.rend());
          fruits.removeEverySecond();
          check(std::vector<std::string>(fruits.rbegin(), fruits.rend()) ==
                std::vector<std::string>{"слива", "яблоко"});
          std::cout << "После удаления каждого второго: ";
          print(fruits.begin(), fruits.end());
          std::cout << "Все проверки пройдены.\n\n";

          std::cout << "Удаление с конца, медиана в нс на операцию\n";
          std::cout << "Размер | Односвязный | Двусвязный\n" << std::fixed << std::setprecision(2);
          for (int size : {1000, 5000, 20000}) {
              const double singly = measurePopBack<SinglyLinkedList<int>>(size);
              const double doubly = measurePopBack<DoublyLinkedList<int>>(size);
              std::cout << size << " | " << singly << " | " << doubly << '\n';
          }
      } catch (const std::exception& error) {
          std::cerr << "Ошибка: " << error.what() << '\n';
          return 1;
      }
  }
