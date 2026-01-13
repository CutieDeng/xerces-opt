# Generated Makefile for macosx
# Compiler: /opt/homebrew/bin/g++-15
# Shared library extension: .dylib
# Platform-specific flags:  -undefined dynamic_lookup

.PHONY: all clean prepare test-xercese test-simple-ptr-field test-simple-virtual-call test-simple-ptr-copy-escape test-bound-check test-bound-read test-malloc-size test-ownership-transfer test-phi-ice test-trivial-assignment test-simple-ptr-field-lto test-simple-virtual-call-lto test-simple-ptr-copy-escape-lto test-bound-check-lto test-bound-read-lto test-malloc-size-lto test-ownership-transfer-lto test-phi-ice-lto test-trivial-assignment-lto test-lto test-xercese-lto

all: out/plugin-array-detect.dylib

out/plugin-array-detect.dylib: obj/ad-core/array-detector.o obj/ad-core/field-assignment-debugger.o obj/ad-core/type-field-hash.o obj/ad-core/virtual-call-analysis.o obj/ad-context/address-resolver.o obj/ad-context/array-detect-context-gcc.o obj/ad-context/context-init.o obj/ad-context/context.o obj/ad-context/state.o obj/ad-gcc-integration/context-init-gcc.o obj/ad-gcc-integration/gcc-ext-util.o obj/ad-gcc-integration/lto-summary.o obj/ad-gcc-integration/plugin-top.o obj/ad-utils/info-print.o obj/ad-utils/string-utils.o obj/ad-field-write/field-write.o obj/ad-write-source/write-source.o obj/ad-source-use-info/source-use-info.o obj/ad-transfer-collect/transfer-collect.o obj/ad-transfer-stats/transfer-stats.o obj/ad-source-escape-use-info/source-escape-use-info.o obj/ad-source-escape-conclude/source-escape-conclude.o obj/ad-field-escape-conclude/field-escape-conclude.o obj/ad-field-wrapper/field-wrapper.o obj/ad-own-conclude/own-conclude.o obj/ad-result-output/result-output.o obj/ad-malloc-capacity/malloc-capacity.o obj/ad-malloc-group/malloc-group.o obj/ad-array-read-collect/array-read-collect.o obj/ad-array-read-bound/array-read-bound.o obj/ad-read-group/read-group.o obj/ad-array-write-collect/array-write-collect.o obj/ad-array-write-bound/array-write-bound.o obj/ad-write-group/write-group.o obj/ad-capacity-conclude/capacity-conclude.o obj/ad-pipeline/pipeline.o obj/ad-driver/driver.o
	/opt/homebrew/bin/g++-15 "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "obj/ad-core/array-detector.o" "obj/ad-core/field-assignment-debugger.o" "obj/ad-core/type-field-hash.o" "obj/ad-core/virtual-call-analysis.o" "obj/ad-context/address-resolver.o" "obj/ad-context/array-detect-context-gcc.o" "obj/ad-context/context-init.o" "obj/ad-context/context.o" "obj/ad-context/state.o" "obj/ad-gcc-integration/context-init-gcc.o" "obj/ad-gcc-integration/gcc-ext-util.o" "obj/ad-gcc-integration/lto-summary.o" "obj/ad-gcc-integration/plugin-top.o" "obj/ad-utils/info-print.o" "obj/ad-utils/string-utils.o" "obj/ad-field-write/field-write.o" "obj/ad-write-source/write-source.o" "obj/ad-source-use-info/source-use-info.o" "obj/ad-transfer-collect/transfer-collect.o" "obj/ad-transfer-stats/transfer-stats.o" "obj/ad-source-escape-use-info/source-escape-use-info.o" "obj/ad-source-escape-conclude/source-escape-conclude.o" "obj/ad-field-escape-conclude/field-escape-conclude.o" "obj/ad-field-wrapper/field-wrapper.o" "obj/ad-own-conclude/own-conclude.o" "obj/ad-result-output/result-output.o" "obj/ad-malloc-capacity/malloc-capacity.o" "obj/ad-malloc-group/malloc-group.o" "obj/ad-array-read-collect/array-read-collect.o" "obj/ad-array-read-bound/array-read-bound.o" "obj/ad-read-group/read-group.o" "obj/ad-array-write-collect/array-write-collect.o" "obj/ad-array-write-bound/array-write-bound.o" "obj/ad-write-group/write-group.o" "obj/ad-capacity-conclude/capacity-conclude.o" "obj/ad-pipeline/pipeline.o" "obj/ad-driver/driver.o" -o "out/plugin-array-detect.dylib"

