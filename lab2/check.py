import numpy as np

matrix1 = 'matrix1.txt'
matrix2 = 'matrix2.txt'
endfile = 'endfile.txt'

def load_matrix(filename):
    with open(filename, 'r') as f:
        n = int(f.readline().strip())
        matrix = np.loadtxt(f, max_rows=n)
        return matrix

A = load_matrix(matrix1)
B = load_matrix(matrix2)

result_numpy = np.dot(A, B)

print("Проверка")
loaded_result = load_matrix(endfile)

if np.allclose(result_numpy, loaded_result):
    print("Результаты совпали")
else:
    print("Результаты не совпали")