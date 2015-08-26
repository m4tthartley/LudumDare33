cd ../build
#opengl32.lib SDL2.lib -link -SUBSYSTEM:WINDOWS

# Why do you have to use -F ? Why doesn't -framework ../lib/SDL2 work? :(
clang -g -O0 ../src/ld33.c -o ld33 -Wno-null-dereference -F../lib -framework SDL2 -framework OpenGL
cd ../src