obj/ad-core/array-detector.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-core/array-detector.cc" -o "obj/ad-core/array-detector.o"

obj/ad-core/field-assignment-debugger.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-core/field-assignment-debugger.cc" -o "obj/ad-core/field-assignment-debugger.o"

obj/ad-core/type-field-hash.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-core/type-field-hash.cc" -o "obj/ad-core/type-field-hash.o"

obj/ad-core/virtual-call-analysis.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-core/virtual-call-analysis.cc" -o "obj/ad-core/virtual-call-analysis.o"

obj/ad-context/address-resolver.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-context/address-resolver.cc" -o "obj/ad-context/address-resolver.o"

obj/ad-context/array-detect-context-gcc.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-context/array-detect-context-gcc.cc" -o "obj/ad-context/array-detect-context-gcc.o"

obj/ad-context/context-init.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-context/context-init.cc" -o "obj/ad-context/context-init.o"

obj/ad-context/context.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-context/context.cc" -o "obj/ad-context/context.o"

obj/ad-context/state.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-context/state.cc" -o "obj/ad-context/state.o"

obj/ad-gcc-integration/context-init-gcc.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-gcc-integration/context-init-gcc.cc" -o "obj/ad-gcc-integration/context-init-gcc.o"

obj/ad-gcc-integration/gcc-ext-util.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-gcc-integration/gcc-ext-util.cc" -o "obj/ad-gcc-integration/gcc-ext-util.o"

obj/ad-gcc-integration/lto-summary.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-gcc-integration/lto-summary.cc" -o "obj/ad-gcc-integration/lto-summary.o"

obj/ad-gcc-integration/plugin-top.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-gcc-integration/plugin-top.cc" -o "obj/ad-gcc-integration/plugin-top.o"

obj/ad-utils/info-print.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-utils/info-print.cc" -o "obj/ad-utils/info-print.o"

obj/ad-utils/string-utils.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-utils/string-utils.cc" -o "obj/ad-utils/string-utils.o"

obj/ad-field-write/field-write.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-field-write/field-write.cc" -o "obj/ad-field-write/field-write.o"

obj/ad-write-source/write-source.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-write-source/write-source.cc" -o "obj/ad-write-source/write-source.o"

obj/ad-source-use-info/source-use-info.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-source-use-info/source-use-info.cc" -o "obj/ad-source-use-info/source-use-info.o"

obj/ad-transfer-collect/transfer-collect.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-transfer-collect/transfer-collect.cc" -o "obj/ad-transfer-collect/transfer-collect.o"

obj/ad-transfer-stats/transfer-stats.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-transfer-stats/transfer-stats.cc" -o "obj/ad-transfer-stats/transfer-stats.o"

obj/ad-source-escape-use-info/source-escape-use-info.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-source-escape-use-info/source-escape-use-info.cc" -o "obj/ad-source-escape-use-info/source-escape-use-info.o"

obj/ad-source-escape-conclude/source-escape-conclude.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-source-escape-conclude/source-escape-conclude.cc" -o "obj/ad-source-escape-conclude/source-escape-conclude.o"

obj/ad-field-escape-conclude/field-escape-conclude.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-field-escape-conclude/field-escape-conclude.cc" -o "obj/ad-field-escape-conclude/field-escape-conclude.o"

obj/ad-field-wrapper/field-wrapper.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-field-wrapper/field-wrapper.cc" -o "obj/ad-field-wrapper/field-wrapper.o"

obj/ad-own-conclude/own-conclude.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-own-conclude/own-conclude.cc" -o "obj/ad-own-conclude/own-conclude.o"

obj/ad-result-output/result-output.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-result-output/result-output.cc" -o "obj/ad-result-output/result-output.o"

obj/ad-malloc-capacity/malloc-capacity.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-malloc-capacity/malloc-capacity.cc" -o "obj/ad-malloc-capacity/malloc-capacity.o"

obj/ad-malloc-group/malloc-group.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-malloc-group/malloc-group.cc" -o "obj/ad-malloc-group/malloc-group.o"

obj/ad-array-read-collect/array-read-collect.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-array-read-collect/array-read-collect.cc" -o "obj/ad-array-read-collect/array-read-collect.o"

obj/ad-array-read-bound/array-read-bound.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-array-read-bound/array-read-bound.cc" -o "obj/ad-array-read-bound/array-read-bound.o"

obj/ad-read-group/read-group.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-read-group/read-group.cc" -o "obj/ad-read-group/read-group.o"

