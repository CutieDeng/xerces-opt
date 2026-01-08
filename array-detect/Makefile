# Generated Makefile for macOS
# Compiler: /opt/homebrew/bin/g++-15
# Shared library extension: .dylib
# Platform-specific flags: -undefined dynamic_lookup

.PHONY: all clean prepare test-xercese test-simple-ptr-field test-simple-virtual-call test-simple-ptr-copy-escape test-bound-check test-bound-read test-malloc-size test-ownership-transfer test-phi-ice test-trivial-assignment test-lto

all: out/plugin-array-detect.dylib

out/plugin-array-detect.dylib: obj/array-detect-core/array-detector.o obj/array-detect-core/field-assignment-debugger.o obj/array-detect-core/type-field-hash.o obj/array-detect-core/virtual-call-analysis.o obj/array-detect-field-write-collector/field-write-collector.o obj/array-detect-context/address-resolver.o obj/array-detect-context/array-detect-context-gcc.o obj/array-detect-context/context-init.o obj/array-detect-context/context.o obj/array-detect-context/state.o obj/array-detect-gcc-integration/context-init-gcc.o obj/array-detect-gcc-integration/gcc-ext-util.o obj/array-detect-gcc-integration/lto-summary.o obj/array-detect-gcc-integration/plugin-top.o obj/array-detect-utils/info-print.o obj/array-detect-utils/string-utils.o obj/array-detect-pipeline-orchestrator/pipeline.o obj/array-detect-field-source-tracer/write-operation-trace-driver.o obj/array-detect-field-source-tracer/write-operation-trace.o obj/array-detect-escape-use-collector/source-escape-collection.o obj/array-detect-escape-evidence-synthesizer/escape-synthesizer.o obj/array-detect-ownership-transfer-analyzer/ownership-transfer-analysis.o obj/array-detect-owned-verdict-generator/owned-conclusion.o obj/array-detect-capacity-field-associator/capacity-association.o obj/array-detect-array-access-collector/array-access-collector.o obj/array-detect-bound-condition-analyzer/bound-condition-analyzer.o obj/array-detect-unified-result-aggregator/result-aggregator.o obj/array-detect-lto-transform/lto-transform.o
	/opt/homebrew/bin/g++-15 "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/libmpc/include" "-L" "/opt/homebrew/opt/libmpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "obj/array-detect-core/array-detector.o" "obj/array-detect-core/field-assignment-debugger.o" "obj/array-detect-core/type-field-hash.o" "obj/array-detect-core/virtual-call-analysis.o" "obj/array-detect-field-write-collector/field-write-collector.o" "obj/array-detect-context/address-resolver.o" "obj/array-detect-context/array-detect-context-gcc.o" "obj/array-detect-context/context-init.o" "obj/array-detect-context/context.o" "obj/array-detect-context/state.o" "obj/array-detect-gcc-integration/context-init-gcc.o" "obj/array-detect-gcc-integration/gcc-ext-util.o" "obj/array-detect-gcc-integration/lto-summary.o" "obj/array-detect-gcc-integration/plugin-top.o" "obj/array-detect-utils/info-print.o" "obj/array-detect-utils/string-utils.o" "obj/array-detect-pipeline-orchestrator/pipeline.o" "obj/array-detect-field-source-tracer/write-operation-trace-driver.o" "obj/array-detect-field-source-tracer/write-operation-trace.o" "obj/array-detect-escape-use-collector/source-escape-collection.o" "obj/array-detect-escape-evidence-synthesizer/escape-synthesizer.o" "obj/array-detect-ownership-transfer-analyzer/ownership-transfer-analysis.o" "obj/array-detect-owned-verdict-generator/owned-conclusion.o" "obj/array-detect-capacity-field-associator/capacity-association.o" "obj/array-detect-array-access-collector/array-access-collector.o" "obj/array-detect-bound-condition-analyzer/bound-condition-analyzer.o" "obj/array-detect-unified-result-aggregator/result-aggregator.o" "obj/array-detect-lto-transform/lto-transform.o" -o "out/plugin-array-detect.dylib"

obj/array-detect-core/array-detector.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/libmpc/include" "-L" "/opt/homebrew/opt/libmpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-core/array-detector.cc" -o "obj/array-detect-core/array-detector.o"

