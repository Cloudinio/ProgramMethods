/**
 * @file main.cpp
 * @brief Содержит реализацию и сравнение алгоритмов сортировки.
 */
#include <vector>
#include <utility>
#include <chrono>
#include "data.h"

using namespace std::chrono;

/// @brief Быстрая сортировка.
template<class T> void quickSortR(T* a, long N) {
    if (N <= 1) return;
    long i = 0, j = N-1;
    T p = a[ N>>1 ];

    do
    {
        while ( a[i] < p ) i++;
        while ( a[j] > p ) j--;

    if (i <= j)
	{
        swap(a[i], a[j]);
        i++; j--;
    }
    } while ( i<=j );

    if ( j > 0 ) quickSortR(a, j + 1);
    if ( N > i ) quickSortR(a + i, N - i);
}

/// @brief Сортировка слиянием.
template<class T> void merge(T a[], long low, long mid, long high) {
	// Variables declaration. 
	T *b = new T[high+1-low];
	long h,i,j,k;
	h=low;
	i=0;
	j=mid+1;
	// Merges the two array's into b[] until the first one is finish
	while((h<=mid)&&(j<=high))
	{
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
    if(h>mid)
	{
		for(k=j;k<=high;k++) {
			b[i]=a[k];
			i++;
		}
	}
	else
	{
		for(k=h;k<=mid;k++) {
			b[i]=a[k];
			i++;
		}
	}
	// Prints into the original array
	for(k=0;k<=high-low;k++) {
		a[k+low]=b[k];
	}
	delete[] b;
}
template<class T> void merge_sort(T a[], long low, long high) {
	long mid;
	if(low<high) {
		mid=(low+high)/2;
		merge_sort(a, low, mid);
		merge_sort(a, mid+1, high);
		merge(a, low, mid, high);
	}
}

/// @brief Пирамидальная сортировка.
template<class T> void downHeap(T a[], long k, long n)
{
    T new_elem = a[k];
    long child;

    while (k <= n / 2) {
        child = 2 * k;

        if (child < n && a[child] < a[child + 1])
            child++;

        if (new_elem >= a[child]) break;

        a[k] = a[child];
        k = child;
    }
    a[k] = new_elem;
}
template<class T> void heapSort(T a[], long size)
{
    if (size <= 1) return;

    for (long i = size / 2 - 1; i >= 0; --i) {
        downHeap(a, i, size - 1);
        if (i == 0) break;
    }

    for (long i = size - 1; i > 0; --i) {
        std::swap(a[i], a[0]);
        downHeap(a, 0, i - 1);
    }
}

/// @brief Функция для замера времени.
/// Функция, которая высчитывает разницу во времени между стартом и концом работы алгоритма.
/// @param func Алгоритм.
/// @return Возвращает время в микросекундах.
template <typename F>
long long measure_us(F&& func) {
    auto t1 = high_resolution_clock::now();
    func();
    auto t2 = high_resolution_clock::now();
    return duration_cast<microseconds>(t2 - t1).count();
}

/**
 * @brief Главная функция программы.
 * Выполняет чтение наборов данных, запускает алгоритмы сортировки, измеряет время их работы и сохраняет результаты в CSV-файл.
 * @return 0 при успешном завершении программы.
 */
int main() {
    vector<int> sizes = {100, 200, 500, 1000, 2000, 5000, 10000, 20000, 30000, 50000, 70000, 100000};
    /// Открываем файл для записи результатов замеров времени.
    ofstream out("timings.csv");
    out << "N,algorithm,time_us\n";

    /// Для каждого размера данных (из списка sizes) будем выполнять сортировку и измерять время.
    for (int N : sizes) {
        /// Формируем путь к файлу данных для текущего размера.
        string file = "football_datasets/dataset_" + to_string(N) + ".csv";

        /// Считываем данные о футболистах из CSV файла в вектор оригинальных данных.
        vector<Player> original = ReadPlayersFromCSV(file);

        /// Проверка, соответствует ли количество строк в файле ожидаемому размеру.
        if (original.size() != N) {
            cerr << "WARNING: expected " << N << " rows, got " << original.size() << " in " << file << "\n";
        }

        /// Лямбда-функция для выполнения алгоритма сортировки и замера времени.
        /// Эта функция принимает имя алгоритма и сам алгоритм сортировки.
        auto run_algo = [&](const string& name, auto sorter) {
            /// оздаём копию оригинальных данных, чтобы не изменять их в процессе сортировки.
            vector<Player> data = original;

            /// Измеряем время работы сортировки для данных.
            long long duration = measure_us([&]() { sorter(data); });

            ///Записываем результат замера времени в файл.
            /// В файл записываем размер массива, название алгоритма и время работы.
            out << N << "," << name << "," << duration << "\n";
        };

        /// Быстрая сортировка (quickSortR).
        run_algo("quickSortR", [&](vector<Player>& data){
            quickSortR(data.data(), data.size());
        });

        /// Сортировка слиянием (mergeSort).
        run_algo("mergeSort", [&](vector<Player>& data){
            merge_sort(data.data(), 0, data.size() - 1);
        });

        /// Пирамидальная сортировка (heapSort).
        run_algo("heapSort", [&](vector<Player>& data){
            heapSort(data.data(), data.size());
        });

        /// Стандартная сортировка.
        run_algo("sort", [&](vector<Player>& data){
            sort(data.begin(), data.end());
        });
    }

    cout << "Done";

    /**
    * @brief Замер зависимости времени от размера массива.
    * @image html sorting_times.png width=80%
    * Ссылка на гитхаб:
    * https://github.com/Cloudinio/ProgramMethods/tree/main/lab_1
    */

    return 0;
}