obj/ad-array-write-collect/array-write-collect.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-array-write-collect/array-write-collect.cc" -o "obj/ad-array-write-collect/array-write-collect.o"

obj/ad-array-write-bound/array-write-bound.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-array-write-bound/array-write-bound.cc" -o "obj/ad-array-write-bound/array-write-bound.o"

obj/ad-write-group/write-group.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-write-group/write-group.cc" -o "obj/ad-write-group/write-group.o"

obj/ad-capacity-conclude/capacity-conclude.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-capacity-conclude/capacity-conclude.cc" -o "obj/ad-capacity-conclude/capacity-conclude.o"

obj/ad-pipeline/pipeline.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-pipeline/pipeline.cc" -o "obj/ad-pipeline/pipeline.o"

obj/ad-driver/driver.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/ad-core" "-I" "include/ad-context" "-I" "include/ad-gcc-integration" "-I" "include/ad-utils" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-transfer-collect" "-I" "include/ad-transfer-stats" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-own-conclude" "-I" "include/ad-result-output" "-I" "include/ad-malloc-capacity" "-I" "include/ad-malloc-group" "-I" "include/ad-array-read-collect" "-I" "include/ad-array-read-bound" "-I" "include/ad-read-group" "-I" "include/ad-array-write-collect" "-I" "include/ad-array-write-bound" "-I" "include/ad-write-group" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-driver/driver.cc" -o "obj/ad-driver/driver.o"

obj/ad-core/array-detector.o: \
  src/ad-core/array-detector.cc \
  include/ad-utils/prelude.hh \
  include/ad-core/array-detector.hh \
  include/ad-context/context.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-utils/info.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-context/array-detect-context-gcc-interface.hh \
  include/ad-field-write/field-write.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-context/address-resolver.hh \
  include/ad-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-core/type-field-hash-traits.hh \
  include/ad-core/type-field-hashmap-traits.hh

obj/ad-core/field-assignment-debugger.o: \
  src/ad-core/field-assignment-debugger.cc \
  include/ad-utils/prelude.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-core/field-assignment-debugger.hh \
  include/ad-context/context.hh \
  include/ad-context/array-detect-context-gcc-interface.hh \
  include/ad-core/array-detector.hh \
  include/ad-utils/info.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-field-write/field-write.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-context/address-resolver.hh \
  include/ad-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-core/type-field-hash-traits.hh \
  include/ad-core/type-field-hashmap-traits.hh \
  include/ad-gcc-integration/gcc-ext-util.hh \
  include/ad-context/context-init.hh

obj/ad-core/type-field-hash.o: \
  src/ad-core/type-field-hash.cc \
  include/ad-core/type-field-hash.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-core/array-detector.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/context.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-utils/info.hh \
  include/ad-context/array-detect-context-gcc-interface.hh \
  include/ad-field-write/field-write.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-context/address-resolver.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-core/type-field-hash-traits.hh \
  include/ad-core/type-field-hashmap-traits.hh

obj/ad-core/virtual-call-analysis.o: \
  src/ad-core/virtual-call-analysis.cc \
  include/ad-core/virtual-call-analysis.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-context/context.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-context/address-resolver.hh \
  include/ad-gcc-integration/gcc-ext-util.hh \
  include/ad-context/array-detect-context-gcc-interface.hh \
  include/ad-utils/info-print.hh \
  include/ad-field-write/field-write.hh \
  include/ad-write-source/write-source.hh

obj/ad-context/address-resolver.o: \
  src/ad-context/address-resolver.cc \
  include/ad-context/address-resolver.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/context.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-gcc-integration/gcc-common.hh

obj/ad-context/array-detect-context-gcc.o: \
  src/ad-context/array-detect-context-gcc.cc \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/context.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-context/address-resolver.hh \
  include/ad-context/array-detect-context-gcc-interface.hh

obj/ad-context/context-init.o: \
  src/ad-context/context-init.cc \
  include/ad-utils/prelude.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-context/context.hh \
  include/ad-context/context-init.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-context/address-resolver.hh \
  include/ad-context/array-detect-context-gcc-interface.hh

obj/ad-context/context.o: \
  src/ad-context/context.cc \
  include/ad-context/context.hh

obj/ad-context/state.o: \
  src/ad-context/state.cc \
  include/ad-context/state.hh \
  config/array-detect-state.txt

obj/ad-gcc-integration/context-init-gcc.o: \
  src/ad-gcc-integration/context-init-gcc.cc \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-context/context.hh \
  include/ad-context/context-init.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-context/address-resolver.hh \
  include/ad-context/array-detect-context-gcc-interface.hh

