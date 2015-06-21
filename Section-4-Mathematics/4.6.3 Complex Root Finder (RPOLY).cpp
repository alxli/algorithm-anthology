/*

4.6.3 - Complex Root Finder (Jenkins-Traub Algorithm)

Determine the complex roots of a polynomial with real coefficients.
This is the variant of the Jenkins-Traub algorithm for polynomials
with real coefficient, known as RPOLY. RPOLY follows follows the
same pattern as the CPOLY algorithm, but computes two roots at a
time, either two real roots or a pair of conjugate complex roots.
See: https://en.wikipedia.org/wiki/Jenkins%E2%80%93Traub_algorithm

The following is a translation of TOMS493 (www.netlib.org/toms/)
from FORTRAN to C++, with a simple wrapper at the end for the C++
<complex> class. Although the code is not meant to be read, it is
extremely efficient and robust, capable of achieving at least an
accuracy of 5 decimal places for really laborious inputs.

*/

#include <cfloat>
#include <cmath>

typedef long double LD;

void divide_quadratic(int n, LD u, LD v, LD p[], LD q[], LD * a, LD * b) {
  q[0] = *b = p[0];
  q[1] = *a = -((*b) * u) + p[1];
  for (int i = 2; i < n; i++) {
    q[i] = -((*a) * u + (*b) * v) + p[i];
    *b = *a;
    *a = q[i];
  }
}

int get_flag(int n, LD a, LD b, LD * a1, LD * a3, LD * a7,
             LD * c, LD * d, LD * e, LD * f, LD * g, LD * h,
             LD k[], LD u, LD v, LD qk[]) {
  divide_quadratic(n, u, v, k, qk, c, d);
  if (fabsl(*c) <= 100.0 * LDBL_EPSILON * fabsl(k[n - 1]) &&
      fabsl(*d) <= 100.0 * LDBL_EPSILON * fabsl(k[n - 2])) return 3;
  *h = v * b;
  if (fabsl(*d) >= fabsl(*c)) {
    *e = a / (*d);
    *f = (*c) / (*d);
    *g = u * b;
    *a1 = -a + (*f) * b;
    *a3 = (*e) * ((*g) + a) + (*h) * (b / (*d));
    *a7 = (*h) + ((*f) + u) * a;
    return 2;
  }
  *e = a / (*c);
  *f = (*d) / (*c);
  *g = (*e) * u;
  *a1 = -(a * ((*d) / (*c))) + b;
  *a3 = (*e) * a + ((*g) + (*h)/(*c)) * b;
  *a7 = (*g) * (*d) + (*h) * (*f) + a;
  return 1;
}

void find_polynomials(int n, int flag, LD a, LD b, LD a1, LD * a3,
                      LD * a7, LD k[], LD qk[], LD qp[]) {
  if (flag == 3) {
    k[1] = k[0] = 0.0;
    for (int i = 2; i < n; i++) k[i] = qk[i - 2];
    return;
  }
  if (fabsl(a1) > 10.0 * LDBL_EPSILON * fabsl(flag == 1 ? b : a)) {
    *a7 /= a1;
    *a3 /= a1;
    k[0] = qp[0];
    k[1] = -((*a7) * qp[0]) + qp[1];
    for (int i = 2; i < n; i++)
      k[i] = -((*a7) * qp[i - 1]) + (*a3) * qk[i - 2] + qp[i];
  } else {
    k[0] = 0.0;
    k[1] = -(*a7) * qp[0];
    for (int i = 2; i < n; i++)
      k[i] = -((*a7) * qp[i - 1]) + (*a3) * qk[i - 2];
  }
}

