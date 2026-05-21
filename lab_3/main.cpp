#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <numeric>
#include <memory>
#include <random>
#include <string>
#include <vector>

using namespace std;
using namespace chrono;

static constexpr double TWO32 = 4294967296.0;
static constexpr double EPS = 1e-12;

// Базовый интерфейс генератора 
struct Generator {
    virtual ~Generator() = default;
    virtual uint32_t nextU32() = 0;
    virtual string name() const = 0;
};

// Модифицированный метод серединных произведений + xorshift-перемешивание
class ModifiedMiddleProductXor : public Generator {
private:
    uint32_t x, y;      // два предыдущих значения генератора
    uint32_t salt;      // соль для уменьшения риска вырождения

public:
    ModifiedMiddleProductXor(uint32_t seed1, uint32_t seed2)
        : x(seed1 ? seed1 : 123456789u), y(seed2 ? seed2 : 362436069u), salt(0x9E3779B9u) {}

    uint32_t nextU32() override {
        uint64_t prod = uint64_t(x) * uint64_t(y);

        uint32_t middle = uint32_t((prod >> 16) & 0xFFFFFFFFu);

        // Перемешиваем биты через xorshift
        middle ^= middle << 13;
        middle ^= middle >> 17;
        middle ^= middle << 5;

        middle += salt;

        x = y ^ (salt << 1);
        y = middle ? middle : salt;

        // 0x9E3779B9 используется для битового смешивания
        salt += 0x9E3779B9u;

        return y;
    }

    string name() const override {
        return "ModifiedMiddleProductXor";
    }
};


// Модифицированный линейный конгруэнтный генератор c перемешиванием
class ModifiedLCGPermuted : public Generator {
    uint64_t state;

public:
    ModifiedLCGPermuted(uint64_t seed)
        : state(seed ? seed : 1) {}

    uint32_t nextU32() override {
        // state = a * state + c mod 2^64
        state = state * 6364136223846793005ULL + 1442695040888963407ULL;

        uint64_t z = state;

        // Отдаём не само состояние LCG, а дополнительно перемешанное значение
        z ^= z >> 30;
        z *= 0xbf58476d1ce4e5b9ULL;

        z ^= z >> 27;
        z *= 0x94d049bb133111ebULL;

        z ^= z >> 31;

        // Возвращаем старшие 32 бита перемешанного 64-битного значения
        return uint32_t(z >> 32);
    }

    string name() const override {
        return "ModifiedLCGPermuted";
    }
};

// Генератор с XOR-перемешиванием с добавлением соли
class XorShift32Salt : public Generator {
private:
    uint32_t x;
    uint32_t w;

public:
    XorShift32Salt(uint32_t seed)
        : x(seed ? seed : 2463534242u),
          w(0x9E3779B9u) {}

    uint32_t nextU32() override {
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;

        w += 0x9E3779B9u;

        return x + w;
    }

    string name() const override {
        return "XorShift32Salt";
    }
};

// Стандартный генератор Mersenne Twister
class StdMT19937 final : public Generator {
    std::mt19937 mt;

public:
    explicit StdMT19937(uint32_t seed)
        : mt(seed) {}

    uint32_t nextU32() override {
        return mt();
    }

    string name() const override {
        return "std_mt19937";
    }
};

// Расчёт основных статистик
struct BasicStats {
    double mean;
    double stddev;
    double cv;
};

// Считает среднее, стандартное отклонение и коэффициент вариации
BasicStats basicStats(const vector<double>& v) {
    double mean = accumulate(v.begin(), v.end(), 0.0) / v.size();

    double var = 0.0;
    for (double x : v) {
        var += (x - mean) * (x - mean);
    }

    var /= (v.size() - 1);

    double sd = sqrt(var);

    return {mean, sd, sd / max(abs(mean), EPS)};
}

