## Complexity: 
- Время `O(NlogN)` 
- Память `O(N)`

```cpp
template<class T> void merge(T a[], long low, long mid, long high) {
	T *b = new T[high+1-low];
	long h,i,j,k;
	h=low;
	i=0;
	j=mid+1;
	
	while((h<=mid)&&(j<=high)) {
		if(a[h]<=a[j]) {
			b[i]=a[h];
			h++;
		}
		else {
			b[i]=a[j];
			j++;
		}
		i++;
	}
	
	if(h>mid) {
		for(k=j;k<=high;k++) {
			b[i]=a[k];
			i++;
		}
	}
	else {
		for(k=h;k<=mid;k++) {
			b[i]=a[k];
			i++;
		}
	} 
	
	for(k=0;k<=high-low;k++) {
		a[k+low]=b[k];
	}
	delete[] b;
}
```
Эта функция слияния: создает временный массив `b` ,  индексы на начало `h`, на  `i-ый`элемент в `b`,  на середину+1 `j`, вспомогательный `k`. Потом запускается `while`, он работает пока `h` и `j` не вышли за свои границы. Внутри сравниваются элементы  из массива `a` , наименьший кладется в `b` и откуда взяли там увеличивается индекс. Так идем до тех пор, пока не дойдем до хотя бы одного конца половины. Потом `if-ом` докладываем в `b` остатки циклом. Затем возвращаем отсортированный участок в `a` и удаляем`b`.

```cpp
template<class T> void merge_sort(T a[], long low, long high) {
	long mid;
	if(low<high) {
		mid=(low+high)/2;
		merge_sort(a, low, mid);
		merge_sort(a, mid+1, high);
		merge(a, low, mid, high);
	}
}
```
Функция для нарезки массива на массивы длины 1. 