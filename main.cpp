    #include <iostream>
    #include <string>
    #include <vector>
    #include <windows.h>

    using namespace std;

    class Purchase {
        string product;
        int price;
        static int count;

    public:
        Purchase(string p, int s) {
            product = p;
            price = s;
            count++;
            cout << "Создана покупка: " << product << endl;
        }

        ~Purchase() {
            count--;
            cout << "Удалена покупка: " << product << endl;
        }

        void show() {
            cout << "Покупка: " << product << ", цена: " << price << " руб." << endl;
        }

        static int getCount() {
            return count;
        }
    };

    int Purchase::count = 0;

    class Customer {
        string name;
        Purchase purchase;

    public:
        Customer(string n, string p, int price) : purchase(p, price) {
            name = n;
            cout << "Создан клиент: " << name << endl;
        }

        ~Customer() {
            cout << "Удалён клиент: " << name << endl;
        }

        void show() {
            cout << "Клиент: " << name << endl;
            purchase.show();
        }
    };

    class Store {
        vector<Customer*> customers;

        Store() {
            cout << "Магазин создан" << endl;
        }

    public:
        Store(const Store&) = delete;
        Store& operator=(const Store&) = delete;

        static Store& getStore() {
            static Store store;
            return store;
        }

        void addCustomer(Customer* customer) {
            if (customer != nullptr) {
                customers.push_back(customer);
            }
        }

        void show() {
            for (Customer* customer : customers) {
                customer->show();
            }
        }

        void clear() {
            customers.clear();
        }
    };

    int main() {

        setlocale(LC_ALL, "Russian");
        cout << "Лабораторная работа №1. Вариант 10" << endl;
        cout << "Интернет-магазин" << endl << endl;

        Store& shop = Store::getStore();
        Store& shop2 = Store::getStore();

        if (&shop == &shop2) {
            cout << "Singleton работает: магазин один" << endl;
        }

        {
            Customer ivan("Иван", "Ноутбук", 75000);
            Customer maria("Мария", "Телефон", 30000);

            shop.addCustomer(&ivan);
            shop.addCustomer(&maria);

            cout << "\nСписок клиентов:" << endl;
            shop.show();

            cout << "\nКоличество покупок: " << Purchase::getCount() << endl;
            shop.clear();
            cout << "Клиенты удалены из списка магазина" << endl;
        }

        cout << "Количество покупок после удаления клиентов: "
             << Purchase::getCount() << endl;
        cout << "Магазин продолжает существовать" << endl;

        return 0;
    }