// Приближённая обратная функция нормального распределения
static double invNorm(double p) {
    static const double a[] = {
        -3.969683028665376e+01,
         2.209460984245205e+02,
        -2.759285104469687e+02,
         1.383577518672690e+02,
        -3.066479806614716e+01,
         2.506628277459239e+00
    };

    static const double b[] = {
        -5.447609879822406e+01,
         1.615858368580409e+02,
        -1.556989798598866e+02,
         6.680131188771972e+01,
        -1.328068155288572e+01
    };

    static const double c[] = {
        -7.784894002430293e-03,
        -3.223964580411365e-01,
        -2.400758277161838e+00,
        -2.549732539343734e+00,
         4.374664141464968e+00,
         2.938163982698783e+00
    };

    static const double d[] = {
        7.784695709041462e-03,
        3.224671290700398e-01,
        2.445134137142996e+00,
        3.754408661907416e+00
    };

    double q, r;

    // Левая хвостовая область
    if (p < 0.02425) {
        q = sqrt(-2 * log(p));
        return (((((c[0]*q+c[1])*q+c[2])*q+c[3])*q+c[4])*q+c[5]) /
               ((((d[0]*q+d[1])*q+d[2])*q+d[3])*q+1);
    }

    // Правая хвостовая область
    if (p > 1 - 0.02425) {
        q = sqrt(-2 * log(1-p));
        return -(((((c[0]*q+c[1])*q+c[2])*q+c[3])*q+c[4])*q+c[5]) /
                ((((d[0]*q+d[1])*q+d[2])*q+d[3])*q+1);
    }

    // Центральная область
    q = p - 0.5;
    r = q * q;

    return (((((a[0]*r+a[1])*r+a[2])*r+a[3])*r+a[4])*r+a[5]) * q /
           (((((b[0]*r+b[1])*r+b[2])*r+b[3])*r+b[4])*r+1);
}

// Приближённое критическое значение хи-квадрат
static double chiSquareCritical(int df, double p) {
    double z = invNorm(p);

    double x = 1.0 - 2.0 / (9.0 * df) + z * sqrt(2.0 / (9.0 * df));

    return df * x * x * x;
}

// Результат проверки хи-квадрат на равномерность распределения
struct ChiResult {
    int bins;
    double chi2;
    double low;
    double high;
    bool pass;
};

// Проверка равномерности распределения по критерию хи-квадрат
ChiResult chiUniform(const vector<double>& v) {
    int N = (int)v.size();

    // Количество интервалов выбирается по правилу Стерджеса: k = 1 + log2(N)
    int k = max(5, (int)ceil(1.0 + log2((double)N)));

    vector<int> cnt(k, 0);

    for (double x : v) {
        int id = min(k - 1, max(0, int(x * k)));
        cnt[id]++;
    }

    // Ожидаемое количество чисел в каждом интервале
    double expected = double(N) / k;

    // Расчёт экспериментального хи-квадрат
    double chi2 = 0.0;
    for (int c : cnt) {
        chi2 += (c - expected) * (c - expected) / expected;
    }

    // Число степеней свободы
    int df = k - 1;

    // Критические границы для уровня 2.5% и 97.5%
    double low = chiSquareCritical(df, 0.025);
    double high = chiSquareCritical(df, 0.975);

    // Генератор проходит тест, если хи-квадрат попал в допустимый интервал
    return {k, chi2, low, high, chi2 >= low && chi2 <= high};
}

// NIST / Diehard-подобные тесты

// Общий формат результата статистического теста
struct TestResult {
    string test;
    double statistic;
    double low;
    double high;
    bool pass;
};

// Преобразует массив 32-битных чисел в последовательность битов
vector<uint8_t> bitsFromNumbers(const vector<uint32_t>& nums) {
    vector<uint8_t> bits;
    bits.reserve(nums.size() * 32);

    for (uint32_t x : nums) {
        for (int b = 31; b >= 0; --b) {
            bits.push_back((x >> b) & 1u);
        }
    }

    return bits;
}


// NIST frequency test
// Проверяет, примерно ли одинаково часто встречаются 0 и 1
TestResult bitFrequency(const vector<uint8_t>& bits) {
    // Количество единиц
    int64_t ones = accumulate(bits.begin(), bits.end(), int64_t(0));

    double n = bits.size();

    // z-статистика: насколько количество единиц отклоняется от n / 2
    double z = (ones - n / 2.0) / sqrt(n / 4.0);

    return {"NIST_bit_frequency_abs_z", fabs(z), 0.0, 1.96, fabs(z) <= 1.96};
}

// NIST runs test
// Проверяет количество серий одинаковых битов
TestResult bitRuns(const vector<uint8_t>& bits) {
    int runs = 1;

    // Считаем количество переходов 0 -> 1 и 1 -> 0
    for (size_t i = 1; i < bits.size(); ++i) {
        if (bits[i] != bits[i - 1]) {
            runs++;
        }
    }

    double n = bits.size();

    // Ожидаемое число серий для случайной битовой последовательности
    double expected = (2 * n - 1) / 2.0;

    // Дисперсия числа серий
    double var = (2 * n - 1) / 4.0;

    double z = (runs - expected) / sqrt(var);

    return {"NIST_runs_abs_z", fabs(z), 0.0, 1.96, fabs(z) <= 1.96};
}

