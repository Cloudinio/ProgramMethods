#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>

using namespace std;
using namespace chrono;

const double TWO32 = 4294967296.0;
const double EPS = 1e-12;

// Модифицированный метод серединных произведений + xorshift-перемешивание
class ModifiedMiddleProductXor {
private:
    uint32_t x;
    uint32_t y;
    uint32_t salt;

public:
    ModifiedMiddleProductXor(uint32_t seed1, uint32_t seed2) {
        if (seed1 != 0) {
            x = seed1;
        } else {
            x = 123456789u;
        }

        if (seed2 != 0) {
            y = seed2;
        } else {
            y = 362436069u;
        }

        salt = 0x9E3779B9u;
    }

    uint32_t nextU32() {
        uint64_t prod = uint64_t(x) * uint64_t(y);

        uint32_t middle = uint32_t((prod >> 16) & 0xFFFFFFFFu);

        middle ^= middle << 13;
        middle ^= middle >> 17;
        middle ^= middle << 5;

        middle += salt;

        x = y ^ (salt << 1);

        if (middle != 0) {
            y = middle;
        } else {
            y = salt;
        }

        salt += 0x9E3779B9u;

        return y;
    }

    string name() {
        return "ModifiedMiddleProductXor";
    }
};

// Модифицированный линейный конгруэнтный генератор c перемешиванием
class ModifiedLCGPermuted {
private:
    uint64_t state;

public:
    ModifiedLCGPermuted(uint64_t seed) {
        if (seed != 0) {
            state = seed;
        } else {
            state = 1;
        }
    }

    uint32_t nextU32() {
        state = state * 6364136223846793005ULL + 1442695040888963407ULL;

        uint64_t z = state;

        z ^= z >> 30;
        z *= 0xbf58476d1ce4e5b9ULL;

        z ^= z >> 27;
        z *= 0x94d049bb133111ebULL;

        z ^= z >> 31;

        return uint32_t(z >> 32);
    }

    string name() {
        return "ModifiedLCGPermuted";
    }
};

// Генератор с XOR-перемешиванием с добавлением соли
class XorShift32Salt {
private:
    uint32_t x;
    uint32_t w;

public:
    XorShift32Salt(uint32_t seed) {
        if (seed != 0) {
            x = seed;
        } else {
            x = 2463534242u;
        }

        w = 0x9E3779B9u;
    }

    uint32_t nextU32() {
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;

        w += 0x9E3779B9u;

        return x + w;
    }

    string name() {
        return "XorShift32Salt";
    }
};

// Стандартный генератор Mersenne Twister
class StdMT19937 {
private:
    mt19937 mt;

public:
    StdMT19937(uint32_t seed) {
        mt.seed(seed);
    }

    uint32_t nextU32() {
        return mt();
    }

    string name() {
        return "std_mt19937";
    }
};


struct BasicStats {
    double mean;
    double stddev;
    double cv;
};

struct ChiResult {
    int bins;
    double chi2;
    double low;
    double high;
    bool pass;
};

struct TestResult {
    string test;
    double statistic;
    double low;
    double high;
    bool pass;
};


// Считает среднее, стандартное отклонение и коэффициент вариации
BasicStats basicStats(const vector<double>& v) {
    double mean = accumulate(v.begin(), v.end(), 0.0) / v.size();

    double var = 0.0;

    for (int i = 0; i < v.size(); i++) {
        var += (v[i] - mean) * (v[i] - mean);
    }

    var /= (v.size() - 1);

    double sd = sqrt(var);

    BasicStats result;
    result.mean = mean;
    result.stddev = sd;
    result.cv = sd / max(abs(mean), EPS);

    return result;
}

