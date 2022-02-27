OSX_MIN_VERSION=10.10
OSX_SDK_VERSION=10.11
OSX_SDK=$(SDK_PATH)/MacOSX$(OSX_SDK_VERSION).sdk
LD64_VERSION=253.9
darwin_CC=clang -target $(host) -mmacosx-version-min=$(OSX_MIN_VERSION) --sysroot $(OSX_SDK) -mlinker-version=$(LD64_VERSION)
darwin_CXX=clang++ -target $(host) -mmacosx-version-min=$(OSX_MIN_VERSION) --sysroot $(OSX_SDK) -mlinker-version=$(LD64_VERSION) -stdlib=libc++darwin_native_binutils=native_cctools
darwin_native_binutils=native_cctools

ifeq ($(strip $(FORCE_USE_SYSTEM_CLANG)),)
# FORCE_USE_SYSTEM_CLANG is empty, so we use our depends-managed, pinned clang
# from llvm.org

# Clang is a dependency of native_cctools when FORCE_USE_SYSTEM_CLANG is empty
darwin_native_toolchain=native_cctools

clang_prog=$(build_prefix)/bin/clang
clangxx_prog=$(clang_prog)++

clang_resource_dir=$(build_prefix)/lib/clang/$(native_clang_version)
else
# FORCE_USE_SYSTEM_CLANG is non-empty, so we use the clang from the user's
# system

darwin_native_toolchain=

darwin_CFLAGS=-pipe
darwin_CXXFLAGS=$(darwin_CFLAGS)

darwin_release_CFLAGS=-O2
darwin_release_CXXFLAGS=$(darwin_release_CFLAGS)

darwin_debug_CFLAGS=-O1
darwin_debug_CXXFLAGS=$(darwin_debug_CFLAGS)

darwin_cmake_system=Darwin
