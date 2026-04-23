#include <vector>
#include <string>
#include "../lab_1/data.h"

using namespace std;

class RBTree {
private:
    enum Color { RED, BLACK };

    struct Node {
        string key;
        vector<int> indices;
        Color color;
        Node* left;
        Node* right;
        Node* parent;

        Node(const string& k, int index)
            : key(k), indices{index}, color(RED),
              left(nullptr), right(nullptr), parent(nullptr) {}
    };

    Node* root = nullptr;

private:
    /// Замени p.fio на настоящее поле с ФИО
    string getKey(const Player& p) const {
        return p.GetFullName();
    }

    Color getColor(Node* node) const {
        return (node == nullptr ? BLACK : node->color);
    }

    void rotateLeft(Node* x) {
        Node* y = x->right;
        if (y == nullptr) return;

        x->right = y->left;
        if (y->left != nullptr) {
            y->left->parent = x;
        }

        y->parent = x->parent;

        if (x->parent == nullptr) {
            root = y;
        } else if (x == x->parent->left) {
            x->parent->left = y;
        } else {
            x->parent->right = y;
        }

        y->left = x;
        x->parent = y;
    }

    void rotateRight(Node* x) {
        Node* y = x->left;
        if (y == nullptr) return;

        x->left = y->right;
        if (y->right != nullptr) {
            y->right->parent = x;
        }

        y->parent = x->parent;

        if (x->parent == nullptr) {
            root = y;
        } else if (x == x->parent->right) {
            x->parent->right = y;
        } else {
            x->parent->left = y;
        }

        y->right = x;
        x->parent = y;
    }

    void fixInsert(Node* z) {
        while (z != root && getColor(z->parent) == RED) {
            Node* parent = z->parent;
            Node* grandparent = parent->parent;

            if (grandparent == nullptr) break;

            if (parent == grandparent->left) {
                Node* uncle = grandparent->right;

                /// Случай 1: дядя красный
                if (getColor(uncle) == RED) {
                    parent->color = BLACK;
                    uncle->color = BLACK;
                    grandparent->color = RED;
                    z = grandparent;
                } else {
                    /// Случай 2: z - правый ребенок
                    if (z == parent->right) {
                        z = parent;
                        rotateLeft(z);
                        parent = z->parent;
                        grandparent = parent ? parent->parent : nullptr;
                    }

                    /// Случай 3: z - левый ребенок
                    if (parent != nullptr) parent->color = BLACK;
                    if (grandparent != nullptr) {
                        grandparent->color = RED;
                        rotateRight(grandparent);
                    }
                }
            } else {
                Node* uncle = grandparent->left;

                /// Зеркальные случаи
                if (getColor(uncle) == RED) {
                    parent->color = BLACK;
                    uncle->color = BLACK;
                    grandparent->color = RED;
                    z = grandparent;
                } else {
                    if (z == parent->left) {
                        z = parent;
                        rotateRight(z);
                        parent = z->parent;
                        grandparent = parent ? parent->parent : nullptr;
                    }

                    if (parent != nullptr) parent->color = BLACK;
                    if (grandparent != nullptr) {
                        grandparent->color = RED;
                        rotateLeft(grandparent);
                    }
                }
            }
        }

        if (root != nullptr) {
            root->color = BLACK;
        }
    }

    void clear(Node* node) {
        if (node == nullptr) return;
        clear(node->left);
        clear(node->right);
        delete node;
    }

public:
    RBTree() = default;

    ~RBTree() {
        clear(root);
    }

    void build(Player a[], long size) {
        clear(root);
        root = nullptr;

        for (int i = 0; i < size; i++) {
            insert(a[i], i);
        }
    }

    void insert(const Player& player, int index) {
        string key = getKey(player);

        Node* y = nullptr;
        Node* x = root;

        while (x != nullptr) {
            y = x;
            if (key < x->key) {
                x = x->left;
            } else if (key > x->key) {
                x = x->right;
            } else {
                /// Такой ключ уже есть -> просто добавляем индекс
                x->indices.push_back(index);
                return;
            }
        }

        Node* z = new Node(key, index);
        z->parent = y;

        if (y == nullptr) {
            root = z;
        } else if (key < y->key) {
            y->left = z;
        } else {
            y->right = z;
        }

        fixInsert(z);
    }

    vector<int> searchAll(const string& key) const {
        Node* cur = root;

        while (cur != nullptr) {
            if (key < cur->key) {
                cur = cur->left;
            } else if (key > cur->key) {
                cur = cur->right;
            } else {
                return cur->indices;
            }
        }

        return {};
    }

    vector<int> searchAll(const Player& target) const {
        return searchAll(getKey(target));
    }
};