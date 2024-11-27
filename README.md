All routines are implemented purely in C `(C23)` as static functions in headers inside `./include/`.    
 
Testing uses Google's `GoogleTest 1.15.2` (included in `./tests/googletest/`) and tests require a C++ compiler with C++20 support.   
Tests for routines in the `C` headers are implemented in the `C++` source files with the same name in the `./tests/` directory.    
Media files (`bronze.jpg`, `middlemarch.txt`, `mobydick.txt`, `plato.txt`, `sqlite3.dll` & `synth.bin`) used for testing and benchmarking are located in `./media/`   

The image was downloaded from [wallpaperswide](https://wallpaperswide.com). All the text files are books downloaded from [Project Guttenberg](https://www.gutenberg.org).
And the DLL was from a `Python` archive and the binary file is a serialized synthetic `Numpy` array.
    

Reference implementation : `Mastering Algorithms with C (1999) Kyle Loudon`