// Приближённая обратная функция нормального распределения (аппроксимация Peter J. Acklam)
double invNorm(double p) {
    const double a[] = {
        -3.969683028665376e+01,
         2.209460984245205e+02,
        -2.759285104469687e+02,
         1.383577518672690e+02,
        -3.066479806614716e+01,
         2.506628277459239e+00
    };

    const double b[] = {
        -5.447609879822406e+01,
         1.615858368580409e+02,
        -1.556989798598866e+02,
         6.680131188771972e+01,
        -1.328068155288572e+01
    };

    const double c[] = {
        -7.784894002430293e-03,
        -3.223964580411365e-01,
        -2.400758277161838e+00,
        -2.549732539343734e+00,
         4.374664141464968e+00,
         2.938163982698783e+00
    };

    const double d[] = {
        7.784695709041462e-03,
        3.224671290700398e-01,
        2.445134137142996e+00,
        3.754408661907416e+00
    };

    double q;
    double r;

    if (p < 0.02425) {
        q = sqrt(-2 * log(p));

        return (((((c[0] * q + c[1]) * q + c[2]) * q + c[3]) * q + c[4]) * q + c[5]) /
               ((((d[0] * q + d[1]) * q + d[2]) * q + d[3]) * q + 1);
    }

    if (p > 1 - 0.02425) {
        q = sqrt(-2 * log(1 - p));

        return -(((((c[0] * q + c[1]) * q + c[2]) * q + c[3]) * q + c[4]) * q + c[5]) /
                 ((((d[0] * q + d[1]) * q + d[2]) * q + d[3]) * q + 1);
    }

    q = p - 0.5;
    r = q * q;

    return (((((a[0] * r + a[1]) * r + a[2]) * r + a[3]) * r + a[4]) * r + a[5]) * q /
           (((((b[0] * r + b[1]) * r + b[2]) * r + b[3]) * r + b[4]) * r + 1);
}

// Приближённое критическое значение хи-квадрат
double chiSquareCritical(int df, double p) {
    double z = invNorm(p);

    double x = 1.0 - 2.0 / (9.0 * df) + z * sqrt(2.0 / (9.0 * df));

    return df * x * x * x;
}

// Проверка равномерности распределения по критерию хи-квадрат
ChiResult chiUniform(const vector<double>& v) {
    int N = v.size();

    int k = max(5, (int)ceil(1.0 + log2((double)N)));

    vector<int> cnt(k, 0);

    for (int i = 0; i < (int)v.size(); i++) {
        int id = int(v[i] * k);

        if (id < 0) {
            id = 0;
        }

        if (id > k - 1) {
            id = k - 1;
        }

        cnt[id]++;
    }

    double expected = double(N) / k;

    double chi2 = 0.0;

    for (int i = 0; i < k; i++) {
        chi2 += (cnt[i] - expected) * (cnt[i] - expected) / expected;
    }

    int df = k - 1;

    double low = chiSquareCritical(df, 0.025);
    double high = chiSquareCritical(df, 0.975);

    ChiResult result;
    result.bins = k;
    result.chi2 = chi2;
    result.low = low;
    result.high = high;
    result.pass = chi2 >= low && chi2 <= high;

    return result;
}

// Преобразует массив 32-битных чисел в последовательность битов
vector<uint8_t> bitsFromNumbers(const vector<uint32_t>& nums) {
    vector<uint8_t> bits;

    bits.reserve(nums.size() * 32);

    for (int i = 0; i < (int)nums.size(); i++) {
        uint32_t x = nums[i];

        for (int b = 31; b >= 0; b--) {
            bits.push_back((x >> b) & 1u);
        }
    }

    return bits;
}


// NIST frequency test
TestResult bitFrequency(const vector<uint8_t>& bits) {
    int64_t ones = 0;

    for (int i = 0; i < (int)bits.size(); i++) {
        ones += bits[i];
    }

    double n = bits.size();

    double z = (ones - n / 2.0) / sqrt(n / 4.0);

    TestResult result;
    result.test = "NIST_bit_frequency_abs_z";
    result.statistic = fabs(z);
    result.low = 0.0;
    result.high = 1.96;
    result.pass = fabs(z) <= 1.96;

    return result;
}