obj/ad-gcc-integration/gcc-ext-util.o: \
  src/ad-gcc-integration/gcc-ext-util.cc \
  include/ad-gcc-integration/gcc-ext-util.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-context/context.hh \
  include/ad-context/array-detect-context-gcc-interface.hh \
  include/ad-utils/info.hh \
  include/ad-core/array-detector.hh \
  include/ad-field-write/field-write.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-context/address-resolver.hh \
  include/ad-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-core/type-field-hash-traits.hh \
  include/ad-core/type-field-hashmap-traits.hh

obj/ad-gcc-integration/lto-summary.o: \
  src/ad-gcc-integration/lto-summary.cc \
  include/ad-gcc-integration/lto-summary.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/context.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-own-conclude/own-conclude.hh \
  include/ad-core/array-detector.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-utils/info.hh \
  include/ad-context/array-detect-context-gcc-interface.hh \
  include/ad-field-write/field-write.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-context/address-resolver.hh \
  include/ad-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-core/type-field-hash-traits.hh \
  include/ad-core/type-field-hashmap-traits.hh \
  include/ad-write-source/write-source.hh \
  include/ad-core/virtual-call-analysis.hh \
  include/ad-utils/string-utils.hh \
  include/ad-gcc-integration/gcc-ext-util.hh

obj/ad-gcc-integration/plugin-top.o: \
  src/ad-gcc-integration/plugin-top.cc \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-context/context.hh \
  include/ad-context/context-init.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-context/address-resolver.hh \
  include/ad-core/array-detector.hh \
  include/ad-utils/info.hh \
  include/ad-context/array-detect-context-gcc-interface.hh \
  include/ad-field-write/field-write.hh \
  include/ad-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-core/type-field-hash-traits.hh \
  include/ad-core/type-field-hashmap-traits.hh \
  include/ad-pipeline/pipeline.hh \
  include/ad-gcc-integration/lto-summary.hh \
  include/ad-own-conclude/own-conclude.hh \
  include/ad-write-source/write-source.hh \
  include/ad-core/virtual-call-analysis.hh \
  include/ad-result-output/result-output.hh \
  include/ad-gcc-integration/gcc-ext-util.hh \
  include/ad-utils/info-print.hh

obj/ad-utils/info-print.o: \
  src/ad-utils/info-print.cc \
  include/ad-utils/info-print.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-context/context.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-field-write/field-write.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-context/address-resolver.hh \
  include/ad-write-source/write-source.hh \
  include/ad-core/virtual-call-analysis.hh \
  include/ad-core/array-detector.hh \
  include/ad-utils/info.hh \
  include/ad-context/array-detect-context-gcc-interface.hh \
  include/ad-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-core/type-field-hash-traits.hh \
  include/ad-core/type-field-hashmap-traits.hh \
  include/ad-gcc-integration/gcc-ext-util.hh

obj/ad-utils/string-utils.o: \
  src/ad-utils/string-utils.cc \
  include/ad-utils/string-utils.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/context.hh \
  include/ad-gcc-integration/gcc-ext-util.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-context/array-detect-context-gcc-interface.hh

obj/ad-field-write/field-write.o: \
  src/ad-field-write/field-write.cc \
  include/ad-field-write/field-write.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-context/context.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-context/address-resolver.hh \
  include/ad-core/array-detector.hh \
  include/ad-utils/info.hh \
  include/ad-context/array-detect-context-gcc-interface.hh \
  include/ad-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-core/type-field-hash-traits.hh \
  include/ad-core/type-field-hashmap-traits.hh \
  include/ad-gcc-integration/gcc-ext-util.hh

obj/ad-write-source/write-source.o: \
  src/ad-write-source/write-source.cc \
  include/ad-write-source/write-source.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-utils/prelude.hh \
  include/ad-core/virtual-call-analysis.hh \
  include/ad-context/context.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-context/address-resolver.hh \
  include/ad-core/array-detector.hh \
  include/ad-utils/info.hh \
  include/ad-context/array-detect-context-gcc-interface.hh \
  include/ad-field-write/field-write.hh \
  include/ad-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-core/type-field-hash-traits.hh \
  include/ad-core/type-field-hashmap-traits.hh \
  include/ad-gcc-integration/gcc-ext-util.hh \
  include/ad-utils/info-print.hh

