#include <algorithm>
#include <climits>
#include <clocale>
#include <iostream>
#include <memory>
#include <queue>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif

struct Node {
    int value;
    std::unique_ptr<Node> left, right;
    explicit Node(int number) : value(number) {}
};

class BinaryTree {
protected:
    std::unique_ptr<Node> root;
private:
    std::unique_ptr<Node> build(const std::vector<int>& values, std::size_t index) {
        if (index >= values.size()) return nullptr;
        auto node = std::make_unique<Node>(values[index]);
        node->left = build(values, 2 * index + 1);
        node->right = build(values, 2 * index + 2);
        return node;
    }
    void preOrder(const Node* node, std::vector<int>& result) const {
        if (!node) return;
        result.push_back(node->value);
        preOrder(node->left.get(), result);
        preOrder(node->right.get(), result);
    }
    void inOrder(const Node* node, std::vector<int>& result) const {
        if (!node) return;
        inOrder(node->left.get(), result);
        result.push_back(node->value);
        inOrder(node->right.get(), result);
    }
    void postOrder(const Node* node, std::vector<int>& result) const {
        if (!node) return;
        postOrder(node->left.get(), result);
        postOrder(node->right.get(), result);
        result.push_back(node->value);
    }
    int height(const Node* node) const {
        if (!node) return 0;
        return 1 + std::max(height(node->left.get()), height(node->right.get()));
    }
    bool perfect(const Node* node, int level, int treeHeight) const {
        if (!node) return true;
        if (!node->left && !node->right) return level == treeHeight;
        if (!node->left || !node->right) return false;
        return perfect(node->left.get(), level + 1, treeHeight) &&
               perfect(node->right.get(), level + 1, treeHeight);
    }
    void show(const Node* node, const std::string& indent, const std::string& side) const {
        if (!node) return;
        show(node->right.get(), indent + "    ", "П: ");
        std::cout << indent << side << node->value << '\n';
        show(node->left.get(), indent + "    ", "Л: ");
    }
public:
    BinaryTree() = default;
    explicit BinaryTree(const std::vector<int>& levelValues) { root = build(levelValues, 0); }
    virtual ~BinaryTree() = default;
    std::vector<int> preOrder() const {
        std::vector<int> result;
        preOrder(root.get(), result);
        return result;
    }
    std::vector<int> inOrder() const {
        std::vector<int> result;
        inOrder(root.get(), result);
        return result;
    }
    std::vector<int> postOrder() const {
        std::vector<int> result;
        postOrder(root.get(), result);
        return result;
    }
    std::vector<int> levelOrder() const {
        std::vector<int> result;
        std::queue<const Node*> nodes;
        if (root) nodes.push(root.get());
        while (!nodes.empty()) {
            const Node* node = nodes.front();
            nodes.pop();
            result.push_back(node->value);
            if (node->left) nodes.push(node->left.get());
            if (node->right) nodes.push(node->right.get());
        }
        return result;
    }
    int height() const { return height(root.get()); }
    bool isPerfect() const { return perfect(root.get(), 1, height()); }
    void show() const {
        if (!root) std::cout << "(пустое дерево)\n";
        else show(root.get(), "", "Корень: ");
    }
};

class BST : public BinaryTree {
    void insert(std::unique_ptr<Node>& node, int value) {
        if (!node) node = std::make_unique<Node>(value);
        else if (value < node->value) insert(node->left, value);
        else if (value > node->value) insert(node->right, value);
    }
    bool remove(std::unique_ptr<Node>& node, int value) {
        if (!node) return false;
        if (value < node->value) return remove(node->left, value);
        if (value > node->value) return remove(node->right, value);
        if (!node->left) {
            node = std::move(node->right);
        } else if (!node->right) {
            node = std::move(node->left);
        } else {
            Node* successor = node->right.get();
            while (successor->left) successor = successor->left.get();
            node->value = successor->value;
            remove(node->right, successor->value);
        }
        return true;
    }
    std::unique_ptr<Node> restore(const std::vector<int>& values, std::size_t& index,
                                  long long lower, long long upper) {
        if (index == values.size() || values[index] <= lower || values[index] >= upper)
            return nullptr;
        int value = values[index++];
        auto node = std::make_unique<Node>(value);
        node->left = restore(values, index, lower, value);
        node->right = restore(values, index, value, upper);
        return node;
    }
public:
    void insert(int value) { insert(root, value); }
    bool remove(int value) { return remove(root, value); }
    bool search(int value) const {
        const Node* node = root.get();
        while (node) {
            if (value == node->value) return true;
            node = value < node->value ? node->left.get() : node->right.get();
        }
        return false;
    }
    int minimum() const {
        if (!root) throw std::underflow_error("Дерево пусто");
        const Node* node = root.get();
        while (node->left) node = node->left.get();
        return node->value;
    }
    int maximum() const {
        if (!root) throw std::underflow_error("Дерево пусто");
        const Node* node = root.get();
        while (node->right) node = node->right.get();
        return node->value;
    }
    void fromPreOrder(const std::vector<int>& values) {
        std::size_t index = 0;
        auto restored = restore(values, index, LLONG_MIN, LLONG_MAX);
        if (index != values.size()) throw std::invalid_argument("Неверный префиксный обход BST");
        root = std::move(restored);
    }
};

