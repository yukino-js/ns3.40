
#ifndef MATRIX_ARRAY_H
#define MATRIX_ARRAY_H

#include "val-array.h"

#include <valarray>

namespace ns3 {

template <class T> class MatrixArray : public ValArray<T> {
public:
  MatrixArray<T>() = default;
  MatrixArray<T>(size_t numRows, size_t numCols = 1, size_t numPages = 1);
  explicit MatrixArray<T>(const std::valarray<T> &values);
  MatrixArray<T>(std::valarray<T> &&values);
  explicit MatrixArray<T>(const std::vector<T> &values);
  MatrixArray<T>(size_t numRows, size_t numCols,
                 const std::valarray<T> &values);
  MatrixArray<T>(size_t numRows, size_t numCols, std::valarray<T> &&values);
  MatrixArray<T>(size_t numRows, size_t numCols, size_t numPages,
                 const std::valarray<T> &values);
  MatrixArray<T>(size_t numRows, size_t numCols, size_t numPages,
                 std::valarray<T> &&values);
  ~MatrixArray<T>() override = default;
  MatrixArray<T>(const MatrixArray<T> &) = default;
  MatrixArray<T> &operator=(const MatrixArray<T> &) = default;
  MatrixArray<T>(MatrixArray<T> &&) = default;
  MatrixArray<T> &operator=(MatrixArray<T> &&) = default;
  MatrixArray<T> operator*(const T &rhs) const;
  MatrixArray<T> operator+(const MatrixArray<T> &rhs) const;
  MatrixArray<T> operator-(const MatrixArray<T> &rhs) const;
  MatrixArray<T> operator-() const;
  MatrixArray<T> operator*(const MatrixArray<T> &rhs) const;
  MatrixArray<T> Transpose() const;
  MatrixArray<T>
  MultiplyByLeftAndRightMatrix(const MatrixArray<T> &lMatrix,
                               const MatrixArray<T> &rMatrix) const;

  using ValArray<T>::GetPagePtr;
  using ValArray<T>::EqualDims;
  using ValArray<T>::AssertEqualDims;

  template <bool EnableBool = true,
            typename = std::enable_if_t<
                (std::is_same_v<T, std::complex<double>> && EnableBool)>>
  MatrixArray<T> HermitianTranspose() const;

protected:
  using ValArray<T>::m_numRows;
  using ValArray<T>::m_numCols;
  using ValArray<T>::m_numPages;
  using ValArray<T>::m_values;
};

using IntMatrixArray = MatrixArray<int>;

using DoubleMatrixArray = MatrixArray<double>;

using ComplexMatrixArray = MatrixArray<std::complex<double>>;

template <class T>
inline MatrixArray<T> MatrixArray<T>::operator*(const T &rhs) const {
  return MatrixArray<T>(
      m_numRows, m_numCols, m_numPages,
      m_values * std::valarray<T>(rhs, m_numRows * m_numCols * m_numPages));
}

template <class T>
inline MatrixArray<T>
MatrixArray<T>::operator+(const MatrixArray<T> &rhs) const {
  AssertEqualDims(rhs);
  return MatrixArray<T>(m_numRows, m_numCols, m_numPages,
                        m_values + rhs.m_values);
}

template <class T>
inline MatrixArray<T>
MatrixArray<T>::operator-(const MatrixArray<T> &rhs) const {
  AssertEqualDims(rhs);
  return MatrixArray<T>(m_numRows, m_numCols, m_numPages,
                        m_values - rhs.m_values);
}

template <class T> inline MatrixArray<T> MatrixArray<T>::operator-() const {
  return MatrixArray<T>(m_numRows, m_numCols, m_numPages, -m_values);
}

} // namespace ns3

#endif
