# Generated Makefile for macosx
# Compiler: /opt/homebrew/bin/g++-15
# Shared library extension: .dylib
# Platform-specific flags:  -undefined dynamic_lookup

.PHONY: all clean prepare test-xercese test-simple-ptr-field test-simple-virtual-call test-simple-ptr-copy-escape test-bound-check test-bound-read test-malloc-size test-ownership-transfer test-phi-ice test-trivial-assignment test-simple-ptr-field-lto test-simple-virtual-call-lto test-simple-ptr-copy-escape-lto test-bound-check-lto test-bound-read-lto test-malloc-size-lto test-ownership-transfer-lto test-phi-ice-lto test-trivial-assignment-lto test-lto test-xercese-lto

all: out/plugin-array-detect.dylib

out/plugin-array-detect.dylib: obj/array-detect-core/array-detector.o obj/array-detect-core/field-assignment-debugger.o obj/array-detect-core/type-field-hash.o obj/array-detect-core/virtual-call-analysis.o obj/array-detect-context/address-resolver.o obj/array-detect-context/array-detect-context-gcc.o obj/array-detect-context/context-init.o obj/array-detect-context/context.o obj/array-detect-context/state.o obj/array-detect-gcc-integration/context-init-gcc.o obj/array-detect-gcc-integration/gcc-ext-util.o obj/array-detect-gcc-integration/lto-summary.o obj/array-detect-gcc-integration/plugin-top.o obj/array-detect-utils/info-print.o obj/array-detect-utils/string-utils.o obj/array-detect-lto-transform/lto-transform.o obj/ad-field-write/field-write.o obj/ad-write-source/write-source.o obj/ad-source-use-info/source-use-info.o obj/ad-ownership-move/ownership-move.o obj/ad-ownership-conclude/ownership-conclude.o obj/ad-source-escape-use-info/source-escape-use-info.o obj/ad-source-escape-conclude/source-escape-conclude.o obj/ad-field-escape-conclude/field-escape-conclude.o obj/ad-field-wrapper/field-wrapper.o obj/ad-owned-verdict/owned-conclusion.o obj/ad-capacity-associator/capacity-association.o obj/ad-array-access/array-access-collector.o obj/ad-bound-condition/bound-condition-analyzer.o obj/ad-result-aggregator/result-aggregator.o obj/ad-field-write-capacity/field-write-capacity.o obj/ad-array-read-capacity/array-read-capacity.o obj/ad-array-write-capacity/array-write-capacity.o obj/ad-capacity-conclude/capacity-conclude.o obj/ad-pipeline/pipeline.o obj/ad-driver/driver.o
	/opt/homebrew/bin/g++-15 "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-field-write-capacity" "-I" "include/ad-array-read-capacity" "-I" "include/ad-array-write-capacity" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "obj/array-detect-core/array-detector.o" "obj/array-detect-core/field-assignment-debugger.o" "obj/array-detect-core/type-field-hash.o" "obj/array-detect-core/virtual-call-analysis.o" "obj/array-detect-context/address-resolver.o" "obj/array-detect-context/array-detect-context-gcc.o" "obj/array-detect-context/context-init.o" "obj/array-detect-context/context.o" "obj/array-detect-context/state.o" "obj/array-detect-gcc-integration/context-init-gcc.o" "obj/array-detect-gcc-integration/gcc-ext-util.o" "obj/array-detect-gcc-integration/lto-summary.o" "obj/array-detect-gcc-integration/plugin-top.o" "obj/array-detect-utils/info-print.o" "obj/array-detect-utils/string-utils.o" "obj/array-detect-lto-transform/lto-transform.o" "obj/ad-field-write/field-write.o" "obj/ad-write-source/write-source.o" "obj/ad-source-use-info/source-use-info.o" "obj/ad-ownership-move/ownership-move.o" "obj/ad-ownership-conclude/ownership-conclude.o" "obj/ad-source-escape-use-info/source-escape-use-info.o" "obj/ad-source-escape-conclude/source-escape-conclude.o" "obj/ad-field-escape-conclude/field-escape-conclude.o" "obj/ad-field-wrapper/field-wrapper.o" "obj/ad-owned-verdict/owned-conclusion.o" "obj/ad-capacity-associator/capacity-association.o" "obj/ad-array-access/array-access-collector.o" "obj/ad-bound-condition/bound-condition-analyzer.o" "obj/ad-result-aggregator/result-aggregator.o" "obj/ad-field-write-capacity/field-write-capacity.o" "obj/ad-array-read-capacity/array-read-capacity.o" "obj/ad-array-write-capacity/array-write-capacity.o" "obj/ad-capacity-conclude/capacity-conclude.o" "obj/ad-pipeline/pipeline.o" "obj/ad-driver/driver.o" -o "out/plugin-array-detect.dylib"

