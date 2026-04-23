#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <vector>
#include <string>
#include "../lab_1/data.h"

using namespace std;

/**
 * @brief Класс хеш-таблицы для поиска объектов Player по ключу.
 * В качестве ключа используется первое нечисловое поле объекта Player, а именно ФИО игрока.
 * Хеш-таблица реализована методом цепочек: каждая ячейка таблицы хранит список элементов, попавших в одну корзину.
 * В узлах таблицы хранятся:
 * - ключ;
 * - список индексов элементов исходного массива с данным ключом.
 * Если в массиве несколько объектов с одинаковым ключом, они не создают новые записи, а их индексы добавляются в вектор indices уже существующей записи.
 */
class HashTable {
private:
    /**
     * @brief Структура записи в хеш-таблице.
     * Каждая запись соответствует одному уникальному ключу и хранит все индексы элементов массива с этим ключом.
     */
    struct Entry {
        string key;          ///< Ключ записи.
        vector<int> indices; ///< Индексы элементов массива с данным ключом.

        /**
         * @brief Конструктор записи хеш-таблицы.
         * @param k Ключ записи.
         * @param index Индекс элемента исходного массива.
         */
        Entry(const string& k, int index) : key(k), indices{index} {}
    };

    vector<vector<Entry>> table; ///< Массив корзин хеш-таблицы.
    size_t tableSize;            ///< Размер таблицы.
    long long collisions;        ///< Количество коллизий при вставке.

private:
    /**
     * @brief Получить строковый ключ из объекта Player.
     * @param p Объект Player.
     * @return ФИО игрока.
     */
    string getKey(const Player& p) const {
        return p.GetFullName();
    }

    /**
     * @brief Вычислить полиномиальный хеш строки.
     * Хеш вычисляется по формуле:
     * h(S) = S[0] + S[1] * P + S[2] * P^2 + ...
     * В качестве основания используется число P = 53.
     * @param s Входная строка.
     * @return Значение хеша.
     */
    unsigned long long hashString(const string& s) const {
        const unsigned long long P = 53;
        unsigned long long hash = 0;
        unsigned long long p_pow = 1;

        for (char c : s) {
            hash += (unsigned long long)(unsigned char)c * p_pow;
            p_pow *= P;
        }

        return hash;
    }

    /**
     * @brief Получить индекс корзины по ключу.
     * @param key Строковый ключ.
     * @return Индекс корзины в таблице.
     */
    size_t indexForKey(const string& key) const {
        return hashString(key) % tableSize;
    }

public:
    /**
     * @brief Конструктор хеш-таблицы.
     * Создает таблицу заданного размера.
     * @param size Размер таблицы. Желательно выбирать больше числа элементов.
     */
    explicit HashTable(size_t size = 100003)
        : table(size), tableSize(size), collisions(0) {}

    /**
     * @brief Очистить хеш-таблицу.
     * Удаляет все записи из всех корзин и сбрасывает счетчик коллизий.
     */
    void clear() {
        table.assign(tableSize, {});
        collisions = 0;
    }

    /**
     * @brief Построить хеш-таблицу по массиву объектов Player.
     * Перед построением старая таблица очищается.
     * @param a Массив объектов Player.
     * @param size Размер массива.
     */
    void build(Player a[], long size) {
        clear();
        for (int i = 0; i < size; i++) {
            insert(a[i], i);
        }
    }

    /**
     * @brief Вставить элемент массива в хеш-таблицу.
     * Если ключ уже существует в соответствующей корзине, новый индекс добавляется в список indices существующей записи.
     * Если корзина уже содержит записи с другими ключами, это считается коллизией.
     * @param player Объект Player.
     * @param index Индекс объекта в исходном массиве.
     */
    void insert(const Player& player, int index) {
        string key = getKey(player);
        size_t pos = indexForKey(key);

        /// Если корзина не пустая, это коллизия
        if (!table[pos].empty()) {
            /// Но если ключ уже есть, это не новая коллизия "разных ключей"
            bool sameKeyExists = false;
            for (auto& entry : table[pos]) {
                if (entry.key == key) {
                    entry.indices.push_back(index);
                    sameKeyExists = true;
                    break;
                }
            }

            if (sameKeyExists) return;

            collisions++;
        }

        /// Добавляем новый ключ
        table[pos].emplace_back(key, index);
    }

    /**
     * @brief Найти все вхождения по строковому ключу.
     * @param key Искомый ключ.
     * @return Вектор индексов всех найденных элементов.
     */
    vector<int> searchAll(const string& key) const {
        size_t pos = indexForKey(key);

        for (const auto& entry : table[pos]) {
            if (entry.key == key) {
                return entry.indices;
            }
        }

        return {};
    }

    /**
     * @brief Найти все вхождения по объекту Player.
     * @param target Объект Player, ключ которого используется для поиска.
     * @return Вектор индексов всех найденных элементов.
     */
    vector<int> searchAll(const Player& target) const {
        return searchAll(getKey(target));
    }

    /**
     * @brief Получить количество коллизий, возникших при построении таблицы.
     * @return Число коллизий.
     */
    long long getCollisions() const {
        return collisions;
    }
};

#endif // HASHTABLE_H