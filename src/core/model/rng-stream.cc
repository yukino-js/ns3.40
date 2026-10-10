
#include "rng-stream.h"

#include "fatal-error.h"
#include "log.h"

#include <cstdlib>
#include <iostream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("RngStream");

}

namespace MRG32k3a {

// clang-format off

typedef double Matrix[3][3];

const double m1   =       4294967087.0;

const double m2   =       4294944443.0;

const double norm =       1.0 / (m1 + 1.0);

const double a12  =       1403580.0;

const double a13n =       810728.0;

const double a21  =       527612.0;

const double a23n =       1370589.0;

const double two17 =      131072.0;

const double two53 =      9007199254740992.0;

const Matrix A1p0 = {
  {       0.0,        1.0,       0.0 },
  {       0.0,        0.0,       1.0 },
  { -810728.0,  1403580.0,       0.0 }
};

const Matrix A2p0 = {
  {        0.0,        1.0,       0.0 },
  {        0.0,        0.0,       1.0 },
  { -1370589.0,        0.0,  527612.0 }
};


double MultModM (double a, double s, double c, double m)
{
  double v;
  int32_t a1;

  v = a * s + c;

  if (v >= two53 || v <= -two53)
    {
      a1 = static_cast<int32_t> (a / two17);
      a -= a1 * two17;
      v  = a1 * s;
      a1 = static_cast<int32_t> (v / m);
      v -= a1 * m;
      v = v * two17 + a * s + c;
    }

  a1 = static_cast<int32_t> (v / m);
  if ((v -= a1 * m) < 0.0)
    {
      return v += m;
    }
  else
    {
      return v;
    }
}


void MatVecModM (const Matrix A, const double s[3], double v[3],
                 double m)
{
  int i;
  double x[3];

  for (i = 0; i < 3; ++i)
    {
      x[i] = MultModM (A[i][0], s[0], 0.0, m);
      x[i] = MultModM (A[i][1], s[1], x[i], m);
      x[i] = MultModM (A[i][2], s[2], x[i], m);
    }
  for (i = 0; i < 3; ++i)
    {
      v[i] = x[i];
    }
}


void MatMatModM (const Matrix A, const Matrix B,
                 Matrix C, double m)
{
  int i;
  int j;
  double V[3];
  Matrix W;

  for (i = 0; i < 3; ++i)
    {
      for (j = 0; j < 3; ++j)
        {
          V[j] = B[j][i];
        }
      MatVecModM (A, V, V, m);
      for (j = 0; j < 3; ++j)
        {
          W[j][i] = V[j];
        }
    }
  for (i = 0; i < 3; ++i)
    {
      for (j = 0; j < 3; ++j)
        {
          C[i][j] = W[i][j];
        }
    }
}


void MatTwoPowModM (const Matrix src, Matrix dst, double m, int32_t e)
{
  int i;
  int j;

  for (i = 0; i < 3; ++i)
    {
      for (j = 0; j < 3; ++j)
        {
          dst[i][j] = src[i][j];
        }
    }
  for (i = 0; i < e; i++)
    {
      MatMatModM (dst, dst, dst, m);
    }
}


void MatPowModM (const double A[3][3], double B[3][3], double m, int32_t n)
{
  int i;
  int j;
  double W[3][3];

  for (i = 0; i < 3; ++i)
    {
      for (j = 0; j < 3; ++j)
        {
          W[i][j] = A[i][j];
          B[i][j] = 0.0;
        }
    }
  for (j = 0; j < 3; ++j)
    {
      B[j][j] = 1.0;
    }

  while (n > 0)
    {
      if (n % 2)
        {
          MatMatModM (W, B, B, m);
        }
      MatMatModM (W, W, W, m);
      n /= 2;
    }
}

struct Precalculated
{
  Matrix a1[190];
  Matrix a2[190];
};

Precalculated PowerOfTwoConstants ()
{
  Precalculated precalculated;
  for (int i = 0; i < 190; i++)
    {
      int power = i + 1;
      MatTwoPowModM (A1p0, precalculated.a1[i], m1, power);
      MatTwoPowModM (A2p0, precalculated.a2[i], m2, power);
    }
  return precalculated;
}
void PowerOfTwoMatrix (int n, Matrix a1p, Matrix a2p)
{
  static  Precalculated constants = PowerOfTwoConstants ();
  for (int i = 0; i < 3; i ++)
    {
      for (int j = 0; j < 3; j++)
        {
          a1p[i][j] = constants.a1[n-1][i][j];
          a2p[i][j] = constants.a2[n-1][i][j];
        }
    }
}

}

// clang-format on

namespace ns3 {

using namespace MRG32k3a;

double RngStream::RandU01() {
  int32_t k;
  double p1;
  double p2;
  double u;

  p1 = a12 * m_currentState[1] - a13n * m_currentState[0];
  k = static_cast<int32_t>(p1 / m1);
  p1 -= k * m1;
  if (p1 < 0.0) {
    p1 += m1;
  }
  m_currentState[0] = m_currentState[1];
  m_currentState[1] = m_currentState[2];
  m_currentState[2] = p1;

  p2 = a21 * m_currentState[5] - a23n * m_currentState[3];
  k = static_cast<int32_t>(p2 / m2);
  p2 -= k * m2;
  if (p2 < 0.0) {
    p2 += m2;
  }
  m_currentState[3] = m_currentState[4];
  m_currentState[4] = m_currentState[5];
  m_currentState[5] = p2;

  u = ((p1 > p2) ? (p1 - p2) * norm : (p1 - p2 + m1) * norm);

  return u;
}

RngStream::RngStream(uint32_t seedNumber, uint64_t stream, uint64_t substream) {
  if (seedNumber >= m1 || seedNumber >= m2 || seedNumber == 0) {
    NS_FATAL_ERROR("invalid Seed " << seedNumber);
  }
  for (int i = 0; i < 6; ++i) {
    m_currentState[i] = seedNumber;
  }
  AdvanceNthBy(stream, 127, m_currentState);
  AdvanceNthBy(substream, 76, m_currentState);
}

RngStream::RngStream(const RngStream &r) {
  for (int i = 0; i < 6; ++i) {
    m_currentState[i] = r.m_currentState[i];
  }
}

void RngStream::AdvanceNthBy(uint64_t nth, int by, double state[6]) {
  Matrix matrix1;
  Matrix matrix2;
  for (int i = 0; i < 64; i++) {
    int nbit = 63 - i;
    int bit = (nth >> nbit) & 0x1;
    if (bit) {
      PowerOfTwoMatrix(by + nbit, matrix1, matrix2);
      MatVecModM(matrix1, state, state, m1);
      MatVecModM(matrix2, &state[3], &state[3], m2);
    }
  }
}

} // namespace ns3