// NIST runs test
TestResult bitRuns(const vector<uint8_t>& bits) {
    int runs = 1;

    for (int i = 1; i < (int)bits.size(); i++) {
        if (bits[i] != bits[i - 1]) {
            runs++;
        }
    }

    double n = bits.size();

    double expected = (2 * n - 1) / 2.0;
    double var = (2 * n - 1) / 4.0;

    double z = (runs - expected) / sqrt(var);

    TestResult result;
    result.test = "NIST_runs_abs_z";
    result.statistic = fabs(z);
    result.low = 0.0;
    result.high = 1.96;
    result.pass = fabs(z) <= 1.96;

    return result;
}

// Serial 2-bit test
TestResult serial2(const vector<uint8_t>& bits) {
    int cnt[4] = {0, 0, 0, 0};

    for (int i = 0; i + 1 < (int)bits.size(); i += 2) {
        int id = bits[i] * 2 + bits[i + 1];
        cnt[id]++;
    }

    double n = bits.size() / 2.0;
    double exp = n / 4.0;
    double chi2 = 0.0;

    for (int i = 0; i < 4; i++) {
        chi2 += (cnt[i] - exp) * (cnt[i] - exp) / exp;
    }

    double low = chiSquareCritical(3, 0.025);
    double high = chiSquareCritical(3, 0.975);

    TestResult result;
    result.test = "NIST_serial_2bit_chi2";
    result.statistic = chi2;
    result.low = low;
    result.high = high;
    result.pass = chi2 >= low && chi2 <= high;

    return result;
}

// Diehard monkey / poker test для 4-битных слов
TestResult poker4(const vector<uint8_t>& bits) {
    int cnt[16];

    for (int i = 0; i < 16; i++) {
        cnt[i] = 0;
    }

    int blocks = bits.size() / 4;

    for (int i = 0; i < blocks; i++) {
        int value = 0;

        for (int j = 0; j < 4; j++) {
            value = (value << 1) | bits[4 * i + j];
        }

        cnt[value]++;
    }

    double exp = blocks / 16.0;
    double chi2 = 0.0;

    for (int i = 0; i < 16; i++) {
        chi2 += (cnt[i] - exp) * (cnt[i] - exp) / exp;
    }

    double low = chiSquareCritical(15, 0.025);
    double high = chiSquareCritical(15, 0.975);

    TestResult result;
    result.test = "Diehard_monkey_poker4_chi2";
    result.statistic = chi2;
    result.low = low;
    result.high = high;
    result.pass = chi2 >= low && chi2 <= high;

    return result;
}