obj/array-detect-core/array-detector.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-core/array-detector.cc" -o "obj/array-detect-core/array-detector.o"

obj/array-detect-core/field-assignment-debugger.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-core/field-assignment-debugger.cc" -o "obj/array-detect-core/field-assignment-debugger.o"

obj/array-detect-core/type-field-hash.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-core/type-field-hash.cc" -o "obj/array-detect-core/type-field-hash.o"

obj/array-detect-core/virtual-call-analysis.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-core/virtual-call-analysis.cc" -o "obj/array-detect-core/virtual-call-analysis.o"

obj/array-detect-context/address-resolver.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-context/address-resolver.cc" -o "obj/array-detect-context/address-resolver.o"

obj/array-detect-context/array-detect-context-gcc.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-context/array-detect-context-gcc.cc" -o "obj/array-detect-context/array-detect-context-gcc.o"

obj/array-detect-context/context-init.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-context/context-init.cc" -o "obj/array-detect-context/context-init.o"

obj/array-detect-context/context.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-context/context.cc" -o "obj/array-detect-context/context.o"

obj/array-detect-context/state.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-context/state.cc" -o "obj/array-detect-context/state.o"

obj/array-detect-gcc-integration/context-init-gcc.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-gcc-integration/context-init-gcc.cc" -o "obj/array-detect-gcc-integration/context-init-gcc.o"

obj/array-detect-gcc-integration/gcc-ext-util.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-gcc-integration/gcc-ext-util.cc" -o "obj/array-detect-gcc-integration/gcc-ext-util.o"

obj/array-detect-gcc-integration/lto-summary.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-gcc-integration/lto-summary.cc" -o "obj/array-detect-gcc-integration/lto-summary.o"

obj/array-detect-gcc-integration/plugin-top.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-gcc-integration/plugin-top.cc" -o "obj/array-detect-gcc-integration/plugin-top.o"

obj/array-detect-utils/info-print.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-utils/info-print.cc" -o "obj/array-detect-utils/info-print.o"

obj/array-detect-utils/string-utils.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-utils/string-utils.cc" -o "obj/array-detect-utils/string-utils.o"

obj/array-detect-lto-transform/lto-transform.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-lto-transform/lto-transform.cc" -o "obj/array-detect-lto-transform/lto-transform.o"

obj/ad-field-write/field-write.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-field-write/field-write.cc" -o "obj/ad-field-write/field-write.o"

obj/ad-write-source/write-source.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-write-source/write-source.cc" -o "obj/ad-write-source/write-source.o"

obj/ad-source-use-info/source-use-info.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-source-use-info/source-use-info.cc" -o "obj/ad-source-use-info/source-use-info.o"

obj/ad-ownership-move/ownership-move.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-ownership-move/ownership-move.cc" -o "obj/ad-ownership-move/ownership-move.o"

obj/ad-ownership-conclude/ownership-conclude.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-ownership-conclude/ownership-conclude.cc" -o "obj/ad-ownership-conclude/ownership-conclude.o"

obj/ad-source-escape-use-info/source-escape-use-info.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-source-escape-use-info/source-escape-use-info.cc" -o "obj/ad-source-escape-use-info/source-escape-use-info.o"