void print(const std::vector<int>& values) {
    for (int value : values) std::cout << value << ' ';
    std::cout << '\n';
}

void check(bool condition) {
    if (!condition) throw std::logic_error("Проверка не пройдена");
}

void tests() {
    BinaryTree empty;
    check(empty.height() == 0 && empty.isPerfect() && empty.levelOrder().empty());
    BinaryTree one({5}), incomplete({1,2,3,4,5,6});
    check(one.isPerfect() && !incomplete.isPerfect());
    BST tree;
    tree.fromPreOrder({4,2,1,3,6,5,7});
    check(tree.preOrder() == std::vector<int>({4,2,1,3,6,5,7}));
    check(tree.inOrder() == std::vector<int>({1,2,3,4,5,6,7}));
    check(tree.postOrder() == std::vector<int>({1,3,2,5,7,6,4}));
    check(tree.levelOrder() == std::vector<int>({4,2,6,1,3,5,7}));
    check(tree.remove(1)); // Лист.
    check(tree.remove(2)); // Один потомок.
    check(tree.remove(4)); // Два потомка.
    check(!tree.remove(99) && !tree.search(4) && tree.search(7));
    check(tree.inOrder() == std::vector<int>({3,5,6,7}));
    auto before = tree.inOrder();
    bool invalid = false;
    try { tree.fromPreOrder({4,6,2}); } catch (const std::invalid_argument&) { invalid = true; }
    check(invalid && tree.inOrder() == before);
    tree.fromPreOrder({INT_MIN, INT_MAX});
    check(tree.minimum() == INT_MIN && tree.maximum() == INT_MAX);
    tree.insert(INT_MAX);
    check(tree.inOrder().size() == 2);
    tree.fromPreOrder({});
    check(!tree.remove(1));
    bool caught = false;
    try { tree.minimum(); } catch (const std::underflow_error&) { caught = true; }
    check(caught);
}

int main() {
    std::setlocale(LC_ALL, "Russian");
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    try {
        std::cout << "Лабораторная №9. Вариант 10\n";
        tests();
        BinaryTree tree({4,2,6,1,3,5,7});
        std::cout << "Прямой обход: "; print(tree.preOrder());
        std::cout << "Симметричный обход: "; print(tree.inOrder());
        std::cout << "Обратный обход: "; print(tree.postOrder());
        std::cout << "По уровням: "; print(tree.levelOrder());
        std::cout << "Дерево совершенное: " << (tree.isPerfect() ? "да" : "нет") << '\n';
        BST restored;
        restored.fromPreOrder({8,3,1,6,4,7,10,14,13});
        std::cout << "BST из префиксного обхода:\n";
        restored.show();

        BST randomTree;
        std::mt19937 random(42);
        std::uniform_int_distribution<int> number(0, 99);
        std::cout << "20 случайных чисел: ";
        for (int i = 0; i < 20; ++i) {
            int value = number(random);
            std::cout << value << ' ';
            randomTree.insert(value);
        }
        std::cout << "\nBST по возрастанию (повторы пропущены): "; print(randomTree.inOrder());
        std::cout << "Минимум: " << randomTree.minimum() << ", максимум: " << randomTree.maximum()
                  << ", высота в узлах: " << randomTree.height() << '\n';
        std::cout << "Введите целое число для удаления: ";
        std::string input;
        if (!std::getline(std::cin, input)) throw std::invalid_argument("Не введено число");
        std::size_t used = 0;
        int value = std::stoi(input, &used);
        if (input.find_first_not_of(" \t\r", used) != std::string::npos)
            throw std::invalid_argument("После числа найдены лишние символы");
        std::cout << (randomTree.remove(value) ? "Элемент удалён.\n" : "Элемент не найден.\n");
        std::cout << "Дерево после удаления (правые ветви сверху):\n";
        randomTree.show();
        std::cout << "Симметричный обход: "; print(randomTree.inOrder());
        std::cout << "Все проверки пройдены.\n";
    } catch (const std::exception& error) {
        std::cerr << "Ошибка: " << error.what() << '\n';
        return 1;
    }
}
