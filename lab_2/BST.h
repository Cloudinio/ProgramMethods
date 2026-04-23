#include <vector>
#include <string>
#include "../lab_1/data.h"

using namespace std;

/**
 * @brief Класс бинарного дерева поиска для поиска объектов Player по ключу.
 * В качестве ключа используется первое нечисловое поле объекта Player, а именно ФИО игрока.
 * Дерево хранит не сами объекты Player, а только ключ и список индексов элементов исходного массива, у которых этот ключ совпадает.
 * Это позволяет:
 * - искать все вхождения по ключу;
 * - не дублировать сами объекты в узлах дерева.
 */
class BST {
private:
    /**
     * @brief Внутренняя структура узла бинарного дерева поиска.
     * Каждый узел хранит:
     * - ключ;
     * - индексы всех элементов массива с таким ключом;
     * - указатели на левое и правое поддеревья.
     */
    struct Node {
        string key;
        vector<int> indices;
        Node* left;
        Node* right;

        /**
         * @brief Конструктор узла дерева.
         * @param k Ключ узла.
         * @param index Индекс элемента массива, соответствующего этому ключу.
         */
        Node(const string& k, int index)
            : key(k), indices{index}, left(nullptr), right(nullptr) {}
    };

    Node* root = nullptr;

private:
    /**
     * @brief Получить ключ из объекта Player.
     * @param p Объект Player.
     * @return ФИО игрока как строковый ключ.
     */
    string getKey(const Player& p) const {
        return p.GetFullName();
    }

    /**
     * @brief Рекурсивная вставка узла в дерево.
     * Если узел с таким ключом уже существует, новый узел не создается, а индекс добавляется в список indices существующего узла.
     * @param node Текущий узел дерева.
     * @param key Ключ для вставки.
     * @param index Индекс элемента в исходном массиве.
     * @return Указатель на корень поддерева после вставки.
     */
    Node* insert(Node* node, const string& key, int index) {
        if (node == nullptr) {
            return new Node(key, index);
        }

        if (key < node->key) {
            node->left = insert(node->left, key, index);
        } else if (key > node->key) {
            node->right = insert(node->right, key, index);
        } else {
            node->indices.push_back(index);
        }

        return node;
    }

    /**
     * @brief Поиск всех индексов элементов с заданным ключом.
     * @param node Текущий узел дерева.
     * @param key Искомый ключ.
     * @return Вектор индексов всех найденных элементов.
     */
    vector<int> search(Node* node, const string& key) const {
        while (node != nullptr) {
            if (key < node->key) {
                node = node->left;
            } else if (key > node->key) {
                node = node->right;
            } else {
                return node->indices;
            }
        }
        return {};
    }

    /**
     * @brief Рекурсивное освобождение памяти дерева.
     * @param node Корень поддерева, которое нужно удалить.
     */
    void clear(Node* node) {
        if (node == nullptr) return;
        clear(node->left);
        clear(node->right);
        delete node;
    }

public:
    /**
     * @brief Конструктор бинарного дерева поиска по умолчанию.
     */
    BST() = default;

    /**
     * @brief Деструктор бинарного дерева поиска.
     * Освобождает всю динамически выделенную память, занятую узлами дерева.
     */
    ~BST() {
        clear(root);
    }

    /**
     * @brief Построить дерево по массиву объектов Player.
     * Перед построением старое дерево очищается.
     * @param a Массив объектов Player.
     * @param size Размер массива.
     */
    void build(Player a[], long size) {
        clear(root);
        root = nullptr;

        for (int i = 0; i < size; i++) {
            root = insert(root, getKey(a[i]), i);
        }
    }

    /**
     * @brief Выполнить поиск всех вхождений объекта Player по его ключу.
     * @param target Объект Player, ключ которого используется для поиска.
     * @return Вектор индексов всех найденных элементов.
     */
    vector<int> searchAll(const Player& target) const {
        return search(root, getKey(target));
    }

    /**
     * @brief Выполнить поиск всех вхождений по строковому ключу.
     * @param key Искомый строковый ключ.
     * @return Вектор индексов всех найденных элементов.
     */
    vector<int> searchAll(const string& key) const {
        return search(root, key);
    }
};