obj/array-detect-core/field-assignment-debugger.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/libmpc/include" "-L" "/opt/homebrew/opt/libmpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-core/field-assignment-debugger.cc" -o "obj/array-detect-core/field-assignment-debugger.o"

obj/array-detect-core/type-field-hash.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/libmpc/include" "-L" "/opt/homebrew/opt/libmpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-core/type-field-hash.cc" -o "obj/array-detect-core/type-field-hash.o"

obj/array-detect-core/virtual-call-analysis.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/libmpc/include" "-L" "/opt/homebrew/opt/libmpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-core/virtual-call-analysis.cc" -o "obj/array-detect-core/virtual-call-analysis.o"

obj/array-detect-field-write-collector/field-write-collector.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/libmpc/include" "-L" "/opt/homebrew/opt/libmpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-field-write-collector/field-write-collector.cc" -o "obj/array-detect-field-write-collector/field-write-collector.o"

obj/array-detect-context/address-resolver.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/libmpc/include" "-L" "/opt/homebrew/opt/libmpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-context/address-resolver.cc" -o "obj/array-detect-context/address-resolver.o"

obj/array-detect-context/array-detect-context-gcc.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/libmpc/include" "-L" "/opt/homebrew/opt/libmpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-context/array-detect-context-gcc.cc" -o "obj/array-detect-context/array-detect-context-gcc.o"

obj/array-detect-context/context-init.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/libmpc/include" "-L" "/opt/homebrew/opt/libmpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-context/context-init.cc" -o "obj/array-detect-context/context-init.o"

obj/array-detect-context/context.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/libmpc/include" "-L" "/opt/homebrew/opt/libmpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-context/context.cc" -o "obj/array-detect-context/context.o"

obj/array-detect-context/state.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/libmpc/include" "-L" "/opt/homebrew/opt/libmpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-context/state.cc" -o "obj/array-detect-context/state.o"

obj/array-detect-gcc-integration/context-init-gcc.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/libmpc/include" "-L" "/opt/homebrew/opt/libmpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-gcc-integration/context-init-gcc.cc" -o "obj/array-detect-gcc-integration/context-init-gcc.o"

obj/array-detect-gcc-integration/gcc-ext-util.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/libmpc/include" "-L" "/opt/homebrew/opt/libmpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-gcc-integration/gcc-ext-util.cc" -o "obj/array-detect-gcc-integration/gcc-ext-util.o"

obj/array-detect-gcc-integration/lto-summary.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/libmpc/include" "-L" "/opt/homebrew/opt/libmpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-gcc-integration/lto-summary.cc" -o "obj/array-detect-gcc-integration/lto-summary.o"

obj/array-detect-gcc-integration/plugin-top.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/libmpc/include" "-L" "/opt/homebrew/opt/libmpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-gcc-integration/plugin-top.cc" -o "obj/array-detect-gcc-integration/plugin-top.o"

obj/array-detect-utils/info-print.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/libmpc/include" "-L" "/opt/homebrew/opt/libmpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-utils/info-print.cc" -o "obj/array-detect-utils/info-print.o"

obj/array-detect-utils/string-utils.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/libmpc/include" "-L" "/opt/homebrew/opt/libmpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-utils/string-utils.cc" -o "obj/array-detect-utils/string-utils.o"

obj/array-detect-pipeline-orchestrator/pipeline.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/libmpc/include" "-L" "/opt/homebrew/opt/libmpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-pipeline-orchestrator/pipeline.cc" -o "obj/array-detect-pipeline-orchestrator/pipeline.o"

obj/array-detect-field-source-tracer/write-operation-trace-driver.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/libmpc/include" "-L" "/opt/homebrew/opt/libmpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-field-source-tracer/write-operation-trace-driver.cc" -o "obj/array-detect-field-source-tracer/write-operation-trace-driver.o"

obj/array-detect-field-source-tracer/write-operation-trace.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/libmpc/include" "-L" "/opt/homebrew/opt/libmpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-field-source-tracer/write-operation-trace.cc" -o "obj/array-detect-field-source-tracer/write-operation-trace.o"

obj/array-detect-escape-use-collector/source-escape-collection.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/libmpc/include" "-L" "/opt/homebrew/opt/libmpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-escape-use-collector/source-escape-collection.cc" -o "obj/array-detect-escape-use-collector/source-escape-collection.o"