// Diehard overlapping permutations test
TestResult overlappingPermutations(const vector<double>& v) {
    int cnt[120];

    for (int i = 0; i < 120; i++) {
        cnt[i] = 0;
    }

    int fact[5] = {1, 1, 2, 6, 24};

    int windows = 0;

    for (int i = 0; i + 4 < (int)v.size(); i++) {
        pair<double, int> a[5];

        for (int j = 0; j < 5; j++) {
            a[j] = make_pair(v[i + j], j);
        }

        sort(a, a + 5);

        int perm[5];

        for (int rank = 0; rank < 5; rank++) {
            perm[rank] = a[rank].second;
        }

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
    double chi2 = 0.0;

    for (int i = 0; i < 120; i++) {
        chi2 += (cnt[i] - exp) * (cnt[i] - exp) / exp;
    }

    double low = chiSquareCritical(119, 0.025);
    double high = chiSquareCritical(119, 0.975);

    TestResult result;
    result.test = "Diehard_overlapping_permutations_chi2";
    result.statistic = chi2;
    result.low = low;
    result.high = high;
    result.pass = chi2 >= low && chi2 <= high;

    return result;
}

// Diehard birthday spacings test
TestResult birthdaySpacings(const vector<double>& v) {
    const int m = 1 << 20;

    int n = (int)v.size();

    if (n > 512) {
        n = 512;
    }

    vector<int> xs(n);

    for (int i = 0; i < n; i++) {
        xs[i] = int(v[i] * m);

        if (xs[i] > m - 1) {
            xs[i] = m - 1;
        }
    }

    sort(xs.begin(), xs.end());

    vector<int> spacings;

    for (int i = 1; i < n; i++) {
        spacings.push_back(xs[i] - xs[i - 1]);
    }

    sort(spacings.begin(), spacings.end());

    int collisions = 0;

    for (int i = 1; i < (int)spacings.size(); i++) {
        if (spacings[i] == spacings[i - 1]) {
            collisions++;
        }
    }

    double lambda = double(n * n * n) / (4.0 * m);

    double z = (collisions - lambda) / sqrt(max(lambda, 1.0));

    TestResult result;
    result.test = "Diehard_birthday_spacings_abs_z";
    result.statistic = fabs(z);
    result.low = 0.0;
    result.high = 3.0;
    result.pass = fabs(z) <= 3.0;

    return result;
}

// Запускает все NIST/Diehard-подобные тесты для одной выборки
vector<TestResult> allTests(const vector<uint32_t>& nums, const vector<double>& vals) {
    vector<uint8_t> bits = bitsFromNumbers(nums);

    vector<TestResult> results;

    results.push_back(bitFrequency(bits));
    results.push_back(bitRuns(bits));
    results.push_back(serial2(bits));
    results.push_back(poker4(bits));
    results.push_back(overlappingPermutations(vals));
    results.push_back(birthdaySpacings(vals));

    return results;
}


void processModifiedMiddleProductXor(
    ofstream& summary,
    ofstream& testsCsv,
    int sampleSize,
    int sampleNumber
) {
    uint64_t seed = 1234567ULL + 1000003ULL * sampleNumber;

    ModifiedMiddleProductXor gen(
        uint32_t(seed),
        uint32_t(seed * 1664525u + 1013904223u)
    );

    vector<uint32_t> nums;
    vector<double> vals;

    nums.reserve(sampleSize);
    vals.reserve(sampleSize);

    for (int i = 0; i < sampleSize; i++) {
        uint32_t u = gen.nextU32();

        nums.push_back(u);
        vals.push_back((u + 0.5) / TWO32);
    }

    BasicStats st = basicStats(vals);
    ChiResult ch = chiUniform(vals);
    vector<TestResult> tests = allTests(nums, vals);

    int passed = 0;

    for (int i = 0; i < (int)tests.size(); i++) {
        if (tests[i].pass) {
            passed++;
        }
    }

    summary << gen.name() << ','
            << sampleNumber + 1 << ','
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

    for (int i = 0; i < (int)tests.size(); i++) {
        testsCsv << gen.name() << ','
                 << sampleNumber + 1 << ','
                 << tests[i].test << ','
                 << tests[i].statistic << ','
                 << tests[i].low << ','
                 << tests[i].high << ','
                 << (tests[i].pass ? 1 : 0)
                 << "\n";
    }
}

void processModifiedLCGPermuted(
    ofstream& summary,
    ofstream& testsCsv,
    int sampleSize,
    int sampleNumber
) {
    uint64_t seed = 1234567ULL + 1000003ULL * sampleNumber + 911ULL;

    ModifiedLCGPermuted gen(seed);

    vector<uint32_t> nums;
    vector<double> vals;

    nums.reserve(sampleSize);
    vals.reserve(sampleSize);

    for (int i = 0; i < sampleSize; i++) {
        uint32_t u = gen.nextU32();

        nums.push_back(u);
        vals.push_back((u + 0.5) / TWO32);
    }

    BasicStats st = basicStats(vals);
    ChiResult ch = chiUniform(vals);
    vector<TestResult> tests = allTests(nums, vals);

    int passed = 0;

    for (int i = 0; i < (int)tests.size(); i++) {
        if (tests[i].pass) {
            passed++;
        }
    }

    summary << gen.name() << ','
            << sampleNumber + 1 << ','
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

    for (int i = 0; i < (int)tests.size(); i++) {
        testsCsv << gen.name() << ','
                 << sampleNumber + 1 << ','
                 << tests[i].test << ','
                 << tests[i].statistic << ','
                 << tests[i].low << ','
                 << tests[i].high << ','
                 << (tests[i].pass ? 1 : 0)
                 << "\n";
    }
}

void processXorShift32Salt(
    ofstream& summary,
    ofstream& testsCsv,
    int sampleSize,
    int sampleNumber
) {
    uint64_t seed = 1234567ULL + 1000003ULL * sampleNumber + 1822ULL;

    XorShift32Salt gen{uint32_t(seed)};

    vector<uint32_t> nums;
    vector<double> vals;

    nums.reserve(sampleSize);
    vals.reserve(sampleSize);

    for (int i = 0; i < sampleSize; i++) {
        uint32_t u = gen.nextU32();

        nums.push_back(u);
        vals.push_back((u + 0.5) / TWO32);
    }

    BasicStats st = basicStats(vals);
    ChiResult ch = chiUniform(vals);
    vector<TestResult> tests = allTests(nums, vals);

    int passed = 0;

    for (int i = 0; i < (int)tests.size(); i++) {
        if (tests[i].pass) {
            passed++;
        }
    }

    summary << gen.name() << ','
            << sampleNumber + 1 << ','
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

    for (int i = 0; i < (int)tests.size(); i++) {
        testsCsv << gen.name() << ','
                 << sampleNumber + 1 << ','
                 << tests[i].test << ','
                 << tests[i].statistic << ','
                 << tests[i].low << ','
                 << tests[i].high << ','
                 << (tests[i].pass ? 1 : 0)
                 << "\n";
    }
}


void speedModifiedMiddleProductXor(ofstream& speed, int N) {
    ModifiedMiddleProductXor gen(
        uint32_t(987654321ULL),
        uint32_t(987654321ULL * 1664525u + 1013904223u)
    );

    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        gen.nextU32();
    }

    high_resolution_clock::time_point t2 = high_resolution_clock::now();

    double ms = duration<double, milli>(t2 - t1).count();

    speed << gen.name() << ',' << N << ',' << ms << "\n";
}