obj/ad-source-use-info/source-use-info.o: \
  src/ad-source-use-info/source-use-info.cc \
  include/ad-source-use-info/source-use-info.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-context/context.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-context/address-resolver.hh \
  include/ad-field-write/field-write.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-core/array-detector.hh \
  include/ad-utils/info.hh \
  include/ad-context/array-detect-context-gcc-interface.hh \
  include/ad-core/type-field-hash.hh \
  include/ad-core/type-field-hash-traits.hh \
  include/ad-core/type-field-hashmap-traits.hh \
  include/ad-utils/info-print.hh \
  include/ad-write-source/write-source.hh \
  include/ad-core/virtual-call-analysis.hh

obj/ad-transfer-collect/transfer-collect.o: \
  src/ad-transfer-collect/transfer-collect.cc \
  include/ad-transfer-collect/transfer-collect.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-context/context.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-context/address-resolver.hh \
  include/ad-field-write/field-write.hh \
  include/ad-write-source/write-source.hh \
  include/ad-core/virtual-call-analysis.hh \
  include/ad-gcc-integration/gcc-ext-util.hh \
  include/ad-context/array-detect-context-gcc-interface.hh \
  include/ad-utils/info-print.hh

obj/ad-transfer-stats/transfer-stats.o: \
  src/ad-transfer-stats/transfer-stats.cc \
  include/ad-transfer-stats/transfer-stats.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-context/context.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-context/address-resolver.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-transfer-collect/transfer-collect.hh \
  include/ad-field-write/field-write.hh \
  include/ad-write-source/write-source.hh \
  include/ad-core/virtual-call-analysis.hh

obj/ad-source-escape-use-info/source-escape-use-info.o: \
  src/ad-source-escape-use-info/source-escape-use-info.cc \
  include/ad-source-escape-use-info/source-escape-use-info.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-context/context.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-context/address-resolver.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-source-use-info/source-use-info.hh \
  include/ad-field-write/field-write.hh \
  include/ad-utils/info-print.hh \
  include/ad-write-source/write-source.hh \
  include/ad-core/virtual-call-analysis.hh

obj/ad-source-escape-conclude/source-escape-conclude.o: \
  src/ad-source-escape-conclude/source-escape-conclude.cc \
  include/ad-source-escape-conclude/source-escape-conclude.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-context/context.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-context/address-resolver.hh \
  include/ad-source-use-info/source-use-info.hh \
  include/ad-field-write/field-write.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-source-escape-use-info/source-escape-use-info.hh \
  include/ad-utils/info-print.hh \
  include/ad-write-source/write-source.hh \
  include/ad-core/virtual-call-analysis.hh

obj/ad-field-escape-conclude/field-escape-conclude.o: \
  src/ad-field-escape-conclude/field-escape-conclude.cc \
  include/ad-field-escape-conclude/field-escape-conclude.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-context/context.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-context/address-resolver.hh \
  include/ad-source-use-info/source-use-info.hh \
  include/ad-field-write/field-write.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-source-escape-use-info/source-escape-use-info.hh \
  include/ad-source-escape-conclude/source-escape-conclude.hh \
  include/ad-write-source/write-source.hh \
  include/ad-core/virtual-call-analysis.hh \
  include/ad-utils/info-print.hh

obj/ad-field-wrapper/field-wrapper.o: \
  src/ad-field-wrapper/field-wrapper.cc \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-context/context.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/address-resolver.hh

obj/ad-own-conclude/own-conclude.o: \
  src/ad-own-conclude/own-conclude.cc \
  include/ad-own-conclude/own-conclude.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/context.hh \
  include/ad-core/array-detector.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-utils/info.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-context/array-detect-context-gcc-interface.hh \
  include/ad-field-write/field-write.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-context/address-resolver.hh \
  include/ad-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-core/type-field-hash-traits.hh \
  include/ad-core/type-field-hashmap-traits.hh \
  include/ad-write-source/write-source.hh \
  include/ad-core/virtual-call-analysis.hh \
  include/ad-gcc-integration/gcc-ext-util.hh \
  include/ad-source-use-info/source-use-info.hh \
  include/ad-source-escape-conclude/source-escape-conclude.hh \
  include/ad-transfer-collect/transfer-collect.hh \
  include/ad-utils/info-print.hh \
  include/ad-utils/string-utils.hh

