## Complexity (в худшем и обычном случаях): `O(N)`

Суть: идем по исходному массиву и добавляем в `result` индексы объектов, а потом просто возвращаем массив с индексами.
```C++
vector<int> linearSearch(Player a[], long size, Player b) {
	vector<int> result;
	for (int i = 0; i < size; i++) {
		if (a[i] == b) {
			result.push_back(i);
		}
	}
	return result;
}
```

## 2 типа улучшения:
- Transposition: найденный элемент сдвигается на 1 позицию влево;
- Move-to-front: найденный элемент сразу переносится в начало.