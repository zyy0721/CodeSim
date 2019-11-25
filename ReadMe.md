# codesim 编译说明

### Softeware Requirements

```bash
$ sudo apt-get install llvm clang libllvm6.0 libclang-6.0-dev
$ sudo apt-get install cmake # >= 3.14
```



### Build

```bash
$sudo cmake CMakeLists.txt
$sudo make
```



### Test Example

```bash
$ codesim testcase/1.cpp testcase/2.cpp
42.37
```