// Serial 2-bit test
// Проверяет равномерность появления пар битов
TestResult serial2(const vector<uint8_t>& bits) {
    array<int, 4> cnt{0, 0, 0, 0};

    // Разбиваем биты на непересекающиеся пары
    for (size_t i = 0; i + 1 < bits.size(); i += 2) {
        cnt[bits[i] * 2 + bits[i + 1]]++;
    }

    double n = bits.size() / 2.0;
    double exp = n / 4.0;
    double chi2 = 0;

    // хи-квадрат по четырём категориям
    for (int c : cnt) {
        chi2 += (c - exp) * (c - exp) / exp;
    }

    double low = chiSquareCritical(3, 0.025);
    double high = chiSquareCritical(3, 0.975);

    return {"NIST_serial_2bit_chi2", chi2, low, high, chi2 >= low && chi2 <= high};
}

// Diehard monkey / poker test для 4-битных слов
TestResult poker4(const vector<uint8_t>& bits) {
    array<int, 16> cnt{};

    size_t blocks = bits.size() / 4;

    // Разбиваем поток битов на блоки по 4 бита
    for (size_t i = 0; i < blocks; i++) {
        int v = 0;

        for (int j = 0; j < 4; j++) {
            v = (v << 1) | bits[4 * i + j];
        }

        cnt[v]++;
    }

    double exp = blocks / 16.0;
    double chi2 = 0;

    for (int c : cnt) {
        chi2 += (c - exp) * (c - exp) / exp;
    }

    double low = chiSquareCritical(15, 0.025);
    double high = chiSquareCritical(15, 0.975);

    return {"Diehard_monkey_poker4_chi2", chi2, low, high, chi2 >= low && chi2 <= high};
}

// Diehard overlapping permutations test
TestResult overlappingPermutations(const vector<double>& v) {
    array<int, 120> cnt{};

    // Факториалы нужны для перевода перестановки в номер
    array<int, 5> fact{1, 1, 2, 6, 24};

    int windows = 0;

    // Окно из 5 соседних чисел
    for (size_t i = 0; i + 4 < v.size(); ++i) {
        array<pair<double, int>, 5> a{};

        // Запоминаем значение и его исходный номер
        for (int j = 0; j < 5; j++) {
            a[j] = {v[i + j], j};
        }

        // Сортируем пять чисел по величине
        sort(a.begin(), a.end());

        array<int, 5> perm{};

        // Получаем порядок исходных индексов после сортировки
        for (int rank = 0; rank < 5; ++rank) {
            perm[rank] = a[rank].second;
        }

        // Код Лемера: переводит перестановку в число от 0 до 119
        int idx = 0;

        for (int j = 0; j < 5; j++) {
            int smaller = 0;

            for (int k = j + 1; k < 5; k++) {
                if (perm[k] < perm[j]) {
                    smaller++;
                }
            }

            idx += smaller * fact[4 - j];
        }

        cnt[idx]++;
        windows++;
    }

    double exp = windows / 120.0;
    double chi2 = 0;

    for (int c : cnt) {
        chi2 += (c - exp) * (c - exp) / exp;
    }

    double low = chiSquareCritical(119, 0.025);
    double high = chiSquareCritical(119, 0.975);

    return {
        "Diehard_overlapping_permutations_chi2",
        chi2,
        low,
        high,
        chi2 >= low && chi2 <= high
    };
}

// Diehard birthday spacings test
TestResult birthdaySpacings(const vector<double>& v) {
    // Размер дискретного пространства
    const int m = 1 << 20;

    // Берём не больше 512 точек
    int n = min<int>(v.size(), 512);

    vector<int> xs(n);

    // Переводим числа из [0; 1) в целые точки от 0 до m - 1
    for (int i = 0; i < n; i++) {
        xs[i] = min(m - 1, int(v[i] * m));
    }

    // Сортируем точки
    sort(xs.begin(), xs.end());

    vector<int> spacings;

    // Считаем расстояния между соседними точками
    for (int i = 1; i < n; i++) {
        spacings.push_back(xs[i] - xs[i - 1]);
    }

    // Сортируем расстояния, чтобы найти совпадения
    sort(spacings.begin(), spacings.end());

    int collisions = 0;

    // Считаем количество совпавших расстояний
    for (size_t i = 1; i < spacings.size(); ++i) {
        if (spacings[i] == spacings[i - 1]) {
            collisions++;
        }
    }

    // Приближённое ожидаемое число совпадений
    double lambda = double(n * n * n) / (4.0 * m);

    // z-оценка отклонения от ожидаемого значения
    double z = (collisions - lambda) / sqrt(max(lambda, 1.0));

    return {"Diehard_birthday_spacings_abs_z", fabs(z), 0.0, 3.0, fabs(z) <= 3.0};
}