obj/ad-source-escape-conclude/source-escape-conclude.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-source-escape-conclude/source-escape-conclude.cc" -o "obj/ad-source-escape-conclude/source-escape-conclude.o"

obj/ad-field-escape-conclude/field-escape-conclude.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-field-escape-conclude/field-escape-conclude.cc" -o "obj/ad-field-escape-conclude/field-escape-conclude.o"

obj/ad-field-wrapper/field-wrapper.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-field-wrapper/field-wrapper.cc" -o "obj/ad-field-wrapper/field-wrapper.o"

obj/ad-owned-verdict/owned-conclusion.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-owned-verdict/owned-conclusion.cc" -o "obj/ad-owned-verdict/owned-conclusion.o"

obj/ad-capacity-associator/capacity-association.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-capacity-associator/capacity-association.cc" -o "obj/ad-capacity-associator/capacity-association.o"

obj/ad-array-access/array-access-collector.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-array-access/array-access-collector.cc" -o "obj/ad-array-access/array-access-collector.o"

obj/ad-bound-condition/bound-condition-analyzer.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-bound-condition/bound-condition-analyzer.cc" -o "obj/ad-bound-condition/bound-condition-analyzer.o"

obj/ad-result-aggregator/result-aggregator.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-result-aggregator/result-aggregator.cc" -o "obj/ad-result-aggregator/result-aggregator.o"

obj/ad-pipeline/pipeline.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-field-write-capacity" "-I" "include/ad-array-read-capacity" "-I" "include/ad-array-write-capacity" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-pipeline/pipeline.cc" -o "obj/ad-pipeline/pipeline.o"

obj/ad-driver/driver.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-field-write-capacity" "-I" "include/ad-array-read-capacity" "-I" "include/ad-array-write-capacity" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-driver/driver.cc" -o "obj/ad-driver/driver.o"

obj/ad-field-write-capacity/field-write-capacity.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-field-write-capacity" "-I" "include/ad-array-read-capacity" "-I" "include/ad-array-write-capacity" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-field-write-capacity/field-write-capacity.cc" -o "obj/ad-field-write-capacity/field-write-capacity.o"

obj/ad-array-read-capacity/array-read-capacity.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-field-write-capacity" "-I" "include/ad-array-read-capacity" "-I" "include/ad-array-write-capacity" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-array-read-capacity/array-read-capacity.cc" -o "obj/ad-array-read-capacity/array-read-capacity.o"

obj/ad-array-write-capacity/array-write-capacity.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-field-write-capacity" "-I" "include/ad-array-read-capacity" "-I" "include/ad-array-write-capacity" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-array-write-capacity/array-write-capacity.cc" -o "obj/ad-array-write-capacity/array-write-capacity.o"

obj/ad-capacity-conclude/capacity-conclude.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-lto-transform" "-I" "include/ad-field-write" "-I" "include/ad-write-source" "-I" "include/ad-source-use-info" "-I" "include/ad-ownership-move" "-I" "include/ad-ownership-conclude" "-I" "include/ad-source-escape-use-info" "-I" "include/ad-source-escape-conclude" "-I" "include/ad-field-escape-conclude" "-I" "include/ad-field-wrapper" "-I" "include/ad-owned-verdict" "-I" "include/ad-capacity-associator" "-I" "include/ad-array-access" "-I" "include/ad-bound-condition" "-I" "include/ad-result-aggregator" "-I" "include/ad-field-write-capacity" "-I" "include/ad-array-read-capacity" "-I" "include/ad-array-write-capacity" "-I" "include/ad-capacity-conclude" "-I" "include/ad-pipeline" "-I" "include/ad-driver" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-capacity-conclude/capacity-conclude.cc" -o "obj/ad-capacity-conclude/capacity-conclude.o"

obj/array-detect-core/array-detector.o: \
  src/array-detect-core/array-detector.cc \
  include/array-detect-utils/prelude.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-context/context.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-utils/info.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/ad-field-write/field-write.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-context/address-resolver.hh \
  include/array-detect-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh

