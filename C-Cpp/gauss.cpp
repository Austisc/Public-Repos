#include <iostream>
#include <vector>
template <typename T> class Mat {

private:
  int rows, cols;
  std::vector<std::vector<T>> MatC;

public:
  Mat(int rows, int cols) : rows(rows), cols(cols) {}
};

template <typename T>
double searchPivot(std::vector<std::vector<T>> &A, std::vector<int> &ipiv,
                   int &icol, int &irow, int n) {
  // search for largest pivot element
  double big = 0.0;
  for (int j = 0; j < n; j++) {
    if (ipiv[j] != 1) {
      for (int k = 0; k < n; k++) {
        if (ipiv[k] == 0) {
          if (abs(A[j][k]) >= big) {
            big = abs(A[j][k]);
            irow = j;
            icol = k;
          }
        }
      }
    }
  }
  return big;
}
// interchange the row given based on the pivot row and col
template <typename T> void swapRow(Mat<T> &A, int &irow, int &icol, int n) {
  if (irow != icol) {
    for (int l = 0; l < n; l++) // swap the row betwee
      std::swap(A[irow][l], A[icol][l]);
  };
}

template <typename T> void scalRow(Mat<T> &A, int &icol, int n) {
  T pivinv = 1.0 / A[icol][icol];
  A.MatC[icol][icol] = 1.0;
  for (int l = 0; l < n; l++) {
    A[icol][l] *= pivinv;
  }
}

template <typename T>
void interChangeCols(Mat<T> &A, std::vector<int> indxr, std::vector<int> indxc,
                     int n) {
  for (int i = n - 1; i >= 0; i--) {
    if (indxr[i] != indxc[i])
      for (int r = 0; r < n; r++) {

        std::swap(A[r][indxr[i]], A[r][indxc[i]]);
      }
  }
}

template <typename T> void reducingRow(Mat<T> &A, int &icol, int n) {
  for (int row = 0; row < n; row++) {
    if (row != icol) {
      double dum = A[row][icol];
      A[row][icol] = 0.0;
      for (int col = 0; col < n; col++) {
        A[row][col] -= A[icol][col] * dum;
      }
    }
  }
}

template <typename T> Mat<T> GaussJordan(Mat<T> a) {
  Mat<T> &A = a;
  int i, icol, irow, j, k, l, ll, n = A.rows, m = A.cols;
  double big, dum, pivinv;
  std::vector<int> indxc = std::vector<int>(n, 0),
                   indxr = std::vector<int>(n, 0),
                   ipiv = std::vector<int>(n, 0);
  for (int i = 0; i < n; i++) {

    big = searchPivot(A, ipiv, icol, irow, n);
    swapRow(A, irow, icol, n);
    indxr[i] = irow;
    indxc[i] = icol;
    if (A[icol][icol] == 0.0) {
      return;
    };
    reducingRow(A, icol, n);
  }
  interChangeCols(A, indxr, indxc, n);
}
#include <vector>
template <typename T> class Mat {

private:
  int rows, cols;
  std::vector<std::vector<T>> MatC;

public:
  Mat(int rows, int cols) : rows(rows), cols(cols) {}
};

template <typename T>
double searchPivot(std::vector<std::vector<T>> &A, std::vector<int> &ipiv,
                   int &icol, int &irow, int n) {
  // search for largest pivot element
  double big = 0.0;
  for (int j = 0; j < n; j++) {
    if (ipiv[j] != 1) {
      for (int k = 0; k < n; k++) {
        if (ipiv[k] == 0) {
          if (abs(A[j][k]) >= big) {
            big = abs(A[j][k]);
            irow = j;
            icol = k;
          }
        }
      }
    }
  }
  return big;
}
// interchange the row given based on the pivot row and col
template <typename T> void swapRow(Mat<T> &A, int &irow, int &icol, int n) {
  if (irow != icol) {
    for (int l = 0; l < n; l++) // swap the row betwee
      std::swap(A[irow][l], A[icol][l]);
  };
}

template <typename T> void scalRow(Mat<T> &A, int &icol, int n) {
  T pivinv = 1.0 / A[icol][icol];
  A.MatC[icol][icol] = 1.0;
  for (int l = 0; l < n; l++) {
    A[icol][l] *= pivinv;
  }
}

template <typename T>
void interChangeCols(Mat<T> &A, std::vector<int> indxr, std::vector<int> indxc,
                     int n) {
  for (int i = n - 1; i >= 0; i--) {
    if (indxr[i] != indxc[i])
      for (int r = 0; r < n; r++) {

        std::swap(A[r][indxr[i]], A[r][indxc[i]]);
      }
  }
}

template <typename T> void reducingRow(Mat<T> &A, int &icol, int n) {
  for (int row = 0; row < n; row++) {
    if (row != icol) {
      double dum = A[row][icol];
      A[row][icol] = 0.0;
      for (int col = 0; col < n; col++) {
        A[row][col] -= A[icol][col] * dum;
      }
    }
  }
}

template <typename T> Mat<T> GaussJordan(Mat<T> a) {
  Mat<T> A = a;
  int i, icol, irow, j, k, l, ll, n = A.rows, m = A.cols;
  double big, dum, pivinv;
  std::vector<int> indxc = std::vector<int>(n, 0),
                   indxr = std::vector<int>(n, 0),
                   ipiv = std::vector<int>(n, 0);
  for (int i = 0; i < n; i++) {

    big = searchPivot(A, ipiv, icol, irow, n);
    swapRow(A, irow, icol, n);
    indxr[i] = irow;
    indxc[i] = icol;
    if (A[icol][icol] == 0.0) {
      return;
    };
    reducingRow(A, icol, n);
  }
  interChangeCols(A, indxr, indxc, n);
  return A;
}