void estimate_coeff(int flag, LD * uu, LD * vv, LD a, LD a1, LD a3, LD a7,
                    LD b, LD c, LD d, LD f, LD g, LD h, LD u, LD v, LD k[],
                    int n, LD p[]) {
  LD a4, a5, b1, b2, c1, c2, c3, c4, temp;
  (*vv) = (*uu) = 0.0;
  if (flag == 3) return;
  if (flag != 2) {
    a4 = a + u * b + h * f;
    a5 = c + (u + v * f) * d;
  } else {
    a4 = (a + g) * f + h;
    a5 = (f + u) * c + v * d;
  }
  b1 = -k[n - 1] / p[n];
  b2 = -(k[n - 2] + b1 * p[n - 1]) / p[n];
  c1 = v * b2 * a1;
  c2 = b1 * a7;
  c3 = b1 * b1 * a3;
  c4 = -(c2 + c3) + c1;
  temp = -c4 + a5 + b1 * a4;
  if (temp != 0.0) {
    *uu= -((u * (c3 + c2) + v * (b1 * a1 + b2 * a7)) / temp) + u;
    *vv = v * (1.0 + c4 / temp);
  }
}

void solve_quadratic(LD a, LD b1, LD c, LD * sr, LD * si, LD * lr, LD * li) {
  LD b, d, e;
  *sr = *si = *lr = *li = 0.0;
  if (a == 0) {
    *sr = (b1 != 0) ? -(c / b1) : *sr;
    return;
  }
  if (c == 0) {
    *lr = -b1 / a;
    return;
  }
  b = b1 / 2.0;
  if (fabsl(b) < fabsl(c)) {
    e = (c >= 0) ? a : -a;
    e = -e + b * (b / fabsl(c));
    d = sqrtl(fabsl(e)) * sqrtl(fabsl(c));
  } else {
    e = -((a / b) * (c / b)) + 1.0;
    d = sqrtl(fabsl(e)) * fabsl(b);
  }
  if (e >= 0) {
    d = (b >= 0) ? -d : d;
    *lr = (-b + d) / a;
    *sr = (*lr != 0) ? (c / *lr) / a : *sr;
  } else {
    *lr = *sr = -(b / a);
    *si = fabsl(d / a);
    *li = -(*si);
  }
}

void quadratic_iterate(int N, int * NZ, LD uu, LD vv,
                       LD * szr, LD * szi, LD * lzr, LD * lzi, LD qp[],
                       int n, LD * a, LD * b, LD p[], LD qk[],
                       LD * a1, LD * a3, LD * a7, LD * c, LD * d, LD * e,
                       LD * f, LD * g, LD * h, LD k[]) {
  int i, j = 0, flag, tried_flag = 0;
  LD ee, mp, omp, relstp, t, u, ui, v, vi, zm;
  *NZ = 0;
  u = uu;
  v = vv;
  do {
    solve_quadratic(1.0, u, v, szr, szi, lzr, lzi);
    if (fabsl(fabsl(*szr) - fabsl(*lzr)) > 0.01 * fabsl(*lzr)) break;
    divide_quadratic(n, u, v, p, qp, a, b);
    mp = fabsl(-((*szr) * (*b)) + *a) + fabsl((*szi) * (*b));
    zm = sqrtl(fabsl(v));
    ee = 2.0 * fabsl(qp[0]);
    t = -((*szr) * (*b));
    for (i = 1; i < N; i++) ee = ee * zm + fabsl(qp[i]);
    ee = ee * zm + fabsl(*a + t);
    ee = ee * 9.0 + 2.0 * fabsl(t) - 7.0 * (fabsl(*a + t) + zm * fabsl(*b));
    ee *= LDBL_EPSILON;
    if (mp <= 20.0 * ee) {
      *NZ = 2;
      break;
    }
    if (++j > 20) break;
    if (j >= 2 && relstp <= 0.01 && mp >= omp && !tried_flag) {
      relstp = (relstp < LDBL_EPSILON) ? sqrtl(LDBL_EPSILON) : sqrtl(relstp);
      u -= u * relstp;
      v += v * relstp;
      divide_quadratic(n, u, v, p, qp, a, b);
      for (i = 0; i < 5; i++) {
        flag = get_flag(N, *a, *b, a1, a3, a7, c, d, e, f, g, h, k, u, v, qk);
        find_polynomials(N, flag, *a, *b, *a1, a3, a7, k, qk, qp);
      }
      tried_flag = 1;
      j = 0;
    }
    omp = mp;
    flag = get_flag(N, *a, *b, a1, a3, a7, c, d, e, f, g, h, k, u, v, qk);
    find_polynomials(N, flag, *a, *b, *a1, a3, a7, k, qk, qp);
    flag = get_flag(N, *a, *b, a1, a3, a7, c, d, e, f, g, h, k, u, v, qk);
    estimate_coeff(flag, &ui, &vi, *a, *a1, *a3, *a7, *b, *c, *d, *f, *g, *h,
                   u, v, k, N, p);
    if (vi != 0) {
      relstp = fabsl((-v + vi) / vi);
      u = ui;
      v = vi;
    }
  } while (vi != 0);
}