obj/array-detect-escape-evidence-synthesizer/escape-synthesizer.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/libmpc/include" "-L" "/opt/homebrew/opt/libmpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-escape-evidence-synthesizer/escape-synthesizer.cc" -o "obj/array-detect-escape-evidence-synthesizer/escape-synthesizer.o"

obj/array-detect-ownership-transfer-analyzer/ownership-transfer-analysis.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/libmpc/include" "-L" "/opt/homebrew/opt/libmpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-ownership-transfer-analyzer/ownership-transfer-analysis.cc" -o "obj/array-detect-ownership-transfer-analyzer/ownership-transfer-analysis.o"

obj/array-detect-owned-verdict-generator/owned-conclusion.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/libmpc/include" "-L" "/opt/homebrew/opt/libmpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-owned-verdict-generator/owned-conclusion.cc" -o "obj/array-detect-owned-verdict-generator/owned-conclusion.o"

obj/array-detect-capacity-field-associator/capacity-association.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/libmpc/include" "-L" "/opt/homebrew/opt/libmpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-capacity-field-associator/capacity-association.cc" -o "obj/array-detect-capacity-field-associator/capacity-association.o"

obj/array-detect-array-access-collector/array-access-collector.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/libmpc/include" "-L" "/opt/homebrew/opt/libmpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-array-access-collector/array-access-collector.cc" -o "obj/array-detect-array-access-collector/array-access-collector.o"

obj/array-detect-bound-condition-analyzer/bound-condition-analyzer.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/libmpc/include" "-L" "/opt/homebrew/opt/libmpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-bound-condition-analyzer/bound-condition-analyzer.cc" -o "obj/array-detect-bound-condition-analyzer/bound-condition-analyzer.o"

obj/array-detect-unified-result-aggregator/result-aggregator.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/libmpc/include" "-L" "/opt/homebrew/opt/libmpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-unified-result-aggregator/result-aggregator.cc" -o "obj/array-detect-unified-result-aggregator/result-aggregator.o"

obj/array-detect-lto-transform/lto-transform.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/libmpc/include" "-L" "/opt/homebrew/opt/libmpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-lto-transform/lto-transform.cc" -o "obj/array-detect-lto-transform/lto-transform.o"

obj/array-detect-core/array-detector.o: \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/src/array-detect-core/array-detector.cc \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/prelude.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/array-detector.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/state.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/config/array-detect-state.txt \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-common.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc-interface.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-result/analysis-data.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hashmap-traits.hh

obj/array-detect-core/field-assignment-debugger.o: \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/src/array-detect-core/field-assignment-debugger.cc \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/prelude.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/state.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/config/array-detect-state.txt \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/field-assignment-debugger.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc-interface.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/array-detector.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-common.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-result/analysis-data.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hashmap-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-ext-util.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context-init.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/address-resolver.hh

obj/array-detect-core/type-field-hash.o: \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/src/array-detect-core/type-field-hash.cc \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-common.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/array-detector.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/prelude.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/state.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/config/array-detect-state.txt \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc-interface.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-result/analysis-data.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hashmap-traits.hh

obj/array-detect-core/virtual-call-analysis.o: \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/src/array-detect-core/virtual-call-analysis.cc \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/virtual-call-analysis.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-common.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/prelude.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/state.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/config/array-detect-state.txt \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/address-resolver.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-result/analysis-data.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-ext-util.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc-interface.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info-print.hh

obj/array-detect-field-write-collector/field-write-collector.o: \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/src/array-detect-field-write-collector/field-write-collector.cc \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/prelude.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/state.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/config/array-detect-state.txt \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-field-write-collector/field-write-collector.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc-interface.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/array-detector.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-common.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-result/analysis-data.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hashmap-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-ext-util.hh

obj/array-detect-context/address-resolver.o: \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/src/array-detect-context/address-resolver.cc \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/address-resolver.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/prelude.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/state.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/config/array-detect-state.txt \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-common.hh

obj/array-detect-context/array-detect-context-gcc.o: \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/src/array-detect-context/array-detect-context-gcc.cc \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/prelude.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/state.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/config/array-detect-state.txt \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-common.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/address-resolver.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc-interface.hh