obj/ad-result-output/result-output.o: \
  src/ad-result-output/result-output.cc \
  include/ad-result-output/result-output.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/context.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-gcc-integration/lto-summary.hh \
  include/ad-own-conclude/own-conclude.hh \
  include/ad-core/array-detector.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-utils/info.hh \
  include/ad-context/array-detect-context-gcc-interface.hh \
  include/ad-field-write/field-write.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-context/address-resolver.hh \
  include/ad-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-core/type-field-hash-traits.hh \
  include/ad-core/type-field-hashmap-traits.hh \
  include/ad-write-source/write-source.hh \
  include/ad-core/virtual-call-analysis.hh \
  include/ad-utils/string-utils.hh \
  include/ad-gcc-integration/gcc-ext-util.hh \
  include/ad-malloc-capacity/malloc-capacity.hh \
  include/ad-array-read-bound/array-read-bound.hh \
  include/ad-array-read-collect/array-read-collect.hh \
  include/ad-array-write-bound/array-write-bound.hh \
  include/ad-array-write-collect/array-write-collect.hh

obj/ad-malloc-capacity/malloc-capacity.o: \
  src/ad-malloc-capacity/malloc-capacity.cc \
  include/ad-malloc-capacity/malloc-capacity.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/context.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-field-write/field-write.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-context/address-resolver.hh \
  include/ad-write-source/write-source.hh \
  include/ad-core/virtual-call-analysis.hh \
  include/ad-gcc-integration/gcc-ext-util.hh \
  include/ad-context/array-detect-context-gcc-interface.hh \
  include/ad-utils/string-utils.hh

obj/ad-malloc-group/malloc-group.o: \
  src/ad-malloc-group/malloc-group.cc \
  include/ad-malloc-group/malloc-group.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/context.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-field-write/field-write.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-context/address-resolver.hh \
  include/ad-write-source/write-source.hh \
  include/ad-core/virtual-call-analysis.hh \
  include/ad-malloc-capacity/malloc-capacity.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-gcc-integration/gcc-ext-util.hh \
  include/ad-context/array-detect-context-gcc-interface.hh \
  include/ad-utils/string-utils.hh

obj/ad-array-read-collect/array-read-collect.o: \
  src/ad-array-read-collect/array-read-collect.cc \
  include/ad-array-read-collect/array-read-collect.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/context.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-context/address-resolver.hh \
  include/ad-core/array-detector.hh \
  include/ad-utils/info.hh \
  include/ad-context/array-detect-context-gcc-interface.hh \
  include/ad-field-write/field-write.hh \
  include/ad-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-core/type-field-hash-traits.hh \
  include/ad-core/type-field-hashmap-traits.hh \
  include/ad-gcc-integration/gcc-ext-util.hh \
  include/ad-utils/string-utils.hh

obj/ad-array-read-bound/array-read-bound.o: \
  src/ad-array-read-bound/array-read-bound.cc \
  include/ad-array-read-bound/array-read-bound.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/context.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-context/address-resolver.hh \
  include/ad-array-read-collect/array-read-collect.hh \
  include/ad-core/array-detector.hh \
  include/ad-utils/info.hh \
  include/ad-context/array-detect-context-gcc-interface.hh \
  include/ad-field-write/field-write.hh \
  include/ad-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-core/type-field-hash-traits.hh \
  include/ad-core/type-field-hashmap-traits.hh \
  include/ad-gcc-integration/gcc-ext-util.hh \
  include/ad-utils/string-utils.hh

obj/ad-read-group/read-group.o: \
  src/ad-read-group/read-group.cc \
  include/ad-read-group/read-group.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/context.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-array-read-collect/array-read-collect.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-context/address-resolver.hh \
  include/ad-core/array-detector.hh \
  include/ad-utils/info.hh \
  include/ad-context/array-detect-context-gcc-interface.hh \
  include/ad-field-write/field-write.hh \
  include/ad-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-core/type-field-hash-traits.hh \
  include/ad-core/type-field-hashmap-traits.hh \
  include/ad-array-read-bound/array-read-bound.hh \
  include/ad-gcc-integration/gcc-ext-util.hh \
  include/ad-utils/string-utils.hh

obj/ad-array-write-collect/array-write-collect.o: \
  src/ad-array-write-collect/array-write-collect.cc \
  include/ad-array-write-collect/array-write-collect.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/context.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-context/address-resolver.hh \
  include/ad-core/array-detector.hh \
  include/ad-utils/info.hh \
  include/ad-context/array-detect-context-gcc-interface.hh \
  include/ad-field-write/field-write.hh \
  include/ad-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-core/type-field-hash-traits.hh \
  include/ad-core/type-field-hashmap-traits.hh \
  include/ad-gcc-integration/gcc-ext-util.hh \
  include/ad-utils/string-utils.hh