void real_iterate(int * flag, int * nz, LD * sss, int n, LD p[],
                  int nn, LD qp[], LD * szr, LD * szi, LD k[], LD qk[]) {
  int i, j = 0;
  LD ee, kv, mp, ms, omp, pv, s, t;
  *flag = *nz = 0;
  for (s = *sss; ; s += t) {
    pv = p[0];
    qp[0] = pv;
    for (i = 1; i < nn; i++) qp[i] = pv = pv*s + p[i];
    mp = fabsl(pv);
    ms = fabsl(s);
    ee = 0.5 * fabsl(qp[0]);
    for (i = 1; i < nn; i++) ee = ee * ms + fabsl(qp[i]);
    if (mp <= 20.0 * LDBL_EPSILON * (2.0 * ee - mp)) {
      *nz = 1;
      *szr = s;
      *szi = 0.0;
      break;
    }
    if (++j > 10) break;
    if (j >= 2 && fabsl(t) <= 0.001 * fabsl(-t + s) && mp > omp) {
      *flag = 1;
      *sss = s;
      break;
    }
    omp = mp;
    qk[0] = kv = k[0];
    for (i = 1; i < n; i++) qk[i] = kv = kv * s + k[i];
    if (fabsl(kv) > fabsl(k[n - 1]) * 10.0 * LDBL_EPSILON) {
      t = -(pv / kv);
      for (k[0] = qp[0], i = 1; i < n; i++)
        k[i] = t * qk[i - 1] + qp[i];
    } else for (k[0] = 0.0, i = 1; i < n; i++) {
      k[i] = qk[i - 1];
    }
    kv = k[0];
    for (i = 1; i < n; i++) kv = kv * s + k[i];
    t = fabsl(kv) > (fabsl(k[n - 1]) * 10.0 * LDBL_EPSILON) ? -(pv / kv) : 0.0;
  }
}

