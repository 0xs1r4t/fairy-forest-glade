# get this file up and running with `CMake` + `vcpkg`

```pwsh
cmake -S . -B build -A x64 -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows
cmake --build build --config Release
.\build\Release\fairy_forest_glade.exe
```

# getting this file up and running

```pwsh
cd src
g++ main.cpp -o main -I C:\\glfw-3.4.bin.WIN64\\glfw-3.4.bin.WIN64\\include -L C:\\glfw-3.4.bin.WIN64\\glfw-3.4.bin.WIN64\\lib-mingw-w64 -I C:\\glew-2.1.0\\include -L C:\\glew-2.1.0\\lib\\Release\\x64 -I C:\\vcpkg\\installed\\x64-windows\\include -L C:\\vcpkg\\installed\\x64-windows\\bin -lassimp -lglfw3dll -lglew32 -lopengl32
./main.exe
```

## getting all my files in one text file

```pwsh
cd src
dir_print . --omit assets --export files.txt
```

## cleaning up the build

```pwsh
cd build
cmake --build . --config Release --clean-first
```
