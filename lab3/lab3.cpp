#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <chrono>
#include <random>
#include <sstream>
#include <omp.h>

using namespace std;
using namespace std::chrono;

const string matrix1_path   = "C:/Parallel-programing/lab3/matrix1.txt";
const string matrix2_path   = "C:/Parallel-programing/lab3/matrix2.txt";
const string result_path    = "C:/Parallel-programing/lab3/endfile.txt";
const string py_result_path = "C:/Parallel-programing/lab3/py_endfile.txt";
const string data_path      = "C:/Parallel-programing/lab3/data.txt";

// ---------- Генерация матрицы ----------
void create_matrix(const string& filename, const int& size) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(1, 100);

    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Ошибка создания файла " << filename << endl;
        return;
    }
    file << size << endl;

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            file << dist(gen) << " ";
        }
        file << endl;
    }
    file.close();
}

// ---------- Чтение матрицы ----------
vector<vector<double>> read_matrix(const string& name) {
    ifstream file(name);
    if (!file.is_open()) {
        cerr << "Ошибка: Не удалось открыть файл " << name << endl;
        return {};
    }
    int n;
    file >> n;
    vector<vector<double>> matrix(n, vector<double>(n));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            file >> matrix[i][j];
    file.close();
    return matrix;
}

// ---------- Запись результата ----------
void write_in_file(const vector<vector<double>>& matrix, const string& path) {
    ofstream file(path);
    if (!file.is_open()) {
        cerr << "Ошибка создания файла " << path << endl;
        return;
    }
    int n = matrix.size();
    file << n << endl;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j)
            file << matrix[i][j] << " ";
        file << endl;
    }
    file.close();
    cout << "Матрица записана в файл: " << path << endl;
}

// ---------- OpenMP умножение ----------
vector<vector<double>> multiplication_omp(
    const vector<vector<double>>& A,
    const vector<vector<double>>& B,
    int num_threads)
{
    int n = A.size();
    vector<vector<double>> C(n, vector<double>(n, 0.0));

    omp_set_num_threads(num_threads);

    #pragma omp parallel for schedule(static)
    for (int k = 0; k < n; ++k) {
        for (int i = 0; i < n; ++i) {
            double aik = A[k][i];
            for (int j = 0; j < n; ++j) {
                C[k][i] += aik * B[j][i];
            }
        }
    }
    return C;
}

// ---------- Список потоков: 1, 2, 4, 8, ... ----------
vector<int> threads_list(int max_threads) {
    vector<int> t;
    for (int c = 1; c <= max_threads; c *= 2)
        t.push_back(c);
    return t;
}

// ---------- Замер одного прогона ----------
long long run_and_measure(const vector<vector<double>>& A,
                          const vector<vector<double>>& B,
                          int threads,
                          vector<vector<double>>& outC)
{
    auto start = high_resolution_clock::now();
    outC = multiplication_omp(A, B, threads);
    auto end = high_resolution_clock::now();
    return duration_cast<microseconds>(end - start).count();
}

int main() {
    cout << "Выбор:\n 1. Создать свою рандомную матрицу (вместе с файлом)\n"
         << " 2. Взять готовые файлы с матрицами\n";
    cout << "Ваш выбор: ";
    int choose = 0;
    cin >> choose;

    if (choose == 1) {
        int size;
        cout << "Выберите количество строк и столбцов: ";
        cin >> size;
        create_matrix(matrix1_path, size);
        create_matrix(matrix2_path, size);
    }
    else if (choose != 2) {
        cout << "Такого числа нет. Конец программы!" << endl;
        return 0;
    }

    auto A = read_matrix(matrix1_path);
    auto B = read_matrix(matrix2_path);

    if (A.empty() || B.empty()) {
        cerr << "Ошибка чтения матриц" << endl;
        return 1;
    }
    if (A.size() != B.size()) {
        cerr << "Ошибка: Размеры матриц не совпадают "
             << A.size() << " vs " << B.size() << endl;
        return 1;
    }

    int n = A.size();
    int max_threads = omp_get_max_threads();
    cout << "Размер матриц: " << n << "x" << n << endl;
    cout << "Доступно потоков (max): " << max_threads << endl;

    auto thread_values = threads_list(max_threads);
    int last_threads = thread_values.back();

    stringstream data_text;
    data_text << "| n | threads | time_mks | time_sec |\n";

        // Объявляем результирующую матрицу как вектор векторов (как в функции умножения)
    vector<vector<double>> result_matrix;
    long long last_mks = 0;

    for (int threads : thread_values) {
        vector<vector<double>> C;
        long long mks = run_and_measure(A, B, threads, C);

        cout << "threads=" << threads
             << "  time=" << mks << " mks  ("
             << (mks / 1'000'000.0) << " s)" << endl;

        stringstream ss;
        ss << "| " << n
           << " | " << threads
           << " | " << mks
           << " | " << (mks / 1'000'000.0) << " |\n";
        
        // В stringstream добавляем через <<, а не += (хотя += тоже работает для stringstream)
        data_text << ss.str(); 

        if (threads == last_threads) {
            result_matrix = std::move(C); // Теперь типы совпадают
            last_mks = mks;
        }
    }

    write_in_file(result_matrix, result_path);

    long long sec = last_mks / 1'000'000;
    long long ms  = (last_mks % 1'000'000) / 1'000;
    long long mks = last_mks % 1'000;
    cout << endl << "Время умножения матриц (последний прогон):" << endl;
    cout << sec << " секунд" << endl;
    cout << ms  << " милисекунд" << endl;
    cout << mks << " микросекунд" << endl;
    cout << "Объем задачи (n^3): " << 1LL * n * n * n << " операций" << endl;

    ofstream out(data_path);
    if (out.is_open()) {
        out << data_text.str();
        out.close();
        cout << "\nРезультаты экспериментов сохранены в " << data_path << endl;
    }

    cout << "\nПроверка через NumPY в питоне:" << endl;
    string cmd = "python C:/Parallel-programing/lab3/check.py \""
                 + matrix1_path + "\" \""
                 + matrix2_path + "\" \""
                 + result_path  + "\" \""
                 + py_result_path + "\"";
    system(cmd.c_str());

    return 0;
}