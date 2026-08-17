# ChH1dd3n3r

Advanced file steganography and encryption tool.

## Building

### Dependencies
- OpenSSL (libcrypto)
- zlib

### Linux
```bash
sudo apt install libssl-dev zlib1g-dev
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### macOS
```bash
brew install openssl zlib
cmake -B build -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl -DZLIB_ROOT=/usr/local/opt/zlib -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Windows (vcpkg)
```bash
vcpkg install openssl zlib
cmake -B build -DCMAKE_TOOLCHAIN_FILE=<vcpkg-root>/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

## Usage
```
./chh1dd3n3r --help
```