obj/array-detect-core/field-assignment-debugger.o: \
  src/array-detect-core/field-assignment-debugger.cc \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-core/field-assignment-debugger.hh \
  include/array-detect-context/context.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-utils/info.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/ad-field-write/field-write.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-context/address-resolver.hh \
  include/array-detect-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/array-detect-gcc-integration/gcc-ext-util.hh \
  include/array-detect-context/context-init.hh

obj/array-detect-core/type-field-hash.o: \
  src/array-detect-core/type-field-hash.cc \
  include/array-detect-core/type-field-hash.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/context.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-utils/info.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/ad-field-write/field-write.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-context/address-resolver.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh

obj/array-detect-core/virtual-call-analysis.o: \
  src/array-detect-core/virtual-call-analysis.cc \
  include/array-detect-core/virtual-call-analysis.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/context.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-context/address-resolver.hh \
  include/array-detect-gcc-integration/gcc-ext-util.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/array-detect-utils/info-print.hh \
  include/ad-field-write/field-write.hh \
  include/ad-write-source/write-source.hh

obj/array-detect-context/address-resolver.o: \
  src/array-detect-context/address-resolver.cc \
  include/array-detect-context/address-resolver.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/context.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-gcc-integration/gcc-common.hh

obj/array-detect-context/array-detect-context-gcc.o: \
  src/array-detect-context/array-detect-context-gcc.cc \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/context.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/address-resolver.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh

obj/array-detect-context/context-init.o: \
  src/array-detect-context/context-init.cc \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-context/context.hh \
  include/array-detect-context/context-init.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/address-resolver.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh

obj/array-detect-context/context.o: \
  src/array-detect-context/context.cc \
  include/array-detect-context/context.hh

obj/array-detect-context/state.o: \
  src/array-detect-context/state.cc \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt

obj/array-detect-gcc-integration/context-init-gcc.o: \
  src/array-detect-gcc-integration/context-init-gcc.cc \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/context.hh \
  include/array-detect-context/context-init.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-context/address-resolver.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh

obj/array-detect-gcc-integration/gcc-ext-util.o: \
  src/array-detect-gcc-integration/gcc-ext-util.cc \
  include/array-detect-gcc-integration/gcc-ext-util.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-context/context.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/array-detect-utils/info.hh \
  include/array-detect-core/array-detector.hh \
  include/ad-field-write/field-write.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-context/address-resolver.hh \
  include/array-detect-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh

obj/array-detect-gcc-integration/lto-summary.o: \
  src/array-detect-gcc-integration/lto-summary.cc \
  include/array-detect-gcc-integration/lto-summary.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/context.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/ad-owned-verdict/owned-conclusion.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-utils/info.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/ad-field-write/field-write.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-context/address-resolver.hh \
  include/array-detect-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/ad-write-source/write-source.hh \
  include/array-detect-core/virtual-call-analysis.hh \
  include/ad-result-aggregator/result-aggregator.hh \
  include/ad-capacity-associator/capacity-association.hh \
  include/ad-array-access/array-access-collector.hh \
  include/ad-bound-condition/bound-condition-analyzer.hh

obj/array-detect-gcc-integration/plugin-top.o: \
  src/array-detect-gcc-integration/plugin-top.cc \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-context/context.hh \
  include/array-detect-context/context-init.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-context/address-resolver.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-utils/info.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/ad-field-write/field-write.hh \
  include/array-detect-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/ad-pipeline/pipeline.hh \
  include/array-detect-gcc-integration/lto-summary.hh \
  include/ad-owned-verdict/owned-conclusion.hh \
  include/ad-write-source/write-source.hh \
  include/array-detect-core/virtual-call-analysis.hh \
  include/array-detect-lto-transform/lto-transform.hh \
  include/array-detect-gcc-integration/gcc-ext-util.hh \
  include/array-detect-utils/info-print.hh

