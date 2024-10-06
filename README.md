#### _Significant portion of the code is inspired/improvised from `Mastering Algorithms with C (1999) Kyle Loudon`._

All routines are implemented purely in C `(C23)` as static functions in headers inside `./include/`.     
Testing uses Google's `gtest`, hence requires a C++ compiler, ideally one with support for `C++20` or modern standards,
`C++14` and `C++17` standards can be made to work with minor refactors (the designated initializers (`C++20`) 
and terse `static_asserts` (`C++17`) will have to go.