// Запускает все NIST/Diehard-подобные тесты для одной выборки
vector<TestResult> allTests(const vector<uint32_t>& nums, const vector<double>& vals) {
    vector<uint8_t>  bits = bitsFromNumbers(nums);

    vector<TestResult> r;

    r.push_back(bitFrequency(bits));
    r.push_back(bitRuns(bits));
    r.push_back(serial2(bits));
    r.push_back(poker4(bits));
    r.push_back(overlappingPermutations(vals));
    r.push_back(birthdaySpacings(vals));

    return r;
}

// Создаёт нужный генератор по номеру метода
unique_ptr<Generator> makeGen(int method, uint64_t seed) {
    switch (method) {
        case 0:
            return make_unique<ModifiedMiddleProductXor>(
                uint32_t(seed),
                uint32_t(seed * 1664525u + 1013904223u)
            );

        case 1:
            return make_unique<ModifiedLCGPermuted>(seed);

        case 2:
            return make_unique<XorShift32Salt>(uint32_t(seed));

        default:
            return make_unique<StdMT19937>(uint32_t(seed));
    }
}

int main() {
    // По условию нужно не менее 20 выборок каждым методом
    const int samplesPerMethod = 20;

    // В каждой выборке не менее 1000 элементов.
    const int sampleSize = 1000;

    // Файл с общей сводкой по выборкам
    ofstream summary("results_summary.csv");
    summary << "method,sample,N,mean,stddev,cv,chi_bins,chi2,chi_low,chi_high,chi_pass,tests_passed,total_tests\n";

    // Файл с подробными результатами NIST/Diehard-подобных тестов
    ofstream testsCsv("tests_detail.csv");
    testsCsv << "method,sample,test,statistic,low,high,pass\n";

    // Основная часть лабораторной:
    // для каждого из трёх собственных генераторов создаём 20 выборок
    for (int method = 0; method < 3; ++method) {
        for (int s = 0; s < samplesPerMethod; ++s) {
            // Для каждой выборки используем свой seed
            unique_ptr<Generator> gen = makeGen(method, 1234567ULL + 1000003ULL * s + 911ULL * method);

            vector<uint32_t> nums;
            nums.reserve(sampleSize);

            vector<double> vals;
            vals.reserve(sampleSize);

            // Генерируем выборку
            for (int i = 0; i < sampleSize; i++) {
                uint32_t u = gen->nextU32();

                nums.push_back(u);

                // Переводим uint32_t в число из диапазона [0; 1)
                vals.push_back((u + 0.5) / TWO32);
            }

            // Считаем основные статистики
            BasicStats st = basicStats(vals);

            // Проверяем равномерность по хи-квадрат
            ChiResult ch = chiUniform(vals);

            // Запускаем NIST/Diehard-подобные тесты
            vector<TestResult> tests = allTests(nums, vals);

            int passed = 0;

            for (size_t i = 0; i < tests.size(); i++) {
                if (tests[i].pass) {
                    passed++;
                }
            }

            // Записываем общую статистику в CSV
            summary << gen->name() << ','
                    << s + 1 << ','
                    << sampleSize << ','
                    << st.mean << ','
                    << st.stddev << ','
                    << st.cv << ','
                    << ch.bins << ','
                    << ch.chi2 << ','
                    << ch.low << ','
                    << ch.high << ','
                    << (ch.pass ? 1 : 0) << ','
                    << passed << ','
                    << tests.size()
                    << "\n";

            // Записываем подробные результаты каждого теста в CSV
            for (size_t i = 0; i < tests.size(); i++) {
                testsCsv << gen->name() << ','
                        << s + 1 << ','
                        << tests[i].test << ','
                        << tests[i].statistic << ','
                        << tests[i].low << ','
                        << tests[i].high << ','
                        << (tests[i].pass ? 1 : 0)
                        << "\n";
            }
        }
    }

    // Замер скорости генерации
    vector<int> sizes = {1000, 5000, 10000, 50000, 100000, 250000, 500000, 1000000};

    ofstream speed("speed.csv");
    speed << "method,N,milliseconds\n";

    // Здесь сравниваются 3 собственных метода и стандартный std::mt19937
    for (int method = 0; method < 4; ++method) {
        for (int N : sizes) {
            unique_ptr<Generator> gen = makeGen(method, 987654321ULL + method);

            high_resolution_clock::time_point t1 = high_resolution_clock::now();

            // Генерируем N чисел
            for (int i = 0; i < N; i++) {
                gen->nextU32();
            }

            high_resolution_clock::time_point t2 = high_resolution_clock::now();
            double ms = duration<double, milli>(t2 - t1).count();

            speed << gen->name() << ',' << N << ',' << ms << "\n";
        }
    }

    cout << "Done";
    return 0;
}