obj/array-detect-utils/info-print.o: \
  src/array-detect-utils/info-print.cc \
  include/array-detect-utils/info-print.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-context/context.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/ad-field-write/field-write.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-context/address-resolver.hh \
  include/ad-write-source/write-source.hh \
  include/array-detect-core/virtual-call-analysis.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-utils/info.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/array-detect-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/array-detect-gcc-integration/gcc-ext-util.hh

obj/array-detect-utils/string-utils.o: \
  src/array-detect-utils/string-utils.cc \
  include/array-detect-utils/string-utils.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/context.hh \
  include/array-detect-gcc-integration/gcc-ext-util.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-context/array-detect-context-gcc-interface.hh

obj/array-detect-lto-transform/lto-transform.o: \
  src/array-detect-lto-transform/lto-transform.cc \
  include/array-detect-lto-transform/lto-transform.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/context.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-gcc-integration/lto-summary.hh \
  include/ad-owned-verdict/owned-conclusion.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-utils/info.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/ad-field-write/field-write.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-context/address-resolver.hh \
  include/array-detect-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/ad-write-source/write-source.hh \
  include/array-detect-core/virtual-call-analysis.hh \
  include/array-detect-context/context-init.hh \
  include/ad-pipeline/pipeline.hh

obj/ad-field-write/field-write.o: \
  src/ad-field-write/field-write.cc \
  include/ad-field-write/field-write.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/context.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-context/address-resolver.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-utils/info.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/array-detect-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/array-detect-gcc-integration/gcc-ext-util.hh

obj/ad-write-source/write-source.o: \
  src/ad-write-source/write-source.cc \
  include/ad-write-source/write-source.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-core/virtual-call-analysis.hh \
  include/array-detect-context/context.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-context/address-resolver.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-utils/info.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/ad-field-write/field-write.hh \
  include/array-detect-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/array-detect-gcc-integration/gcc-ext-util.hh \
  include/array-detect-utils/info-print.hh

obj/ad-source-use-info/source-use-info.o: \
  src/ad-source-use-info/source-use-info.cc \
  include/ad-source-use-info/source-use-info.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/context.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-context/address-resolver.hh \
  include/ad-field-write/field-write.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-source-escape-use-info/source-escape-use-info.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-utils/info.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/array-detect-core/type-field-hash.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/array-detect-utils/info-print.hh \
  include/ad-write-source/write-source.hh \
  include/array-detect-core/virtual-call-analysis.hh

obj/ad-ownership-move/ownership-move.o: \
  src/ad-ownership-move/ownership-move.cc \
  include/ad-ownership-move/ownership-move.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/context.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-context/address-resolver.hh \
  include/ad-field-write/field-write.hh \
  include/ad-write-source/write-source.hh \
  include/array-detect-core/virtual-call-analysis.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-utils/info.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/array-detect-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/array-detect-gcc-integration/gcc-ext-util.hh \
  include/array-detect-utils/info-print.hh

obj/ad-source-escape-use-info/source-escape-use-info.o: \
  src/ad-source-escape-use-info/source-escape-use-info.cc \
  include/ad-source-escape-use-info/source-escape-use-info.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/context.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-context/address-resolver.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-source-use-info/source-use-info.hh \
  include/ad-field-write/field-write.hh \
  include/array-detect-utils/info-print.hh \
  include/ad-write-source/write-source.hh \
  include/array-detect-core/virtual-call-analysis.hh

obj/ad-source-escape-conclude/source-escape-conclude.o: \
  src/ad-source-escape-conclude/source-escape-conclude.cc \
  include/ad-source-escape-conclude/source-escape-conclude.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/context.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-context/address-resolver.hh \
  include/ad-source-use-info/source-use-info.hh \
  include/ad-field-write/field-write.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-source-escape-use-info/source-escape-use-info.hh \
  include/array-detect-utils/info-print.hh \
  include/ad-write-source/write-source.hh \
  include/array-detect-core/virtual-call-analysis.hh

