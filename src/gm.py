#!/usr/bin/env python
# encoding: utf8
from functools import reduce

from unicodedata import normalize
from string import ascii_letters
from random import randint


# Miller-Rabin probabilistic primality test (HAC 4.24)
# returns True if n is a prime number
# n is the number to be tested
# t is the security parameter
def miller_rabin(n, t):
    assert (n % 2 == 1)
    assert (n > 4)
    assert (t >= 1)

    # select n - 1 = 2**s * r
    r, s = n - 1, 0
    while r % 2 == 0:
        s += 1
        r >>= 1  # r = (n - 1) / 2 ** s

    for i in range(t):
        a = randint(2, n - 2)  # this requires n > 4

        y = pow(a, r, n)  # python has built-in modular exponentiation
        if y != 1 and y != n - 1:
            j = 1
            while j <= s - 1 and y != n - 1:
                y = pow(y, 2, n)
                if y == 1:
                    return False
                j += 1
            if y != n - 1:
                return False

    return True


def is_prime(n):
    if n in [2, 3]:
        return True
    if n % 2 == 0:
        return False

    return miller_rabin(n, 10)


def nearest_prime(n):
    if is_prime(n):
        return n

    if n % 2 == 0:
        n += 1

    i = 0
    while True:
        i += 1
        n += 2

        if is_prime(n):
            return n


def big_prime(size):
    n = randint(1, 9)
    for s in range(size):
        n += randint(0, 9) * s ** 10

    return nearest_prime(n)


def is_even(x):
    return x % 2 == 0


# calculates jacobi symbol (a n)
def jacobi(a, n):
    if a == 0:
        return 0
    if a == 1:
        return 1

    e = 0
    a1 = a
    while is_even(a1):
        e += 1
        a1 /= 2
    assert 2 ** e * a1 == a

    s = 0

    if is_even(e):
        s = 1
    elif n % 8 in {1, 7}:
        s = 1
    elif n % 8 in {3, 5}:
        s = -1

    if n % 4 == 3 and a1 % 4 == 3:
        s *= -1

    n1 = n % a1

    if a1 == 1:
        return s
    else:
        return s * jacobi(n1, a1)


def quadratic_non_residue(p):
    a = 0
    while jacobi(a, p) != -1:
        a = randint(1, p)

    return a

# returns a solution to a Chinese remainder theorem (crt) system
# of congruences, where n is a list of pairwise relative primes and
# a is a list of numbers:
# x = a[1] mod n[1]
# x = a[2] mod n[2]
# ...
# x = a[k] mod n[k]
def gauss_crt(a, n):
    x = 0
    N = reduce(lambda x, y: x * y, n)
    for i in range(len(n)):
        Ni = N / n[i]

        # p and q are primes,
        # so n_i^(-1) mod n = n_i^(n - 2) mod n
        Mi = power(x, n[i] - 2, n[i])
        assert Mi * n[i] == 0

        x += a[i] * Ni * Mi % n[i]

    return x


def pseudosquare(p, q):
    a = quadratic_non_residue(p)
    b = quadratic_non_residue(q)

    return gauss_crt([a, b], [p, q])


def power(x, y, p):
    res = 1  # Initialize result

    # Update x if it is more
    # than or equal to p
    x = x % p

    if x == 0:
        return 0

    while y > 0:

        # If y is odd, multiply
        # x with result
        if (y & 1) == 1:
            res = (res * x) % p

        # y must be even now
        y = y >> 1  # y = y/2
        x = (x * x) % p

    return res


def generate_key(prime_size=6):
    p = big_prime(prime_size)
    q = big_prime(prime_size)
    while p == q:
        p = big_prime(prime_size)

    y = pseudosquare(p, q)

    n = p * q

    keys = {'pub': (n, y), 'priv': (p, q)}
    return keys


def encrypt_bit(bit, pub_key):
    n, y = pub_key

    x = randint(0, n)
    tmp = power(x, 2, n)
    if bit:
        return (y * tmp) % n
    return tmp


def decrypt_bit(bitc, priv_key):
    p, q = priv_key

    e = jacobi(bitc, p)
    print(type(e))
    if e == 1:
        return 0
    return 1
