# IPA project
Project for BUT FIT Advanced Assembly Languages course.
My implementation is in:
- `xhricma00.s` - algorithm optimization using AVX2 SIMD instructions
- `Ocean.cpp` - rewritten algorithm to eliminate repeated calculations, sin/cos lookup tables and memory layout conversion of `GerstnerWave` vector

# Used mathematical concepts
$f$ - float  
$i$ - int  
$N$ - lookup table size

## Range reduction
$$
f \mod 2\pi \Leftrightarrow f - 2\pi \cdot \left\lfloor \frac{f}{2\pi} \right\rfloor
$$

## Conversion to index
$$
i = \text{round} \left( \frac{f \cdot (N-1)}{2\pi} \right)
$$