obj/array-detect-context/context-init.o: \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/src/array-detect-context/context-init.cc \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/prelude.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/state.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/config/array-detect-state.txt \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context-init.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-common.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/address-resolver.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc-interface.hh

obj/array-detect-context/context.o: \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/src/array-detect-context/context.cc \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context.hh

obj/array-detect-context/state.o: \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/src/array-detect-context/state.cc \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/state.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/config/array-detect-state.txt

obj/array-detect-gcc-integration/context-init-gcc.o: \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/src/array-detect-gcc-integration/context-init-gcc.cc \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-common.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context-init.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/prelude.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/state.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/config/array-detect-state.txt \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/address-resolver.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc-interface.hh

obj/array-detect-gcc-integration/gcc-ext-util.o: \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/src/array-detect-gcc-integration/gcc-ext-util.cc \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-ext-util.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-common.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/prelude.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/state.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/config/array-detect-state.txt \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc-interface.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/array-detector.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-result/analysis-data.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hashmap-traits.hh

obj/array-detect-gcc-integration/lto-summary.o: \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/src/array-detect-gcc-integration/lto-summary.cc \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/lto-summary.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-common.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-owned-verdict-generator/owned-conclusion.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/prelude.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/array-detector.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/state.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/config/array-detect-state.txt \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc-interface.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-result/analysis-data.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hashmap-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-field-source-tracer/field-source-variant.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-unified-result-aggregator/result-aggregator.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-capacity-field-associator/capacity-association.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-array-access-collector/array-access-collector.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-bound-condition-analyzer/bound-condition-analyzer.hh

obj/array-detect-gcc-integration/plugin-top.o: \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/src/array-detect-gcc-integration/plugin-top.cc \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-common.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/prelude.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/state.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/config/array-detect-state.txt \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context-init.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/address-resolver.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/array-detector.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc-interface.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-result/analysis-data.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hashmap-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-pipeline-orchestrator/pipeline.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/lto-summary.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-owned-verdict-generator/owned-conclusion.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-field-source-tracer/field-source-variant.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-lto-transform/lto-transform.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-ext-util.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info-print.hh

obj/array-detect-utils/info-print.o: \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/src/array-detect-utils/info-print.cc \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info-print.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/prelude.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/state.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/config/array-detect-state.txt \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-common.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/array-detector.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc-interface.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-result/analysis-data.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hashmap-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/virtual-call-analysis.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/address-resolver.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-ext-util.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-field-source-tracer/field-source-variant.hh

obj/array-detect-utils/string-utils.o: \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/src/array-detect-utils/string-utils.cc \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/string-utils.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/prelude.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-ext-util.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-common.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/state.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/config/array-detect-state.txt \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc-interface.hh

obj/array-detect-pipeline-orchestrator/pipeline.o: \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/src/array-detect-pipeline-orchestrator/pipeline.cc \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-pipeline-orchestrator/pipeline.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/prelude.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/state.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/config/array-detect-state.txt \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-field-write-collector/field-write-collector.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc-interface.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-field-source-tracer/write-operation-trace.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/array-detector.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-common.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-result/analysis-data.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hashmap-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-field-source-tracer/field-source-variant.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-escape-use-collector/source-escape-collection.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/address-resolver.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-escape-evidence-synthesizer/escape-synthesizer.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-ownership-transfer-analyzer/ownership-transfer-analysis.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-owned-verdict-generator/owned-conclusion.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-capacity-field-associator/capacity-association.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-array-access-collector/array-access-collector.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-bound-condition-analyzer/bound-condition-analyzer.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-unified-result-aggregator/result-aggregator.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info-print.hh

obj/array-detect-field-source-tracer/write-operation-trace-driver.o: \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/src/array-detect-field-source-tracer/write-operation-trace-driver.cc \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/prelude.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/state.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/config/array-detect-state.txt \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-field-source-tracer/write-operation-trace.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc-interface.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/array-detector.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-common.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-result/analysis-data.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hashmap-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-field-source-tracer/field-source-variant.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-ext-util.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/virtual-call-analysis.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/address-resolver.hh

obj/array-detect-field-source-tracer/write-operation-trace.o: \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/src/array-detect-field-source-tracer/write-operation-trace.cc \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/prelude.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/state.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/config/array-detect-state.txt \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-field-source-tracer/write-operation-trace.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc-interface.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/array-detector.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-common.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-result/analysis-data.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hashmap-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-field-source-tracer/field-source-variant.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-ext-util.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info-print.hh