void speedModifiedLCGPermuted(ofstream& speed, int N) {
    ModifiedLCGPermuted gen(987654321ULL + 1ULL);

    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        gen.nextU32();
    }

    high_resolution_clock::time_point t2 = high_resolution_clock::now();

    double ms = duration<double, milli>(t2 - t1).count();

    speed << gen.name() << ',' << N << ',' << ms << "\n";
}

void speedXorShift32Salt(ofstream& speed, int N) {
    XorShift32Salt gen(uint32_t(987654321ULL + 2ULL));

    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        gen.nextU32();
    }

    high_resolution_clock::time_point t2 = high_resolution_clock::now();

    double ms = duration<double, milli>(t2 - t1).count();

    speed << gen.name() << ',' << N << ',' << ms << "\n";
}

void speedStdMT19937(ofstream& speed, int N) {
    StdMT19937 gen(uint32_t(987654321ULL + 3ULL));

    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        gen.nextU32();
    }

    high_resolution_clock::time_point t2 = high_resolution_clock::now();

    double ms = duration<double, milli>(t2 - t1).count();

    speed << gen.name() << ',' << N << ',' << ms << "\n";
}


int main() {
    const int samplesPerMethod = 20;
    const int sampleSize = 1000;

    ofstream summary("results_summary.csv");
    summary << "method,sample,N,mean,stddev,cv,chi_bins,chi2,chi_low,chi_high,chi_pass,tests_passed,total_tests\n";

    ofstream testsCsv("tests_detail.csv");
    testsCsv << "method,sample,test,statistic,low,high,pass\n";

    for (int s = 0; s < samplesPerMethod; s++) {
        processModifiedMiddleProductXor(summary, testsCsv, sampleSize, s);
        processModifiedLCGPermuted(summary, testsCsv, sampleSize, s);
        processXorShift32Salt(summary, testsCsv, sampleSize, s);
    }

    ofstream speed("speed.csv");
    speed << "method,N,milliseconds\n";

    int sizes[8] = {
        1000,
        5000,
        10000,
        50000,
        100000,
        250000,
        500000,
        1000000
    };

    for (int i = 0; i < 8; i++) {
        speedModifiedMiddleProductXor(speed, sizes[i]);
        speedModifiedLCGPermuted(speed, sizes[i]);
        speedXorShift32Salt(speed, sizes[i]);
        speedStdMT19937(speed, sizes[i]);
    }

    cout << "Done";
}