## Время: `O(NlogN)`
## Память: `O(1)`

```cpp
template<class T> void downHeap(T a[], long k, long n) {
	T new_elem = a[k];
	long child;
	while (2*k+1 <= n) {
		child = 2*k+1;
		if (child < n && a[child] < a[child + 1]) child++;
		if (new_elem >= a[child]) break;

		a[k] = a[child];
		k = child;
	}
	a[k] = new_elem;
}
```
Функция для просеивания значения в пирамиде: берем элемент, который хотим просеять `new_elem` , в цикле идем пока есть левый ребенок, потом проверяем, есть ли правый ребенок и больше ли он левого.  Если элемент, который мы опускаем, уже больше или равен большему ребёнку, значит свойство max-heap не нарушено. Если `new_elem` меньше большего ребёнка, то ребёнка поднимаем вверх. А позиция `k` спускается вниз.

```cpp
template<class T> void heapSort(T a[], long size) {
	if (size <= 1) return;
	
	for (long i = size/2-1; i >= 0; --i) {
		downHeap(a, i, size - 1);
		if (i == 0) break;
	}
	for (long i = size - 1; i > 0; --i) {
		swap(a[i], a[0]);
		downHeap(a, 0, i - 1);
	}
}
```
Функция для построения max-heap и по одному вытаскивать максимум в конец массива: сначала строим кучу. Во втором `for-е` сортируем, перенеся максимум в конец и опять строим пирамиду.