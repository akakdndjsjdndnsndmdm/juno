# juno
A blazing fast scripting language aimed for safety and simplicity.

# Features
* Bytecode virtual machine ([JNVM](https://github.com/tracyxmr/jnvm))
* Variables, simple declarations `let x = 5;`
* Expressions and literals
* Type safety, static typing to catch errors before runtime.
* Memory safe, native safeguards to mitigate against common issues.

# Quick Start
Installation
```bash
# clone the repo
git clone https://github.com/tracyxmr/juno.git
cd juno

# build from source
./build.sh
```

# Examples and Explanations
```
let x = 10;

{
    let y = 15;
}

let z = 20;
```

Juno has a global scope like the large majority of programming languages, each scope in Juno has a predefined start register for any variables declared in that scope. When the compiler leaves the scope, the compiler which take note of the exiting of the scope and overwrite these registers as they're no longer in use. This is one way Juno cleans up variables.

# Roadmap
* [x] Bytecode VM
* [x] Variables and expressions
* [ ] Functions and closures (Juno has a native print function)
* [ ] Standard Library
* [ ] Language Server
* [ ] Performance optimizations
* [ ] Detailed documentation

# Building from Source
### Prerequisites
* C++17 or higher
* CMake 3.15 or higher
* A C++ compiler ( GCC, Clang or MSVC )
* Git

### Steps
```bash
# clone the repo
git clone https://github.com/tracyxmr/juno.git
cd juno

# create a build directory
mkdir build && cd build

# configure cmake
cmake ..

# build for debug
cmake --build .

# OR

# build for release
cmake --build . --config Release
```

# Support
If you encounter any issues or have questions, please open an issue on GitHub.