cd ../build
clang -g -O2 ../src/ld33.c -o ld33 -Wno-null-dereference -DBUILD_RELEASE -framework OpenGL ./libSDL2.dylib
cd ../src