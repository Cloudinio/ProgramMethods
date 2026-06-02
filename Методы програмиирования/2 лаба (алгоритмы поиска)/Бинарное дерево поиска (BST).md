Бинарное (двоичное) дерево поиска – это бинарное дерево, для которого выполняются следующие дополнительные условия (свойства дерева поиска):
- оба поддерева – левое и правое, являются двоичными деревьями поиска;
- у всех узлов левого поддерева произвольного узла X значения ключей данных меньше, чем значение ключа данных самого узла X;
- у всех узлов правого поддерева произвольного узла X значения ключей данных не меньше, чем значение ключа данных узла X.
## Complexity: 
- В лучшем и обычном случаях `O(logN)`
- В худшем случае `O(N)`

## Узел дерева
```C++
struct Node {
	string key;
	vector<int> indices;
	Node* left;
	Node* right;
	Node(const string& k, int index): key(k), indices{index}, left(nullptr), right(nullptr) {}
};
```
Так выглядит узел дерева:
- ключ-строка (ФИО игрока)
- вектор индексов (список индексов элементов исходного массива `Player`, у которых такой же ключ)
- указатели на левое и правое поддерево
- конструктор (кладет сразу в массив индекс и ключ)

## Private часть дерева
Кроме структуры храним указатель на корень, чтобы понимать какие узлы относятся к какому дереву.

Функция, чтобы получить ФИО игрока-объекта
```C++
string getKey(const Player& p) const {
	return p.GetFullName();
}
```

Рекурсивная вставка узла в дерево.
```C++
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
```
Если узел с таким ключом уже существует, новый узел не создается, а индекс добавляется в список indices существующего узла.

Поиск всех индексов элементов с заданным ключом.
```C++
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
```

Рекурсивное освобождение памяти дерева.
```C++
void clear(Node* node) {
	if (node == nullptr) return;
	clear(node->left);
	clear(node->right);
	delete node;
}
```

Конструктор бинарного дерева поиска по умолчанию.
```C++
BST() = default;
```

Деструктор бинарного дерева поиска.
```C++
~BST() {
	clear(root);
```

Построить дерево по массиву объектов Player.
```cpp
void build(Player a[], long size) {
	clear(root);
	root = nullptr;
	for (int i = 0; i < size; i++) {
		root = insert(root, getKey(a[i]), i);
	}
}
```

Выполнить поиск всех вхождений объекта Player по его ключу.
```cpp
vector<int> searchAll(const Player& target) const {
	return search(root, getKey(target));
}
```