#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <chrono>
#include <random>
using namespace std;
using namespace std::chrono;

const string matrix1_path = "C:/Parallel-programing/lab1/matrix1.txt";
const string matrix2_path = "C:/Parallel-programing/lab1/matrix2.txt";
const string result_path = "C:/Parallel-programing/lab1/endfile.txt";

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
  const long long sec, const long long ms, const string& result_path) {
  ofstream file(result_path);

  if (!file.is_open()) {
    cerr << "Ошибка создания файла" << endl;
    return;
  }

  int n = matrix.size();
  file << n << endl;

  vector<vector<double>> result_matrix(n, vector<double>(n, 0.0));
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      file << matrix[i][j] << " ";
    }
    file << endl;
  }
  cout << endl << "Время умножения матриц: " << endl;
  cout << sec << " секунд" << endl;
  cout << ms << " милисекунд" << endl;
  cout << mks << " микросекунд" << endl;
  cout << "Объем задачи (n^3): " << n * n * n << " операций" << endl;
  file.close();
  cout << "Матрица записана в файл: endfile " << endl << endl;

  cout << "Проверка через NumPY в питоне:" << endl;
  system("python C:/Parallel-programing/lab1/check.py");
}

vector<vector<double>> multiplication(const vector<vector<double>>& matrix1,
  const vector<vector<double>>& matrix2) {

  int n = matrix1.size();
  vector<vector<double>> result_matrix(n, vector<double>(n, 0.0));
  for (int k = 0; k < n; ++k) {
    for (int i = 0; i < n; ++i) {
      for (int j = 0; j < n; ++j) {
        result_matrix[k][i] += matrix1[k][j] * matrix2[j][i];
      }
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
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      file >> matrix[i][j];
    }
  }
  file.close();

  return matrix;
}

int main() {
  cout << "Выбор: \n 1. Создать свою рандомную матрицу(вместе с файлом) \n 2. Взять готовые файлы с матрицами" << endl;
  cout << "Ваш выбор: ";
  int choose = 0;
  cin >> choose;
  if (choose == 1) {
    int size;
    cout << "Выберите количество строк и столбцов: ";
    cin >> size;
    create_matrix(matrix1_path, size);
    create_matrix(matrix2_path, size);

    auto matrix1 = read_matrix(matrix1_path);
    auto matrix2 = read_matrix(matrix2_path);

    auto start = high_resolution_clock::now();

    auto result_matrix = multiplication(matrix1, matrix2);

    auto end = high_resolution_clock::now();

    auto total_microseconds = duration_cast<microseconds>(end - start).count();
    long long sec = total_microseconds / 1'000'000;
    long long ms = (total_microseconds % 1'000'000) / 1'000;
    long long mks = total_microseconds % 1'000; 

    write_in_file(result_matrix, mks, sec, ms, result_path);
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

    auto start = high_resolution_clock::now();

    auto result_matrix = multiplication(matrix1, matrix2);

    auto end = high_resolution_clock::now();

    auto total_microseconds = duration_cast<microseconds>(end - start).count();
    long long sec = total_microseconds / 1'000'000;
    long long ms = (total_microseconds % 1'000'000) / 1'000;
    long long mks = total_microseconds % 1'000; 

    write_in_file(result_matrix, mks, sec, ms, result_path);
    return 0;
  }
  else {
    cout << "Такого числа нет. Конец программы!";
  }
}