void solve_fixedshift(int l2, int * nz, LD sr, LD v, LD k[], int n,
                      LD p[], int nn, LD qp[], LD u, LD qk[], LD svk[],
                      LD * lzi, LD * lzr, LD * szi, LD * szr) {
  int i, j, flag, _flag, __flag = 1, spass, stry, vpass, vtry;
  LD a, a1, a3, a7, b, betas, betav, c, d, e, f, g, h, oss, ots, otv, ovv;
  LD s, ss, ts, tss, tv, tvv, ui, vi, vv;
  *nz = 0;
  betav = betas = 0.25;
  oss = sr;
  ovv = v;
  divide_quadratic(nn, u, v, p, qp, &a, &b);
  flag = get_flag(n, a, b, &a1, &a3, &a7, &c, &d, &e, &f, &g, &h,
                  k, u, v, qk);
  for (j = 0; j < l2; j++) {
    _flag = 1;
    find_polynomials(n, flag, a, b, a1, &a3, &a7, k, qk, qp);
    flag = get_flag(n, a, b, &a1, &a3, &a7, &c, &d, &e, &f, &g, &h,
                    k, u, v, qk);
    estimate_coeff(flag, &ui, &vi, a, a1, a3, a7, b, c, d, f, g, h,
                   u, v, k, n, p);
    vv = vi;
    ss = k[n - 1] != 0.0 ? -(p[n] / k[n - 1]) : 0.0;
    ts = tv = 1.0;
    if ((j != 0) && (flag != 3)) {
      tv = (vv != 0.0) ? fabsl((vv - ovv) / vv) : tv;
      ts = (ss != 0.0) ? fabsl((ss - oss) / ss) : ts;
      tvv = (tv < otv) ? tv * otv : 1.0;
      tss = (ts < ots) ? ts * ots : 1.0;
      vpass = (tvv < betav) ? 1 : 0;
      spass = (tss < betas) ? 1 : 0;
      if (spass || vpass) {
        for (i = 0; i < n; i++) svk[i] = k[i];
        s = ss; stry = vtry = 0;
        while (true) {
          if (!((_flag && ((_flag = 0) == 0)) &&
              ((spass) && (!vpass || (tss < tvv))))) {
            quadratic_iterate(n, nz, ui, vi, szr, szi, lzr, lzi, qp, nn,
                  &a, &b, p, qk, &a1, &a3, &a7, &c, &d, &e, &f, &g, &h, k);
            if ((*nz) > 0) return;
            __flag = vtry = 1;
            betav *= 0.25;
            if (stry || !spass) {
              __flag = 0;
            } else {
              for (i = 0; i < n; i++) k[i] = svk[i];
            }
          }
          if (__flag != 0) {
            real_iterate(&__flag, nz, &s, n, p, nn, qp, szr, szi, k, qk);
            if ((*nz) > 0) return;
            stry = 1; betas *= 0.25;
            if (__flag != 0) {
              ui = -(s + s);
              vi = s * s;
              continue;
            }
          }
          for (i = 0; i < n; i++) k[i] = svk[i];
          if (!vpass || vtry) break;
        }
        divide_quadratic(nn, u, v, p, qp, &a, &b);
        flag = get_flag(n, a, b, &a1, &a3, &a7, &c, &d, &e, &f, &g, &h,
                        k, u, v, qk);
      }
    }
    ovv = vv;
    oss = ss;
    otv = tv;
    ots = ts;
  }
}

