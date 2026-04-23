#include <vector>
#include <string>
#include "../lab_1/data.h"

using namespace std;

class BST {
private:
    struct Node {
        string key;
        vector<int> indices;
        Node* left;
        Node* right;

        Node(const string& k, int index)
            : key(k), indices{index}, left(nullptr), right(nullptr) {}
    };

    Node* root = nullptr;

private:
    string getKey(const Player& p) const {
        return p.GetFullName();
    }

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

    void clear(Node* node) {
        if (node == nullptr) return;
        clear(node->left);
        clear(node->right);
        delete node;
    }

public:
    BST() = default;

    ~BST() {
        clear(root);
    }

    /// Построить дерево по массиву
    void build(Player a[], long size) {
        clear(root);
        root = nullptr;

        for (int i = 0; i < size; i++) {
            root = insert(root, getKey(a[i]), i);
        }
    }

    /// Поиск по объекту Player
    vector<int> searchAll(const Player& target) const {
        return search(root, getKey(target));
    }

    /// Поиск по строковому ключу
    vector<int> searchAll(const string& key) const {
        return search(root, key);
    }
};