obj/ad-field-escape-conclude/field-escape-conclude.o: \
  src/ad-field-escape-conclude/field-escape-conclude.cc \
  include/ad-field-escape-conclude/field-escape-conclude.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/context.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-context/address-resolver.hh \
  include/ad-source-use-info/source-use-info.hh \
  include/ad-field-write/field-write.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-source-escape-use-info/source-escape-use-info.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-utils/info.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/array-detect-core/type-field-hash.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/ad-source-escape-conclude/source-escape-conclude.hh \
  include/array-detect-utils/info-print.hh \
  include/ad-write-source/write-source.hh \
  include/array-detect-core/virtual-call-analysis.hh \
  include/array-detect-gcc-integration/gcc-ext-util.hh

obj/ad-field-wrapper/field-wrapper.o: \
  src/ad-field-wrapper/field-wrapper.cc \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-context/context.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/address-resolver.hh

obj/ad-owned-verdict/owned-conclusion.o: \
  src/ad-owned-verdict/owned-conclusion.cc \
  include/ad-owned-verdict/owned-conclusion.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/context.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-utils/info.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/ad-field-write/field-write.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-context/address-resolver.hh \
  include/array-detect-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/ad-write-source/write-source.hh \
  include/array-detect-core/virtual-call-analysis.hh \
  include/array-detect-gcc-integration/gcc-ext-util.hh \
  include/ad-source-use-info/source-use-info.hh \
  include/ad-source-escape-use-info/source-escape-use-info.hh \
  include/ad-source-escape-conclude/source-escape-conclude.hh \
  include/ad-ownership-move/ownership-move.hh \
  include/array-detect-utils/info-print.hh \
  include/array-detect-utils/string-utils.hh

obj/ad-capacity-associator/capacity-association.o: \
  src/ad-capacity-associator/capacity-association.cc \
  include/ad-capacity-associator/capacity-association.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/context.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-utils/info.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/ad-field-write/field-write.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-context/address-resolver.hh \
  include/array-detect-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/ad-owned-verdict/owned-conclusion.hh \
  include/ad-write-source/write-source.hh \
  include/array-detect-core/virtual-call-analysis.hh \
  include/ad-array-access/array-access-collector.hh \
  include/array-detect-gcc-integration/gcc-ext-util.hh \
  include/array-detect-utils/info-print.hh \
  include/ad-bound-condition/bound-condition-analyzer.hh \
  include/array-detect-utils/string-utils.hh

obj/ad-array-access/array-access-collector.o: \
  src/ad-array-access/array-access-collector.cc \
  include/ad-array-access/array-access-collector.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/context.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-utils/info.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/ad-field-write/field-write.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-context/address-resolver.hh \
  include/array-detect-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/array-detect-gcc-integration/gcc-ext-util.hh \
  include/array-detect-utils/info-print.hh \
  include/ad-write-source/write-source.hh \
  include/array-detect-core/virtual-call-analysis.hh \
  include/array-detect-utils/string-utils.hh

obj/ad-bound-condition/bound-condition-analyzer.o: \
  src/ad-bound-condition/bound-condition-analyzer.cc \
  include/ad-bound-condition/bound-condition-analyzer.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/context.hh \
  include/ad-array-access/array-access-collector.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-utils/info.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/ad-field-write/field-write.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-context/address-resolver.hh \
  include/array-detect-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/array-detect-gcc-integration/gcc-ext-util.hh \
  include/array-detect-utils/info-print.hh \
  include/ad-write-source/write-source.hh \
  include/array-detect-core/virtual-call-analysis.hh \
  include/array-detect-utils/string-utils.hh

obj/ad-result-aggregator/result-aggregator.o: \
  src/ad-result-aggregator/result-aggregator.cc \
  include/ad-result-aggregator/result-aggregator.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/context.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-utils/info.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/ad-field-write/field-write.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-context/address-resolver.hh \
  include/array-detect-core/type-field-hash.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/ad-owned-verdict/owned-conclusion.hh \
  include/ad-write-source/write-source.hh \
  include/array-detect-core/virtual-call-analysis.hh \
  include/ad-capacity-associator/capacity-association.hh \
  include/ad-array-access/array-access-collector.hh \
  include/ad-bound-condition/bound-condition-analyzer.hh \
  include/array-detect-gcc-integration/gcc-ext-util.hh