obj/ad-array-write-bound/array-write-bound.o: \
  src/ad-array-write-bound/array-write-bound.cc \
  include/ad-array-write-bound/array-write-bound.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/context.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-context/address-resolver.hh \
  include/ad-array-write-collect/array-write-collect.hh \
  include/ad-core/array-detector.hh \
  include/ad-utils/info.hh \
  include/ad-context/array-detect-context-gcc-interface.hh \
  include/ad-field-write/field-write.hh \
  include/ad-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-core/type-field-hash-traits.hh \
  include/ad-core/type-field-hashmap-traits.hh \
  include/ad-gcc-integration/gcc-ext-util.hh \
  include/ad-utils/string-utils.hh

obj/ad-write-group/write-group.o: \
  src/ad-write-group/write-group.cc \
  include/ad-write-group/write-group.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/context.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-array-write-collect/array-write-collect.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-context/address-resolver.hh \
  include/ad-core/array-detector.hh \
  include/ad-utils/info.hh \
  include/ad-context/array-detect-context-gcc-interface.hh \
  include/ad-field-write/field-write.hh \
  include/ad-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-core/type-field-hash-traits.hh \
  include/ad-core/type-field-hashmap-traits.hh \
  include/ad-array-write-bound/array-write-bound.hh \
  include/ad-gcc-integration/gcc-ext-util.hh \
  include/ad-utils/string-utils.hh

obj/ad-capacity-conclude/capacity-conclude.o: \
  src/ad-capacity-conclude/capacity-conclude.cc \
  include/ad-capacity-conclude/capacity-conclude.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/context.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-context/address-resolver.hh \
  include/ad-malloc-capacity/malloc-capacity.hh \
  include/ad-field-write/field-write.hh \
  include/ad-write-source/write-source.hh \
  include/ad-core/virtual-call-analysis.hh \
  include/ad-array-read-bound/array-read-bound.hh \
  include/ad-array-read-collect/array-read-collect.hh \
  include/ad-core/array-detector.hh \
  include/ad-utils/info.hh \
  include/ad-context/array-detect-context-gcc-interface.hh \
  include/ad-core/type-field-hash.hh \
  include/ad-core/type-field-hash-traits.hh \
  include/ad-core/type-field-hashmap-traits.hh \
  include/ad-array-write-bound/array-write-bound.hh \
  include/ad-array-write-collect/array-write-collect.hh \
  include/ad-utils/string-utils.hh \
  include/ad-gcc-integration/gcc-ext-util.hh

obj/ad-pipeline/pipeline.o: \
  src/ad-pipeline/pipeline.cc \
  include/ad-pipeline/pipeline.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/context.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-field-write/field-write.hh \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-context/address-resolver.hh \
  include/ad-write-source/write-source.hh \
  include/ad-core/virtual-call-analysis.hh \
  include/ad-source-use-info/source-use-info.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-source-escape-use-info/source-escape-use-info.hh \
  include/ad-source-escape-conclude/source-escape-conclude.hh \
  include/ad-field-escape-conclude/field-escape-conclude.hh \
  include/ad-transfer-collect/transfer-collect.hh \
  include/ad-transfer-stats/transfer-stats.hh \
  include/ad-core/array-detector.hh \
  include/ad-utils/info.hh \
  include/ad-context/array-detect-context-gcc-interface.hh \
  include/ad-core/type-field-hash.hh \
  include/ad-core/type-field-hash-traits.hh \
  include/ad-core/type-field-hashmap-traits.hh \
  include/ad-utils/info-print.hh \
  include/ad-malloc-capacity/malloc-capacity.hh \
  include/ad-malloc-group/malloc-group.hh \
  include/ad-array-read-collect/array-read-collect.hh \
  include/ad-array-read-bound/array-read-bound.hh \
  include/ad-read-group/read-group.hh \
  include/ad-array-write-collect/array-write-collect.hh \
  include/ad-array-write-bound/array-write-bound.hh \
  include/ad-write-group/write-group.hh \
  include/ad-capacity-conclude/capacity-conclude.hh \
  include/ad-own-conclude/own-conclude.hh \
  include/ad-gcc-integration/lto-summary.hh \
  include/ad-result-output/result-output.hh

