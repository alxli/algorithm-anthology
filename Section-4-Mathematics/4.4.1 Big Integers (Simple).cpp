/*

4.4.1 - Big Integers (Simple)

Description: Integer arbitrary precision functions.
To use, pass bigints to the functions by addresses.
e.g. add(&a, &b, &c) stores the sum of a and b into c.

Complexity: comp(), to_string(), digit_shift(), add(),
and sub() are O(N) on the number of digits. mul() and
div() are O(N^2). zero_justify() is amortized constant.

*/

#include <string>

struct bigint {
  static const int maxdigits = 1000;

  char dig[maxdigits], sign;
  int last;

  bigint(long long x = 0): sign(x < 0 ? -1 : 1) {
    for (int i = 0; i < maxdigits; i++) dig[i] = 0;
    if (x == 0) { last = 0; return; }
    if (x < 0) x = -x;
    for (last = -1; x > 0; x /= 10) dig[++last] = x % 10;
  }

  bigint(const std::string & s): sign(s[0] == '-' ? -1 : 1) {
    for (int i = 0; i < maxdigits; i++) dig[i] = 0;
    last = -1;
    for (int i = s.size() - 1; i >= 0; i--)
      dig[++last] = (s[i] - '0');
    if (dig[last] + '0' == '-') dig[last--] = 0;
  }
};

void zero_justify(bigint * x) {
  while (x->last > 0 && !x->dig[x->last]) x->last--;
  if (x->last == 0 && x->dig[0] == 0) x->sign = 1;
}

void add(bigint * a, bigint * b, bigint * c);
void sub(bigint * a, bigint * b, bigint * c);

//returns: -1 if a < b, 0 if a == b, or 1 if a > b
int comp(bigint * a, bigint * b) {
  if (a->sign != b->sign) return b->sign;
  if (b->last > a->last) return a->sign;
  if (a->last > b->last) return -a->sign;
  for (int i = a->last; i >= 0; i--) {
    if (a->dig[i] > b->dig[i]) return -a->sign;
    if (b->dig[i] > a->dig[i]) return  a->sign;
  }
  return 0;
}

void add(bigint * a, bigint * b, bigint * c) {
  if (a->sign != b->sign) {
    if (a->sign == -1)
      a->sign = 1, sub(b, a, c), a->sign = -1;
    else
      b->sign = 1, sub(a, b, c), b->sign = -1;
    return;
  }
  c->sign = a->sign;
  c->last = (a->last > b->last ? a->last : b->last) + 1;
  for (int i = 0, carry = 0; i <= c->last; i++) {
    c->dig[i] = (carry + a->dig[i] + b->dig[i]) % 10;
    carry = (carry + a->dig[i] + b->dig[i]) / 10;
  }
  zero_justify(c);
}

void sub(bigint * a, bigint * b, bigint * c) {
  if (a->sign == -1 || b->sign == -1) {
    b->sign *= -1, add(a, b, c), b->sign *= -1;
    return;
  }
  if (comp(a, b) == 1) {
    sub(b, a, c), c->sign = -1;
    return;
  }
  c->last = (a->last > b->last) ? a->last : b->last;
  for (int i = 0, borrow = 0, v; i <= c->last; i++) {
    v = a->dig[i] - borrow;
    if (i <= b->last) v -= b->dig[i];
    if (a->dig[i] > 0) borrow = 0;
    if (v < 0) v += 10, borrow = 1;
    c->dig[i] = v % 10;
  }
  zero_justify(c);
}

void digit_shift(bigint * x, int n) {
  if (!x->last && !x->dig[0]) return;
  for (int i = x->last; i >= 0; i--)
    x->dig[i + n] = x->dig[i];
  for (int i = 0; i < n; i++) x->dig[i] = 0;
  x->last += n;
}

void mul(bigint * a, bigint * b, bigint * c) {
  bigint row = *a, tmp;
  for (int i = 0; i <= b->last; i++) {
    for (int j = 1; j <= b->dig[i]; j++) {
      add(c, &row, &tmp);
      *c = tmp;
    }
    digit_shift(&row, 1);
  }
  c->sign = a->sign * b->sign;
  zero_justify(c);
}

void div(bigint * a, bigint * b, bigint * c) {
  bigint row, tmp;
  int asign = a->sign, bsign = b->sign;
  a->sign = b->sign = 1;
  c->last = a->last;
  for (int i = a->last; i >= 0; i--) {
    digit_shift(&row, 1);
    row.dig[0] = a->dig[i];
    c->dig[i] = 0;
    for (; comp(&row, b) != 1; row = tmp) {
      c->dig[i]++;
      sub(&row, b, &tmp);
    }
  }
  c->sign = (a->sign = asign) * (b->sign = bsign);
  zero_justify(c);
}

std::string to_string(bigint * x) {
  std::string s(x->sign == -1 ? "-" : "");
  for (int i = x->last; i >= 0; i--)
    s += (char)('0' + x->dig[i]);
  return s;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  bigint a("-9899819294989142124"), b("12398124981294214");
  bigint sum; add(&a, &b, &sum);
  bigint dif; sub(&a, &b, &dif);
  bigint prd; mul(&a, &b, &prd);
  bigint quo; div(&a, &b, &quo);
  assert(to_string(&sum) == "-9887421170007847910");
  assert(to_string(&dif) == "-9912217419970436338");
  assert(to_string(&prd) == "-122739196911503356525379735104870536");
  assert(to_string(&quo) == "-798");
  return 0;
}