obj/array-detect-escape-use-collector/source-escape-collection.o: \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/src/array-detect-escape-use-collector/source-escape-collection.cc \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/prelude.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/state.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/config/array-detect-state.txt \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-escape-use-collector/source-escape-collection.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-common.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/address-resolver.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-result/analysis-data.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/array-detector.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc-interface.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hashmap-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-field-source-tracer/write-operation-trace.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-field-source-tracer/field-source-variant.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info-print.hh

obj/array-detect-escape-evidence-synthesizer/escape-synthesizer.o: \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/src/array-detect-escape-evidence-synthesizer/escape-synthesizer.cc \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-escape-evidence-synthesizer/escape-synthesizer.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-common.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/prelude.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/state.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/config/array-detect-state.txt \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/address-resolver.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-result/analysis-data.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-escape-use-collector/source-escape-collection.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/array-detector.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc-interface.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hashmap-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info-print.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-ext-util.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-field-source-tracer/field-source-variant.hh

obj/array-detect-ownership-transfer-analyzer/ownership-transfer-analysis.o: \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/src/array-detect-ownership-transfer-analyzer/ownership-transfer-analysis.cc \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-ownership-transfer-analyzer/ownership-transfer-analysis.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-common.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/prelude.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/state.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/config/array-detect-state.txt \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/address-resolver.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-result/analysis-data.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/array-detector.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc-interface.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hashmap-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-field-source-tracer/field-source-variant.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-ext-util.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info-print.hh

obj/array-detect-owned-verdict-generator/owned-conclusion.o: \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/src/array-detect-owned-verdict-generator/owned-conclusion.cc \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-owned-verdict-generator/owned-conclusion.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/prelude.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/array-detector.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/state.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/config/array-detect-state.txt \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-common.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc-interface.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-result/analysis-data.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hashmap-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-field-source-tracer/field-source-variant.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-ext-util.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-escape-use-collector/source-escape-collection.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/address-resolver.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-escape-evidence-synthesizer/escape-synthesizer.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-ownership-transfer-analyzer/ownership-transfer-analysis.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info-print.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/string-utils.hh

obj/array-detect-capacity-field-associator/capacity-association.o: \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/src/array-detect-capacity-field-associator/capacity-association.cc \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-capacity-field-associator/capacity-association.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/prelude.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/array-detector.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/state.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/config/array-detect-state.txt \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-common.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc-interface.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-result/analysis-data.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hashmap-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-owned-verdict-generator/owned-conclusion.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-field-source-tracer/field-source-variant.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-array-access-collector/array-access-collector.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-ext-util.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info-print.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-bound-condition-analyzer/bound-condition-analyzer.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/string-utils.hh

obj/array-detect-array-access-collector/array-access-collector.o: \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/src/array-detect-array-access-collector/array-access-collector.cc \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-array-access-collector/array-access-collector.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/prelude.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/array-detector.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/state.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/config/array-detect-state.txt \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-common.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc-interface.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-result/analysis-data.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hashmap-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-ext-util.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info-print.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/string-utils.hh

obj/array-detect-bound-condition-analyzer/bound-condition-analyzer.o: \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/src/array-detect-bound-condition-analyzer/bound-condition-analyzer.cc \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-bound-condition-analyzer/bound-condition-analyzer.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/prelude.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-array-access-collector/array-access-collector.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/array-detector.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/state.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/config/array-detect-state.txt \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-common.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc-interface.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-result/analysis-data.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hashmap-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-ext-util.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info-print.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/address-resolver.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/string-utils.hh

obj/array-detect-unified-result-aggregator/result-aggregator.o: \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/src/array-detect-unified-result-aggregator/result-aggregator.cc \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-unified-result-aggregator/result-aggregator.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/prelude.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/array-detector.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/state.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/config/array-detect-state.txt \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-common.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc-interface.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-result/analysis-data.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hashmap-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-owned-verdict-generator/owned-conclusion.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-field-source-tracer/field-source-variant.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-capacity-field-associator/capacity-association.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-array-access-collector/array-access-collector.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-bound-condition-analyzer/bound-condition-analyzer.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/address-resolver.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-ext-util.hh

