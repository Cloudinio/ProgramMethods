#include <vector>
#include <chrono>
#include "BST.h"
#include "RBTree.h"
#include "../lab_1/data.h"
#include "HashTable.h"
#include <map>

using namespace std::chrono;

/// @brief Функция для замера времени.
/// Функция, которая высчитывает разницу во времени между стартом и концом работы алгоритма поиска.
/// @param func Алгоритм поиска.
/// @return Возвращает время в микросекундах.
template <typename F>
long long measure_us(F&& func) {
    auto t1 = high_resolution_clock::now();
    func();
    auto t2 = high_resolution_clock::now();
    return duration_cast<microseconds>(t2 - t1).count();
}

/// @brief Линейный поиск.
/// В структуре Player ключом является поле ФИО.
/// Оператор == перегружен так, что сравнение объектов Player выполняется только по полю ФИО.
vector<int> linearSearch(Player a[], long size, Player b) {
    vector<int> result;

    for (int i = 0; i < size; i++) {
        if (a[i] == b) {
            result.push_back(i);
        }
    }
    return result;
}

/// @brief Multimap.
vector<Player> multimapSearch(multimap<string, Player>& mp, const string& key) {
    vector<Player> result;
    auto range = mp.equal_range(key);
    for (auto it = range.first; it != range.second; ++it) {
        result.push_back(it->second);
    }
    return result;
}

int main() {
    vector<int> sizes = {100, 200, 500, 1000, 2000, 5000, 10000, 20000, 30000, 50000, 70000, 100000};
    /// Открываем файл для записи результатов замеров времени.
    ofstream out("timings.csv");
    out << "N,algorithm,time_us\n";

    /// Для каждого размера данных (из списка sizes) будем выполнять поиск и измерять время.
    for (int N : sizes) {
        /// Формируем путь к файлу данных для текущего размера.
        string file = "../lab_1/football_datasets/dataset_" + to_string(N) + ".csv";

        /// Считываем данные о футболистах из CSV файла в вектор оригинальных данных.
        vector<Player> original = ReadPlayersFromCSV(file);

        /// Проверка, соответствует ли количество строк в файле ожидаемому размеру.
        if (original.size() != N) {
            cerr << "WARNING: expected " << N << " rows, got " << original.size() << " in " << file << "\n";
        }

        /// ============== Линейный поиск ==============
        Player target = original[original.size() / 2];
        long long t_linear = measure_us([&]() { 
            linearSearch(original.data(), N, target);
        });
        out << N << ",linearSearch," << t_linear << "\n";

        /// ===== BST =====
        BST tree;
        tree.build(original.data(), N);
        long long t_bst = measure_us([&]() {
            vector<int> found = tree.searchAll(target);
        });
        out << N << ",BST," << t_bst << "\n";

        /// ===== RBTree =====
        RBTree rbtree;
        rbtree.build(original.data(), N);
        long long t_rbt = measure_us([&]() {
            vector<int> found = rbtree.searchAll(target);
        });
        out << N << ",RBTree," << t_rbt << "\n";

        /// ===== HashTable =====
        HashTable ht(2 * N + 1);
        ht.build(original.data(), N);
        long long t_hash = measure_us([&]() {
            vector<int> found = ht.searchAll(target);
        });
        out << N << ",HashTable," << t_hash << "\n";

        /// ===== multimap =====
        multimap<string, Player> mp;
        for (int i = 0; i < N; i++) {
            mp.insert({original[i].GetFullName(), original[i]});
        }
        long long t_multimap = measure_us([&]() {
            vector<Player> found = multimapSearch(mp, target.GetFullName());
        });
        out << N << ",multimap," << t_multimap << "\n";
    }

    cout << "Done";
    return 0;
}