obj/ad-driver/driver.o: \
  src/ad-driver/driver.cc \
  include/ad-driver/driver.hh \
  include/ad-utils/prelude.hh \
  include/ad-context/context.hh \
  include/ad-context/state.hh \
  config/array-detect-state.txt \
  include/ad-gcc-integration/gcc-common.hh \
  include/ad-field-write/field-write.hh \
  include/ad-context/array-detect-context-gcc.hh \
  include/ad-context/address-resolver.hh \
  include/ad-write-source/write-source.hh \
  include/ad-core/virtual-call-analysis.hh \
  include/ad-source-use-info/source-use-info.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-source-escape-conclude/source-escape-conclude.hh \
  include/ad-transfer-collect/transfer-collect.hh \
  include/ad-core/array-detector.hh \
  include/ad-utils/info.hh \
  include/ad-context/array-detect-context-gcc-interface.hh \
  include/ad-core/type-field-hash.hh \
  include/ad-core/type-field-hash-traits.hh \
  include/ad-core/type-field-hashmap-traits.hh \
  include/ad-source-escape-use-info/source-escape-use-info.hh \
  include/ad-utils/info-print.hh

clean:
	rm -rf obj out

test-xercese: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\"))" | racket test-script/test-xercese.rkt

prepare:
	mkdir -p obj/ad-core
	mkdir -p obj/ad-context
	mkdir -p obj/ad-gcc-integration
	mkdir -p obj/ad-utils
	mkdir -p obj/ad-field-write
	mkdir -p obj/ad-write-source
	mkdir -p obj/ad-source-use-info
	mkdir -p obj/ad-transfer-collect
	mkdir -p obj/ad-transfer-stats
	mkdir -p obj/ad-source-escape-use-info
	mkdir -p obj/ad-source-escape-conclude
	mkdir -p obj/ad-field-escape-conclude
	mkdir -p obj/ad-field-wrapper
	mkdir -p obj/ad-own-conclude
	mkdir -p obj/ad-result-output
	mkdir -p obj/ad-malloc-capacity
	mkdir -p obj/ad-malloc-group
	mkdir -p obj/ad-array-read-collect
	mkdir -p obj/ad-array-read-bound
	mkdir -p obj/ad-read-group
	mkdir -p obj/ad-array-write-collect
	mkdir -p obj/ad-array-write-bound
	mkdir -p obj/ad-write-group
	mkdir -p obj/ad-capacity-conclude
	mkdir -p obj/ad-pipeline
	mkdir -p obj/ad-driver
	mkdir -p out

test-simple-ptr-field: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\"))" | racket test-script/test-simple-ptr-field.rkt

test-simple-virtual-call: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\"))" | racket test-script/test-simple-virtual-call.rkt

test-simple-ptr-copy-escape: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\"))" | racket test-script/test-simple-ptr-copy-escape.rkt

test-bound-check: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\"))" | racket test-script/test-bound-check.rkt

test-bound-read: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\"))" | racket test-script/test-bound-read.rkt

test-malloc-size: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\"))" | racket test-script/test-malloc-size.rkt

test-ownership-transfer: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\"))" | racket test-script/test-ownership-transfer.rkt

test-phi-ice: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\"))" | racket test-script/test-phi-ice.rkt

test-trivial-assignment: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\"))" | racket test-script/test-trivial-assignment.rkt

test-simple-ptr-field-lto: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\" \"-flto\" \"-flto-compression-level=0\"))" | racket test-script/test-simple-ptr-field.rkt

test-simple-virtual-call-lto: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\" \"-flto\" \"-flto-compression-level=0\"))" | racket test-script/test-simple-virtual-call.rkt

test-simple-ptr-copy-escape-lto: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\" \"-flto\" \"-flto-compression-level=0\"))" | racket test-script/test-simple-ptr-copy-escape.rkt

test-bound-check-lto: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\" \"-flto\" \"-flto-compression-level=0\"))" | racket test-script/test-bound-check.rkt

test-bound-read-lto: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\" \"-flto\" \"-flto-compression-level=0\"))" | racket test-script/test-bound-read.rkt

test-malloc-size-lto: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\" \"-flto\" \"-flto-compression-level=0\"))" | racket test-script/test-malloc-size.rkt

test-ownership-transfer-lto: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\" \"-flto\" \"-flto-compression-level=0\"))" | racket test-script/test-ownership-transfer.rkt

test-phi-ice-lto: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\" \"-flto\" \"-flto-compression-level=0\"))" | racket test-script/test-phi-ice.rkt

test-trivial-assignment-lto: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\" \"-flto\" \"-flto-compression-level=0\"))" | racket test-script/test-trivial-assignment.rkt

test-lto: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\" \"-flto\" \"-flto-compression-level=0\"))" | racket test-script/test-lto.rkt

test-xercese-lto: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\" \"-flto\" \"-flto-compression-level=0\"))" | racket test-script/test-xercese.rkt

