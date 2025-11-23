# Create the build directory.
mkdir "bin"
# Cd into build, if this fails exit
cd "bin" || exit
# Configure cmake
cmake ..
# Build Juno for release
cmake --build . --config Release