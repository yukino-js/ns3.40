#ifndef VAL_ARRAY_H
#define VAL_ARRAY_H

#include "assert.h"
#include "simple-ref-count.h"

#include <complex>
#include <valarray>
#include <vector>

namespace ns3 {

template <class T> class ValArray : public SimpleRefCount<ValArray<T>> {
public:
  ValArray<T>() = default;
  ValArray<T>(size_t numRows, size_t numCols = 1, size_t numPages = 1);
  explicit ValArray<T>(const std::valarray<T> &values);
  ValArray<T>(std::valarray<T> &&values);
  explicit ValArray<T>(const std::vector<T> &values);
  ValArray<T>(size_t numRows, size_t numCols, const std::valarray<T> &values);
  ValArray<T>(size_t numRows, size_t numCols, std::valarray<T> &&values);
  ValArray<T>(size_t numRows, size_t numCols, size_t numPages,
              const std::valarray<T> &values);
  ValArray<T>(size_t numRows, size_t numCols, size_t numPages,
              std::valarray<T> &&values);
  virtual ~ValArray<T>() = default;
  ValArray<T>(const ValArray<T> &) = default;
  ValArray<T> &operator=(const ValArray<T> &) = default;
  ValArray<T>(ValArray<T> &&) = default;
  ValArray<T> &operator=(ValArray<T> &&) = default;
  size_t GetNumRows() const;
  size_t GetNumCols() const;
  size_t GetNumPages() const;
  size_t GetSize() const;
  T &operator()(size_t rowIndex, size_t colIndex, size_t pageIndex);
  const T &operator()(size_t rowIndex, size_t colIndex, size_t pageIndex) const;
  T &operator()(size_t rowIndex, size_t colIndex);
  const T &operator()(size_t rowIndex, size_t colIndex) const;
  T &operator()(size_t index);
  const T &operator()(size_t index) const;
  ValArray<T> operator*(const T &rhs) const;
  ValArray<T> operator+(const ValArray<T> &rhs) const;
  ValArray<T> operator-(const ValArray<T> &rhs) const;
  ValArray<T> operator-() const;
  ValArray<T> &operator+=(const ValArray<T> &rhs);
  ValArray<T> &operator-=(const ValArray<T> &rhs);
  bool operator==(const ValArray<T> &rhs) const;
  bool operator!=(const ValArray<T> &rhs) const;
  bool IsAlmostEqual(const ValArray<T> &rhs, T tol) const;
  T *GetPagePtr(size_t pageIndex);
  const T *GetPagePtr(size_t pageIndex) const;
  bool EqualDims(const ValArray<T> &rhs) const;
  void AssertEqualDims(const ValArray<T> &rhs) const;
  T &operator[](size_t index);
  const T &operator[](size_t index) const;
  const std::valarray<T> &GetValues() const;
  T &Elem(size_t row, size_t col, size_t page);
  const T &Elem(size_t row, size_t col, size_t page) const;

protected:
  size_t m_numRows = 0;
  size_t m_numCols = 0;
  size_t m_numPages = 0;
  std::valarray<T> m_values;
};

template <class T> inline size_t ValArray<T>::GetNumRows() const {
  return m_numRows;
};

template <class T> inline size_t ValArray<T>::GetNumCols() const {
  return m_numCols;
};

template <class T> inline size_t ValArray<T>::GetNumPages() const {
  return m_numPages;
};

template <class T> inline size_t ValArray<T>::GetSize() const {
  return m_values.size();
}

template <class T>
inline T &ValArray<T>::operator()(size_t rowIndex, size_t colIndex,
                                  size_t pageIndex) {
  NS_ASSERT_MSG(rowIndex < m_numRows, "Row index out of bounds");
  NS_ASSERT_MSG(colIndex < m_numCols, "Column index out of bounds");
  NS_ASSERT_MSG(pageIndex < m_numPages, "Pages index out of bounds");
  size_t index = (rowIndex + m_numRows * (colIndex + m_numCols * pageIndex));
  return m_values[index];
};

template <class T>
inline const T &ValArray<T>::operator()(size_t rowIndex, size_t colIndex,
                                        size_t pageIndex) const {
  NS_ASSERT_MSG(rowIndex < m_numRows, "Row index out of bounds");
  NS_ASSERT_MSG(colIndex < m_numCols, "Column index out of bounds");
  NS_ASSERT_MSG(pageIndex < m_numPages, "Pages index out of bounds");
  size_t index = (rowIndex + m_numRows * (colIndex + m_numCols * pageIndex));
  return m_values[index];
};

template <class T>
inline T &ValArray<T>::operator()(size_t rowIndex, size_t colIndex) {
  NS_ASSERT_MSG(m_numPages == 1,
                "Cannot use 2D access operator for 3D ValArray.");
  return (*this)(rowIndex, colIndex, 0);
};

template <class T>
inline const T &ValArray<T>::operator()(size_t rowIndex,
                                        size_t colIndex) const {
  NS_ASSERT_MSG(m_numPages == 1,
                "Cannot use 2D access operator for 3D ValArray.");
  return (*this)(rowIndex, colIndex, 0);
};

template <class T> inline T &ValArray<T>::operator()(size_t index) {
  NS_ASSERT_MSG(index < m_values.size(),
                "Invalid index to 1D ValArray. The size of the array should be "
                "set through "
                "constructor.");
  NS_ASSERT_MSG(((m_numRows == 1 || m_numCols == 1) && (m_numPages == 1)) ||
                    (m_numRows == 1 && m_numCols == 1),
                "Access operator allowed only for 1D ValArray.");
  return m_values[index];
};

template <class T> inline const T &ValArray<T>::operator()(size_t index) const {
  NS_ASSERT_MSG(index < m_values.size(),
                "Invalid index to 1D ValArray.The size of the array should be "
                "set through "
                "constructor.");
  NS_ASSERT_MSG(((m_numRows == 1 || m_numCols == 1) && (m_numPages == 1)) ||
                    (m_numRows == 1 && m_numCols == 1),
                "Access operator allowed only for 1D ValArray.");
  return m_values[index];
};

template <class T>
inline ValArray<T> ValArray<T>::operator*(const T &rhs) const {
  return ValArray<T>(
      m_numRows, m_numCols, m_numPages,
      m_values * std::valarray<T>(rhs, m_numRows * m_numCols * m_numPages));
}

template <class T>
inline ValArray<T> ValArray<T>::operator+(const ValArray<T> &rhs) const {
  AssertEqualDims(rhs);
  return ValArray<T>(m_numRows, m_numCols, m_numPages, m_values + rhs.m_values);
}

template <class T>
inline ValArray<T> ValArray<T>::operator-(const ValArray<T> &rhs) const {
  AssertEqualDims(rhs);
  return ValArray<T>(m_numRows, m_numCols, m_numPages, m_values - rhs.m_values);
}

template <class T> inline ValArray<T> ValArray<T>::operator-() const {
  return ValArray<T>(m_numRows, m_numCols, m_numPages, -m_values);
}

template <class T>
inline ValArray<T> &ValArray<T>::operator+=(const ValArray<T> &rhs) {
  AssertEqualDims(rhs);
  m_values += rhs.m_values;
  return *this;
}

template <class T>
inline ValArray<T> &ValArray<T>::operator-=(const ValArray<T> &rhs) {
  AssertEqualDims(rhs);
  m_values -= rhs.m_values;
  return *this;
}

template <class T> inline T *ValArray<T>::GetPagePtr(size_t pageIndex) {
  NS_ASSERT_MSG(pageIndex < m_numPages, "Invalid page index.");
  return &(m_values[m_numRows * m_numCols * pageIndex]);
};

template <class T>
inline const T *ValArray<T>::GetPagePtr(size_t pageIndex) const {
  NS_ASSERT_MSG(pageIndex < m_numPages, "Invalid page index.");
  return &(m_values[m_numRows * m_numCols * pageIndex]);
};

template <class T>
inline bool ValArray<T>::EqualDims(const ValArray<T> &rhs) const {
  return (m_numRows == rhs.m_numRows) && (m_numCols == rhs.m_numCols) &&
         (m_numPages == rhs.m_numPages);
}

template <class T> inline T &ValArray<T>::operator[](size_t index) {
  return (*this)(index);
}

template <class T> inline const T &ValArray<T>::operator[](size_t index) const {
  return (*this)(index);
}

template <class T>
inline const std::valarray<T> &ValArray<T>::GetValues() const {
  return m_values;
}

template <class T>
inline T &ValArray<T>::Elem(size_t row, size_t col, size_t page) {
  return (*this)(row, col, page);
};

template <class T>
inline const T &ValArray<T>::Elem(size_t row, size_t col, size_t page) const {
  return (*this)(row, col, page);
};

template <class T>
ValArray<T>::ValArray(size_t numRows, size_t numCols, size_t numPages)
    : m_numRows{numRows}, m_numCols{numCols}, m_numPages{numPages} {
  m_values.resize(m_numRows * m_numCols * m_numPages);
};

template <class T>
ValArray<T>::ValArray(const std::valarray<T> &values)
    : m_numRows{values.size()}, m_numCols{1}, m_numPages{1}, m_values{values} {}

template <class T>
ValArray<T>::ValArray(std::valarray<T> &&values)
    : m_numRows{values.size()}, m_numCols{1}, m_numPages{1},
      m_values{std::move(values)} {}

template <class T>
ValArray<T>::ValArray(const std::vector<T> &values)
    : m_numRows{values.size()}, m_numCols{1}, m_numPages{1} {
  m_values.resize(values.size());
  std::copy(values.begin(), values.end(), std::begin(m_values));
}

template <class T>
ValArray<T>::ValArray(size_t numRows, size_t numCols,
                      const std::valarray<T> &values)
    : m_numRows{numRows}, m_numCols{numCols}, m_numPages{1}, m_values{values} {
  NS_ASSERT_MSG(m_numRows * m_numCols == values.size(),
                "Dimensions and the initialization array size do not match.");
};

template <class T>
ValArray<T>::ValArray(size_t numRows, size_t numCols, std::valarray<T> &&values)
    : m_numRows{numRows}, m_numCols{numCols}, m_numPages{1} {
  NS_ASSERT_MSG(m_numRows * m_numCols == values.size(),
                "Dimensions and the initialization array size do not match.");
  m_values = std::move(values);
};

template <class T>
ValArray<T>::ValArray(size_t numRows, size_t numCols, size_t numPages,
                      const std::valarray<T> &values)
    : m_numRows{numRows}, m_numCols{numCols}, m_numPages{numPages},
      m_values{values} {
  NS_ASSERT_MSG(m_numRows * m_numCols * m_numPages == values.size(),
                "Dimensions and the initialization array size do not match.");
};

template <class T>
ValArray<T>::ValArray(size_t numRows, size_t numCols, size_t numPages,
                      std::valarray<T> &&values)
    : m_numRows{numRows}, m_numCols{numCols}, m_numPages{numPages} {
  NS_ASSERT_MSG(m_numRows * m_numCols * m_numPages == values.size(),
                "Dimensions and the initialization array size do not match.");
  m_values = std::move(values);
};

template <class T> bool ValArray<T>::operator==(const ValArray<T> &rhs) const {
  return EqualDims(rhs) && std::equal(std::begin(m_values), std::end(m_values),
                                      std::begin(rhs.m_values));
}

template <class T> bool ValArray<T>::operator!=(const ValArray<T> &rhs) const {
  return !((*this) == rhs);
}

template <class T>
bool ValArray<T>::IsAlmostEqual(const ValArray<T> &rhs, T tol) const {
  return EqualDims(rhs) &&
         std::equal(std::begin(m_values), std::end(m_values),
                    std::begin(rhs.m_values), [tol](T lhsValue, T rhsValue) {
                      return lhsValue == rhsValue ||
                             std::abs(lhsValue - rhsValue) <= std::abs(tol);
                    });
}

template <class T>
void ValArray<T>::AssertEqualDims(const ValArray<T> &rhs) const {
  NS_ASSERT_MSG(EqualDims(rhs), "Dimensions mismatch: "
                                "lhs (rows, cols, pages) = ("
                                    << m_numRows << ", " << m_numCols << ", "
                                    << m_numPages
                                    << ") and "
                                       "rhs (rows, cols, pages) = ("
                                    << rhs.m_numRows << ", " << rhs.m_numCols
                                    << ", " << rhs.m_numPages << ")");
}

template <class T>
std::ostream &operator<<(std::ostream &os, const ValArray<T> &a) {
  os << "\n";
  for (size_t p = 0; p != a.GetNumPages(); ++p) {
    os << "Page " << p << ":\n";
    for (size_t i = 0; i != a.GetNumRows(); ++i) {
      for (size_t j = 0; j != a.GetNumCols(); ++j) {
        os << "\t" << a(i, j, p);
      }
      os << "\n";
    }
  }
  return os;
}

} // namespace ns3

#endif
