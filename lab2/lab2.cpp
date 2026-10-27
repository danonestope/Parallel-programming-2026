#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <chrono>
#include <random>
#include <cstdlib>
#include <omp.h>

using namespace std;
using namespace std::chrono;

const string matrix1_path = "matrix1.txt";
const string matrix2_path = "matrix2.txt";
const string result_path = "endfile.txt";

void create_matrix(const string& filename, const int& size) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 100);

    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Ошибка создания файла" << endl;
        return;
    }
    file << size << endl;

    vector<vector<double>> matrix(size, vector<double>(size, 0.0));
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            matrix[i][j] = dist(gen);
            file << matrix[i][j] << " ";
        }
        file << endl;
    }
    file.close();
}

void write_in_file(const vector<vector<double>>& matrix, const long long mks,
                   const long long sec, const long long ms,
                   const string& result_path, int num_threads) {
    ofstream file(result_path);
    if (!file.is_open()) {
        cerr << "Ошибка создания файла: " << result_path << endl;
        return;
    }

    int n = matrix.size();
    file << n << endl;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            file << matrix[i][j] << " ";
        }
        file << endl;
    }
    file.close();

    cout << endl << "Время умножения матриц (потоков: " << num_threads << "):" << endl;
    cout << sec  << " секунд" << endl;
    cout << ms   << " милисекунд" << endl;
    cout << mks  << " микросекунд" << endl;
    cout << "Объем задачи (n^3): " << (long long)n * n * n << " операций" << endl;
    cout << "Матрица записана в файл: endfile" << endl << endl;

    cout << "Проверка через NumPY в питоне:" << endl;
    system("python check.py");;
}

vector<vector<double>> multiplication(const vector<vector<double>>& matrix1,
                                      const vector<vector<double>>& matrix2,
                                      int num_threads) {
    int n = (int)matrix1.size();
    vector<vector<double>> result_matrix(n, vector<double>(n, 0.0));

    omp_set_num_threads(num_threads);

    #pragma omp parallel for default(none) \
    shared(matrix1, matrix2, result_matrix, n) schedule(static)
    for (int i = 0; i < n; ++i) {
        for (int k = 0; k < n; ++k) {
            double aik = matrix1[i][k];
            for (int j = 0; j < n; ++j)
                result_matrix[i][j] += aik * matrix2[k][j];
        }
    }
    return result_matrix;
}

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

int main(int argc, char* argv[]) {
  int cli_size    = (argc > 1) ? atoi(argv[1]) : 0;
  int cli_threads = (argc > 2) ? atoi(argv[2]) : 4;

  cout << "Доступно ядер: " << omp_get_num_procs() << endl;
  cout << "Max threads по умолчанию: " << omp_get_max_threads() << endl;

  int choose = 0;
  int size = 0;

  if (cli_size > 0) {
    choose = 1;
    size = cli_size;
  } 
  else {
    cout << "Выбор: \n 1. Создать свою рандомную матрицу (вместе с файлом)\n"
      << " 2. Взять готовые файлы с матрицами" << endl;
    cout << "Ваш выбор: ";
    cin >> choose;
  }

  if (choose == 1) {
    if (size == 0) {
      cout << "Выберите количество строк и столбцов: ";
      cin >> size;
    }
    
    create_matrix(matrix1_path, size);
    create_matrix(matrix2_path, size);

    auto matrix1 = read_matrix(matrix1_path);
    auto matrix2 = read_matrix(matrix2_path);

    double start = omp_get_wtime();

    auto result_matrix = multiplication(matrix1, matrix2, cli_threads);

    double end = omp_get_wtime();

    long long total_microseconds = (long long)((end - start) * 1'000'000);
    long long sec = total_microseconds / 1'000'000;
    long long ms = (total_microseconds % 1'000'000) / 1'000;
    long long mks = total_microseconds % 1'000; 

      write_in_file(result_matrix, mks, sec, ms, result_path, cli_threads);
      return 0;
  }

  else if (choose == 2) {
      auto matrix1 = read_matrix(matrix1_path);
      auto matrix2 = read_matrix(matrix2_path);

    if (matrix1.size() != matrix2.size()) {
      cerr << "Ошибка: Размеры матриц не совпадают"
      << matrix1.size() << " vs " << matrix2.size() << endl;
      return 1;
    }

      double start = omp_get_wtime();
      auto result_matrix = multiplication(matrix1, matrix2, cli_threads);
      double end = omp_get_wtime();
      long long total_microseconds = (long long)((end - start) * 1'000'000);

      long long sec = total_microseconds / 1'000'000;
      long long ms = (total_microseconds % 1'000'000) / 1'000;
      long long mks = total_microseconds % 1'000; 

      write_in_file(result_matrix, mks, sec, ms, result_path, cli_threads);
      return 0;
  }
  else {
      cout << "Такого числа нет. Конец программы!";
  }
  return 0;
}