obj/ad-pipeline/pipeline.o: \
  src/ad-pipeline/pipeline.cc \
  include/ad-pipeline/pipeline.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/context.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/ad-field-write/field-write.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-context/address-resolver.hh \
  include/ad-write-source/write-source.hh \
  include/array-detect-core/virtual-call-analysis.hh \
  include/ad-source-use-info/source-use-info.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-source-escape-use-info/source-escape-use-info.hh \
  include/ad-source-escape-conclude/source-escape-conclude.hh \
  include/ad-field-escape-conclude/field-escape-conclude.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-utils/info.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/array-detect-core/type-field-hash.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/ad-ownership-move/ownership-move.hh \
  include/array-detect-utils/info-print.hh

obj/ad-driver/driver.o: \
  src/ad-driver/driver.cc \
  include/ad-driver/driver.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/context.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/ad-field-write/field-write.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-context/address-resolver.hh \
  include/ad-write-source/write-source.hh \
  include/array-detect-core/virtual-call-analysis.hh \
  include/ad-source-use-info/source-use-info.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/ad-source-escape-use-info/source-escape-use-info.hh \
  include/ad-source-escape-conclude/source-escape-conclude.hh \
  include/ad-ownership-move/ownership-move.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-utils/info.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/array-detect-core/type-field-hash.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/array-detect-utils/info-print.hh

clean:
	rm -rf obj out

test-xercese: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\"))" | racket test-script/test-xercese.rkt

prepare:
	mkdir -p obj/array-detect-core
	mkdir -p obj/array-detect-context
	mkdir -p obj/array-detect-gcc-integration
	mkdir -p obj/array-detect-utils
	mkdir -p obj/array-detect-lto-transform
	mkdir -p obj/ad-field-write
	mkdir -p obj/ad-write-source
	mkdir -p obj/ad-source-use-info
	mkdir -p obj/ad-ownership-move
	mkdir -p obj/ad-ownership-conclude
	mkdir -p obj/ad-source-escape-use-info
	mkdir -p obj/ad-source-escape-conclude
	mkdir -p obj/ad-field-escape-conclude
	mkdir -p obj/ad-field-wrapper
	mkdir -p obj/ad-owned-verdict
	mkdir -p obj/ad-capacity-associator
	mkdir -p obj/ad-array-access
	mkdir -p obj/ad-bound-condition
	mkdir -p obj/ad-result-aggregator
	mkdir -p obj/ad-field-write-capacity
	mkdir -p obj/ad-array-read-capacity
	mkdir -p obj/ad-array-write-capacity
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
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\" \"-flto\"))" | racket test-script/test-simple-ptr-field.rkt

test-simple-virtual-call-lto: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\" \"-flto\"))" | racket test-script/test-simple-virtual-call.rkt

test-simple-ptr-copy-escape-lto: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\" \"-flto\"))" | racket test-script/test-simple-ptr-copy-escape.rkt

test-bound-check-lto: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\" \"-flto\"))" | racket test-script/test-bound-check.rkt

test-bound-read-lto: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\" \"-flto\"))" | racket test-script/test-bound-read.rkt

test-malloc-size-lto: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\" \"-flto\"))" | racket test-script/test-malloc-size.rkt

test-ownership-transfer-lto: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\" \"-flto\"))" | racket test-script/test-ownership-transfer.rkt

test-phi-ice-lto: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\" \"-flto\"))" | racket test-script/test-phi-ice.rkt

test-trivial-assignment-lto: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\" \"-flto\"))" | racket test-script/test-trivial-assignment.rkt

test-lto: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\" \"-flto\"))" | racket test-script/test-lto.rkt

test-xercese-lto: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\" \"-flto\"))" | racket test-script/test-xercese.rkt

