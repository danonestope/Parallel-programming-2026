import sys
import numpy as np


def read_matrix(file_name: str) -> np.ndarray:
    """Читает матрицу формата лабы №1: первая строка — n, далее n строк по n чисел."""
    with open(file_name, "r", encoding="utf-8") as f:
        n = int(f.readline().strip())
        rows = []
        for _ in range(n):
            rows.append([float(x) for x in f.readline().split()])
    return np.array(rows)


def main():
    if len(sys.argv) < 5:
        print("Usage: python check.py <matrix1> <matrix2> <cpp_result> <py_result>")
        sys.exit(1)

    path_a, path_b, path_cpp, path_py = sys.argv[1:5]

    A = read_matrix(path_a)
    B = read_matrix(path_b)
    C_cpp = read_matrix(path_cpp)

    C_py = A @ B
    np.savetxt(path_py, C_py, delimiter=" ", fmt="%.10g")

    if C_cpp.shape == C_py.shape and np.allclose(C_cpp, C_py, rtol=0, atol=1e-6):
        print("С++ матрица равна numpy матрице")
    else:
        print("Матрицы не равны")
        if C_cpp.shape != C_py.shape:
            print(f"  shape cpp={C_cpp.shape}, py={C_py.shape}")
        else:
            print(f"  max diff = {np.abs(C_cpp - C_py).max()}")


if __name__ == "__main__":
    main()