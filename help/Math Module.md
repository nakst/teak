## Permutations

```c
// Create an list of length n with values [0, 1, 2, 3, ..., n - 1].
int[] PermutationFirst(int n);

// Get the next permutation of length n. After the n!th call, this will return false.
bool PermutationIterate(int[] p);
```

## Integer factorization 

// Get a list of integers that multiply to give x.
// Each integer in the list is either -1 or a prime number >= 2.
// If -1 occurs in the list, it will be the first element.
int[] IntegerPrimeFactorization(int x);

// Compute the greatest common divisor of a and b.
// a and b cannot both be zero. The returned result is always >= 1.
// a and b are coprime iff the result is 1.
int IntegerGCD(int a, int b);

// Compute the least common multiple of a and b.
// a and b cannot both be zero. The returned result is always >= 0.
int IntegerLCM(int a, int b);

// Runs the extended Euclidean algorithm on a and b, returning gcd, ca, cb.
// gcd is the GCD as returned by IntegerGCD. ca and cb satisfy: gcd = ca * a + cb * b.
tuple[int, int, int] IntegerExtendedEuclidean(int a, int b);

// Get the integer solutions to the equations x = r[i] (mod m[i]).
// Returns x, M. The solutions are of the form x + k * M for all integers k.
tuple[int, int] IntegerCRT(int[] r, int[] m);

## Rational numbers

TODO