obj/array-detect-lto-transform/lto-transform.o: \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/src/array-detect-lto-transform/lto-transform.cc \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-lto-transform/lto-transform.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/gcc-common.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-gcc-integration/lto-summary.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-owned-verdict-generator/owned-conclusion.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/prelude.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/array-detector.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/state.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/config/array-detect-state.txt \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-utils/info.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc-interface.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-result/analysis-data.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hash-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-core/type-field-hashmap-traits.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-field-source-tracer/field-source-variant.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/array-detect-context-gcc.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/address-resolver.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-context/context-init.hh \
  /Users/cutiedeng/Documents/trae-gcc-plugin/array-detect/include/array-detect-pipeline-orchestrator/pipeline.hh

clean:
	rm -rf obj out

test-xercese: out/plugin-array-detect.dylib
	@(cd ../test/test-xercese && mkdir -p out && echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=../../array-detect/out/plugin-array-detect.dylib\"))" | racket build-xercese.rkt)

prepare:
	mkdir -p obj/array-detect-core
	mkdir -p obj/array-detect-field-write-collector
	mkdir -p obj/array-detect-context
	mkdir -p obj/array-detect-gcc-integration
	mkdir -p obj/array-detect-utils
	mkdir -p obj/array-detect-pipeline-orchestrator
	mkdir -p obj/array-detect-field-source-tracer
	mkdir -p obj/array-detect-escape-use-collector
	mkdir -p obj/array-detect-escape-evidence-synthesizer
	mkdir -p obj/array-detect-ownership-transfer-analyzer
	mkdir -p obj/array-detect-owned-verdict-generator
	mkdir -p obj/array-detect-capacity-field-associator
	mkdir -p obj/array-detect-array-access-collector
	mkdir -p obj/array-detect-bound-condition-analyzer
	mkdir -p obj/array-detect-unified-result-aggregator
	mkdir -p obj/array-detect-lto-transform
	mkdir -p out

test-simple-ptr-field: out/plugin-array-detect.dylib
	@(cd ../test/test-simple-ptr-field && mkdir -p out && echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=../../array-detect/out/plugin-array-detect.dylib\"))" | racket build.rkt)

test-simple-virtual-call: out/plugin-array-detect.dylib
	@(cd ../test/test-simple-virtual-call && mkdir -p out && echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=../../array-detect/out/plugin-array-detect.dylib\"))" | racket build.rkt)

test-simple-ptr-copy-escape: out/plugin-array-detect.dylib
	@(cd ../test/test-simple-ptr-copy-escape && mkdir -p out && echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=../../array-detect/out/plugin-array-detect.dylib\"))" | racket build.rkt)

test-bound-check: out/plugin-array-detect.dylib
	@(cd ../test/test-bound-check && mkdir -p out && echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=../../array-detect/out/plugin-array-detect.dylib\"))" | racket build.rkt)

test-bound-read: out/plugin-array-detect.dylib
	@(cd ../test/test-bound-read && mkdir -p out && echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=../../array-detect/out/plugin-array-detect.dylib\"))" | racket build.rkt)

test-malloc-size: out/plugin-array-detect.dylib
	@(cd ../test/test-malloc-size && mkdir -p out && echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=../../array-detect/out/plugin-array-detect.dylib\"))" | racket build.rkt)

test-ownership-transfer: out/plugin-array-detect.dylib
	@(cd ../test/test-ownership-transfer && mkdir -p out && echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=../../array-detect/out/plugin-array-detect.dylib\"))" | racket build.rkt)

test-phi-ice: out/plugin-array-detect.dylib
	@(cd ../test/test-phi-ice && mkdir -p out && echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=../../array-detect/out/plugin-array-detect.dylib\"))" | racket build.rkt)

test-trivial-assignment: out/plugin-array-detect.dylib
	@(cd ../test/test-trivial-assignment && mkdir -p out && echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=../../array-detect/out/plugin-array-detect.dylib\"))" | racket build.rkt)

test-lto: out/plugin-array-detect.dylib
	@(cd ../test/test-lto && mkdir -p out && echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=../../array-detect/out/plugin-array-detect.dylib\" \"-flto\"))" | racket build.rkt)

