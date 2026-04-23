#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <vector>
#include <string>
#include "../lab_1/data.h"

using namespace std;

class HashTable {
private:
    struct Entry {
        string key;
        vector<int> indices;

        Entry(const string& k, int index) : key(k), indices{index} {}
    };

    vector<vector<Entry>> table;
    size_t tableSize;
    long long collisions;

private:
    /// Ключ поиска: ФИО
    string getKey(const Player& p) const {
        return p.GetFullName();
    }

    /// Полиномиальный хэш строки
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

    size_t indexForKey(const string& key) const {
        return hashString(key) % tableSize;
    }

public:
    /// size желательно брать больше числа элементов
    explicit HashTable(size_t size = 100003)
        : table(size), tableSize(size), collisions(0) {}

    void clear() {
        table.assign(tableSize, {});
        collisions = 0;
    }

    void build(Player a[], long size) {
        clear();
        for (int i = 0; i < size; i++) {
            insert(a[i], i);
        }
    }

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

    vector<int> searchAll(const string& key) const {
        size_t pos = indexForKey(key);

        for (const auto& entry : table[pos]) {
            if (entry.key == key) {
                return entry.indices;
            }
        }

        return {};
    }

    vector<int> searchAll(const Player& target) const {
        return searchAll(getKey(target));
    }

    long long getCollisions() const {
        return collisions;
    }
};

#endif // HASHTABLE_H