void find_roots(int degree, LD co[], LD re[], LD im[]) {
  int i, j, jj, l, n, NM1, nn, NZ, zero, SZ = degree + 1;
  LD k[SZ], p[SZ], pt[SZ], qp[SZ], temp[SZ], qk[SZ], svk[SZ];
  LD bnd, df, dx, factor, ff, moduli_max, moduli_min, sc, x, xm;
  LD aa, bb, cc, lzi, lzr, sr, szi, szr, t, u, xx, xxx, yy;
  n = degree;
  xx = sqrtl(0.5);
  yy = -xx;
  for (j = 0; co[n] == 0; n--, j++) re[j] = im[j] = 0.0;
  nn = n + 1;
  for (i = 0; i < nn; i++) p[i] = co[i];
  while (n >= 1) {
    if (n <= 2) {
      if (n < 2) {
        re[degree - 1] = -(p[1] / p[0]);
        im[degree - 1] = 0.0;
      } else {
        solve_quadratic(p[0], p[1], p[2], &re[degree - 2],
                        &im[degree - 2], &re[degree - 1], &im[degree - 1]);
      }
      break;
    }
    moduli_max = 0.0;
    moduli_min = LDBL_MAX;
    for (i = 0; i < nn; i++) {
      x = fabsl(p[i]);
      if (x > moduli_max) moduli_max = x;
      if ((x != 0) && x < moduli_min) moduli_min = x;
    }
    sc = LDBL_MIN / LDBL_EPSILON / moduli_min;
    if ((sc <= 1.0 && moduli_max >= 10) ||
        (sc > 1.0 && LDBL_MAX / sc >= moduli_max)) {
      sc = (sc == 0) ? LDBL_MIN : sc;
      l = (int)(logl(sc) / logl(2.0) + 0.5);
      factor = powl(2.0, l);
      if (factor != 1.0)
        for (i = 0; i < nn; i++) p[i] *= factor;
    }
    for (i = 0; i < nn; i++) pt[i] = fabsl(p[i]);
    pt[n] = -(pt[n]);
    NM1 = n - 1;
    x = expl((logl(-pt[n]) - logl(pt[0])) / (LD)n);
    if (pt[NM1] != 0) {
      xm = -pt[n] / pt[NM1];
      if (xm < x) x = xm;
    }
    xm = x;
    do {
      x = xm;
      xm = 0.1 * x;
      ff = pt[0];
      for (i = 1; i < nn; i++)
        ff = ff *xm + pt[i];
    } while (ff > 0);
    dx = x;
    do {
      df = ff = pt[0];
      for (i = 1; i < n; i++) {
        ff = x * ff + pt[i];
        df = x * df + ff;
      }
      ff = x*ff + pt[n];
      dx = ff / df;
      x -= dx;
    } while (fabsl(dx / x) > 0.005);
    for (bnd = x, i = 1; i < n; i++)
      k[i] = (LD)(n - i) * p[i] / (LD)n;
    k[0] = p[0];
    aa = p[n];
    bb = p[NM1];
    zero = (k[NM1] == 0) ? 1 : 0;
    for (jj = 0; jj < 5; jj++) {
      cc = k[NM1];
      if (zero) {
        for (i = 0; i < NM1; i++) {
          j = NM1 - i;
          k[j] = k[j - 1];
        }
        k[0] = 0;
        zero = (k[NM1] == 0) ? 1 : 0;
      } else {
        t = -aa / cc;
        for (i = 0; i < NM1; i++) {
          j = NM1 - i;
          k[j] = t * k[j - 1] + p[j];
        }
        k[0] = p[0];
        zero = (fabsl(k[NM1]) <= fabsl(bb) * LDBL_EPSILON * 10.0) ? 1 : 0;
      }
    }
    for (i = 0; i < n; i++) temp[i] = k[i];
    static const LD DEG = 0.01745329251994329576923690768489L;
    for (jj = 1; jj <= 20; jj++) {
      xxx = -sinl(94.0 * DEG) * yy + cosl(94.0 * DEG) * xx;
      yy = sinl(94.0 * DEG) * xx + cosl(94.0 * DEG) * yy;
      xx = xxx;
      sr = bnd * xx;
      u = -2.0 * sr;
      for (int i = 0; i < nn; i++) qk[i] = svk[i] = 0.0;
      solve_fixedshift(20 * jj, &NZ, sr, bnd, k, n, p, nn, qp, u,
                       qk, svk, &lzi, &lzr, &szi, &szr);
      if (NZ != 0) {
        j = degree - n;
        re[j] = szr;
        im[j] = szi;
        nn = nn - NZ;
        n = nn - 1;
        for (i = 0; i < nn; i++) p[i] = qp[i];
        if (NZ != 1) {
          re[j + 1] = lzr;
          im[j + 1] = lzi;
        }
        break;
      } else {
        for (i = 0; i < n; i++) k[i] = temp[i];
      }
    }
    if (jj > 20) break;
  }
}

/*** Wrapper ***/

#include <algorithm>
#include <complex>
#include <vector>

typedef std::complex<LD> root;

template <class T> bool comp(const root & a, const root & b) {
  if (real(a) != real(b)) return real(a) < real(b);
  return imag(a) < imag(b);
}

std::vector<root> find_roots(int degree, LD coefficients[]) {
  std::reverse(coefficients, coefficients + degree + 1);
  LD re[degree], im[degree];
  find_roots(degree, coefficients, re, im);
  std::vector<root> res;
  for (int i = 0; i < degree; i++)
    res.push_back(root(re[i], im[i]));
  std::sort(res.begin(), res.end(), comp<LD>);
  return res;
}

/*** Example Usage (http://wcipeg.com/problem/rootsolve) ***/

#include <iostream>
using namespace std;

int T, degree, p;
LD c, coeff[101];

int main() {
  degree = 0;
  cin >> T;
  for (int i = 0; i < T; i++) {
    cin >> c >> p;
    if (p > degree) degree = p;
    coeff[p] = c;
  }
  std::vector<root> roots = find_roots(degree, coeff);
  bool printed = false;
  cout.precision(6);
  for (int i = 0; i < roots.size(); i++) {
    if (fabsl(roots[i].imag()) < LDBL_EPSILON) {
      cout << fixed << roots[i].real() << "\n";
      printed = true;
    }
  }
  if (!printed) cout << "NO REAL ROOTS\n";
  return 0;
}
