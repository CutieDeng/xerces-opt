# Generated Makefile for macosx
# Compiler: /opt/homebrew/bin/g++-15
# Shared library extension: .dylib
# Platform-specific flags:  -undefined dynamic_lookup

.PHONY: all clean prepare test-xercese test-simple-ptr-field test-simple-virtual-call test-simple-ptr-copy-escape test-bound-check test-bound-read test-malloc-size test-ownership-transfer test-phi-ice test-trivial-assignment test-simple-ptr-field-lto test-simple-virtual-call-lto test-simple-ptr-copy-escape-lto test-bound-check-lto test-bound-read-lto test-malloc-size-lto test-ownership-transfer-lto test-phi-ice-lto test-trivial-assignment-lto test-lto test-xercese-lto

all: out/plugin-array-detect.dylib

out/plugin-array-detect.dylib: obj/array-detect-core/array-detector.o obj/array-detect-core/field-assignment-debugger.o obj/array-detect-core/type-field-hash.o obj/array-detect-core/virtual-call-analysis.o obj/array-detect-field-write-collector/field-write-collector.o obj/array-detect-context/address-resolver.o obj/array-detect-context/array-detect-context-gcc.o obj/array-detect-context/context-init.o obj/array-detect-context/context.o obj/array-detect-context/state.o obj/array-detect-gcc-integration/context-init-gcc.o obj/array-detect-gcc-integration/gcc-ext-util.o obj/array-detect-gcc-integration/lto-summary.o obj/array-detect-gcc-integration/plugin-top.o obj/array-detect-utils/info-print.o obj/array-detect-utils/string-utils.o obj/array-detect-pipeline-orchestrator/pipeline.o obj/array-detect-field-source-tracer/write-operation-trace-driver.o obj/array-detect-field-source-tracer/write-operation-trace.o obj/array-detect-escape-use-collector/source-escape-collection.o obj/array-detect-escape-evidence-synthesizer/escape-synthesizer.o obj/array-detect-ownership-transfer-analyzer/ownership-transfer-analysis.o obj/array-detect-owned-verdict-generator/owned-conclusion.o obj/array-detect-capacity-field-associator/capacity-association.o obj/array-detect-array-access-collector/array-access-collector.o obj/array-detect-bound-condition-analyzer/bound-condition-analyzer.o obj/array-detect-unified-result-aggregator/result-aggregator.o obj/array-detect-lto-transform/lto-transform.o obj/ad-escaped-use/escaped-use.o obj/ad-source-escape/source-escape.o obj/ad-field-escape/field-escape.o obj/ad-field-wrapper/field-wrapper.o
	/opt/homebrew/bin/g++-15 "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "obj/array-detect-core/array-detector.o" "obj/array-detect-core/field-assignment-debugger.o" "obj/array-detect-core/type-field-hash.o" "obj/array-detect-core/virtual-call-analysis.o" "obj/array-detect-field-write-collector/field-write-collector.o" "obj/array-detect-context/address-resolver.o" "obj/array-detect-context/array-detect-context-gcc.o" "obj/array-detect-context/context-init.o" "obj/array-detect-context/context.o" "obj/array-detect-context/state.o" "obj/array-detect-gcc-integration/context-init-gcc.o" "obj/array-detect-gcc-integration/gcc-ext-util.o" "obj/array-detect-gcc-integration/lto-summary.o" "obj/array-detect-gcc-integration/plugin-top.o" "obj/array-detect-utils/info-print.o" "obj/array-detect-utils/string-utils.o" "obj/array-detect-pipeline-orchestrator/pipeline.o" "obj/array-detect-field-source-tracer/write-operation-trace-driver.o" "obj/array-detect-field-source-tracer/write-operation-trace.o" "obj/array-detect-escape-use-collector/source-escape-collection.o" "obj/array-detect-escape-evidence-synthesizer/escape-synthesizer.o" "obj/array-detect-ownership-transfer-analyzer/ownership-transfer-analysis.o" "obj/array-detect-owned-verdict-generator/owned-conclusion.o" "obj/array-detect-capacity-field-associator/capacity-association.o" "obj/array-detect-array-access-collector/array-access-collector.o" "obj/array-detect-bound-condition-analyzer/bound-condition-analyzer.o" "obj/array-detect-unified-result-aggregator/result-aggregator.o" "obj/array-detect-lto-transform/lto-transform.o" "obj/ad-escaped-use/escaped-use.o" "obj/ad-source-escape/source-escape.o" "obj/ad-field-escape/field-escape.o" "obj/ad-field-wrapper/field-wrapper.o" -o "out/plugin-array-detect.dylib"

obj/array-detect-core/array-detector.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-core/array-detector.cc" -o "obj/array-detect-core/array-detector.o"

obj/array-detect-core/field-assignment-debugger.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-core/field-assignment-debugger.cc" -o "obj/array-detect-core/field-assignment-debugger.o"

obj/array-detect-core/type-field-hash.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-core/type-field-hash.cc" -o "obj/array-detect-core/type-field-hash.o"

obj/array-detect-core/virtual-call-analysis.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-core/virtual-call-analysis.cc" -o "obj/array-detect-core/virtual-call-analysis.o"

obj/array-detect-field-write-collector/field-write-collector.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-field-write-collector/field-write-collector.cc" -o "obj/array-detect-field-write-collector/field-write-collector.o"

obj/array-detect-context/address-resolver.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-context/address-resolver.cc" -o "obj/array-detect-context/address-resolver.o"

obj/array-detect-context/array-detect-context-gcc.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-context/array-detect-context-gcc.cc" -o "obj/array-detect-context/array-detect-context-gcc.o"

obj/array-detect-context/context-init.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-context/context-init.cc" -o "obj/array-detect-context/context-init.o"

obj/array-detect-context/context.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-context/context.cc" -o "obj/array-detect-context/context.o"

obj/array-detect-context/state.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-context/state.cc" -o "obj/array-detect-context/state.o"

obj/array-detect-gcc-integration/context-init-gcc.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-gcc-integration/context-init-gcc.cc" -o "obj/array-detect-gcc-integration/context-init-gcc.o"

obj/array-detect-gcc-integration/gcc-ext-util.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-gcc-integration/gcc-ext-util.cc" -o "obj/array-detect-gcc-integration/gcc-ext-util.o"

obj/array-detect-gcc-integration/lto-summary.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-gcc-integration/lto-summary.cc" -o "obj/array-detect-gcc-integration/lto-summary.o"

obj/array-detect-gcc-integration/plugin-top.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-gcc-integration/plugin-top.cc" -o "obj/array-detect-gcc-integration/plugin-top.o"

obj/array-detect-utils/info-print.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-utils/info-print.cc" -o "obj/array-detect-utils/info-print.o"

obj/array-detect-utils/string-utils.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-utils/string-utils.cc" -o "obj/array-detect-utils/string-utils.o"

obj/array-detect-pipeline-orchestrator/pipeline.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-pipeline-orchestrator/pipeline.cc" -o "obj/array-detect-pipeline-orchestrator/pipeline.o"

obj/array-detect-field-source-tracer/write-operation-trace-driver.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-field-source-tracer/write-operation-trace-driver.cc" -o "obj/array-detect-field-source-tracer/write-operation-trace-driver.o"

obj/array-detect-field-source-tracer/write-operation-trace.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-field-source-tracer/write-operation-trace.cc" -o "obj/array-detect-field-source-tracer/write-operation-trace.o"

obj/array-detect-escape-use-collector/source-escape-collection.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-escape-use-collector/source-escape-collection.cc" -o "obj/array-detect-escape-use-collector/source-escape-collection.o"

obj/array-detect-escape-evidence-synthesizer/escape-synthesizer.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-escape-evidence-synthesizer/escape-synthesizer.cc" -o "obj/array-detect-escape-evidence-synthesizer/escape-synthesizer.o"

obj/array-detect-ownership-transfer-analyzer/ownership-transfer-analysis.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-ownership-transfer-analyzer/ownership-transfer-analysis.cc" -o "obj/array-detect-ownership-transfer-analyzer/ownership-transfer-analysis.o"

obj/array-detect-owned-verdict-generator/owned-conclusion.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-owned-verdict-generator/owned-conclusion.cc" -o "obj/array-detect-owned-verdict-generator/owned-conclusion.o"

obj/array-detect-capacity-field-associator/capacity-association.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-capacity-field-associator/capacity-association.cc" -o "obj/array-detect-capacity-field-associator/capacity-association.o"

obj/array-detect-array-access-collector/array-access-collector.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-array-access-collector/array-access-collector.cc" -o "obj/array-detect-array-access-collector/array-access-collector.o"

obj/array-detect-bound-condition-analyzer/bound-condition-analyzer.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-bound-condition-analyzer/bound-condition-analyzer.cc" -o "obj/array-detect-bound-condition-analyzer/bound-condition-analyzer.o"

obj/array-detect-unified-result-aggregator/result-aggregator.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-unified-result-aggregator/result-aggregator.cc" -o "obj/array-detect-unified-result-aggregator/result-aggregator.o"

obj/array-detect-lto-transform/lto-transform.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/array-detect-lto-transform/lto-transform.cc" -o "obj/array-detect-lto-transform/lto-transform.o"

obj/ad-escaped-use/escaped-use.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-escaped-use/escaped-use.cc" -o "obj/ad-escaped-use/escaped-use.o"

obj/ad-source-escape/source-escape.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-source-escape/source-escape.cc" -o "obj/ad-source-escape/source-escape.o"

obj/ad-field-escape/field-escape.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-field-escape/field-escape.cc" -o "obj/ad-field-escape/field-escape.o"

obj/ad-field-wrapper/field-wrapper.o:
	/opt/homebrew/bin/g++-15 -c "-fPIC" "-fno-rtti" "-shared" "-I" "config" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin" "-I" "/opt/homebrew/Cellar/gcc/15.2.0/bin/../lib/gcc/current/gcc/aarch64-apple-darwin25/15/plugin/include" "-I" "include/array-detect-core" "-I" "include/array-detect-field-write-collector" "-I" "include/array-detect-context" "-I" "include/array-detect-gcc-integration" "-I" "include/array-detect-utils" "-I" "include/array-detect-pipeline-orchestrator" "-I" "include/array-detect-field-source-tracer" "-I" "include/array-detect-escape-use-collector" "-I" "include/array-detect-escape-evidence-synthesizer" "-I" "include/array-detect-ownership-transfer-analyzer" "-I" "include/array-detect-owned-verdict-generator" "-I" "include/array-detect-capacity-field-associator" "-I" "include/array-detect-array-access-collector" "-I" "include/array-detect-bound-condition-analyzer" "-I" "include/array-detect-unified-result-aggregator" "-I" "include/array-detect-lto-transform" "-I" "include/ad-escaped-use" "-I" "include/ad-source-escape" "-I" "include/ad-field-escape" "-I" "include/ad-field-wrapper" "-I" "include/array-detect-result" "-undefined" "dynamic_lookup" "-Wall" "-Wextra" "-Wno-deprecated-declarations" "-Wa,-Wno-overriding-deployment-version" "-std=c++17" "-g" "-O2" "-undefined" "dynamic_lookup" "-I" "/opt/homebrew/opt/gmp/include" "-L" "/opt/homebrew/opt/gmp/lib" "-I" "/opt/homebrew/opt/mpc/include" "-L" "/opt/homebrew/opt/mpc/lib" "-I" "/opt/homebrew/opt/mpfr/include" "-L" "/opt/homebrew/opt/mpfr/lib" "src/ad-field-wrapper/field-wrapper.cc" -o "obj/ad-field-wrapper/field-wrapper.o"

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
  include/array-detect-result/analysis-data.hh \
  include/array-detect-core/type-field-hash.hh \
  include/array-detect-core/field-analysis.hh \
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
  include/array-detect-result/analysis-data.hh \
  include/array-detect-core/type-field-hash.hh \
  include/array-detect-core/field-analysis.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/array-detect-gcc-integration/gcc-ext-util.hh \
  include/array-detect-context/context-init.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-context/address-resolver.hh

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
  include/array-detect-result/analysis-data.hh \
  include/array-detect-core/field-analysis.hh \
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
  include/array-detect-result/analysis-data.hh \
  include/array-detect-gcc-integration/gcc-ext-util.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/array-detect-utils/info-print.hh \
  include/array-detect-field-source-tracer/field-source-variant.hh

obj/array-detect-field-write-collector/field-write-collector.o: \
  src/array-detect-field-write-collector/field-write-collector.cc \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-field-write-collector/field-write-collector.hh \
  include/array-detect-context/context.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-utils/info.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-result/analysis-data.hh \
  include/array-detect-core/type-field-hash.hh \
  include/array-detect-core/field-analysis.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/array-detect-gcc-integration/gcc-ext-util.hh

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
  include/array-detect-result/analysis-data.hh \
  include/array-detect-core/type-field-hash.hh \
  include/array-detect-core/field-analysis.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh

obj/array-detect-gcc-integration/lto-summary.o: \
  src/array-detect-gcc-integration/lto-summary.cc \
  include/array-detect-gcc-integration/lto-summary.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/context.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-owned-verdict-generator/owned-conclusion.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-utils/info.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/array-detect-result/analysis-data.hh \
  include/array-detect-core/type-field-hash.hh \
  include/array-detect-core/field-analysis.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/array-detect-field-source-tracer/field-source-variant.hh \
  include/array-detect-unified-result-aggregator/result-aggregator.hh \
  include/array-detect-capacity-field-associator/capacity-association.hh \
  include/array-detect-array-access-collector/array-access-collector.hh \
  include/array-detect-bound-condition-analyzer/bound-condition-analyzer.hh

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
  include/array-detect-result/analysis-data.hh \
  include/array-detect-core/type-field-hash.hh \
  include/array-detect-core/field-analysis.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/array-detect-pipeline-orchestrator/pipeline.hh \
  include/array-detect-gcc-integration/lto-summary.hh \
  include/array-detect-owned-verdict-generator/owned-conclusion.hh \
  include/array-detect-field-source-tracer/field-source-variant.hh \
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
  include/array-detect-result/analysis-data.hh \
  include/array-detect-field-source-tracer/field-source-variant.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-utils/info.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/array-detect-core/type-field-hash.hh \
  include/array-detect-core/field-analysis.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/array-detect-core/virtual-call-analysis.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-context/address-resolver.hh \
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

obj/array-detect-pipeline-orchestrator/pipeline.o: \
  src/array-detect-pipeline-orchestrator/pipeline.cc \
  include/array-detect-pipeline-orchestrator/pipeline.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/context.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-field-write-collector/field-write-collector.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/array-detect-field-source-tracer/write-operation-trace.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-utils/info.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-result/analysis-data.hh \
  include/array-detect-core/type-field-hash.hh \
  include/array-detect-core/field-analysis.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/array-detect-field-source-tracer/field-source-variant.hh \
  include/array-detect-escape-use-collector/source-escape-collection.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-context/address-resolver.hh \
  include/array-detect-escape-evidence-synthesizer/escape-synthesizer.hh \
  include/ad-escaped-use/escaped-use.hh \
  include/ad-source-escape/source-escape.hh \
  include/ad-field-escape/field-escape.hh \
  include/array-detect-ownership-transfer-analyzer/ownership-transfer-analysis.hh \
  include/array-detect-owned-verdict-generator/owned-conclusion.hh \
  include/array-detect-capacity-field-associator/capacity-association.hh \
  include/array-detect-array-access-collector/array-access-collector.hh \
  include/array-detect-bound-condition-analyzer/bound-condition-analyzer.hh \
  include/array-detect-unified-result-aggregator/result-aggregator.hh \
  include/array-detect-utils/info-print.hh

obj/array-detect-field-source-tracer/write-operation-trace-driver.o: \
  src/array-detect-field-source-tracer/write-operation-trace-driver.cc \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-field-source-tracer/write-operation-trace.hh \
  include/array-detect-context/context.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-utils/info.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-result/analysis-data.hh \
  include/array-detect-core/type-field-hash.hh \
  include/array-detect-core/field-analysis.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/array-detect-field-source-tracer/field-source-variant.hh \
  include/array-detect-gcc-integration/gcc-ext-util.hh \
  include/array-detect-core/virtual-call-analysis.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-context/address-resolver.hh

obj/array-detect-field-source-tracer/write-operation-trace.o: \
  src/array-detect-field-source-tracer/write-operation-trace.cc \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-field-source-tracer/write-operation-trace.hh \
  include/array-detect-context/context.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-utils/info.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-result/analysis-data.hh \
  include/array-detect-core/type-field-hash.hh \
  include/array-detect-core/field-analysis.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/array-detect-field-source-tracer/field-source-variant.hh \
  include/array-detect-gcc-integration/gcc-ext-util.hh \
  include/array-detect-utils/info-print.hh

obj/array-detect-escape-use-collector/source-escape-collection.o: \
  src/array-detect-escape-use-collector/source-escape-collection.cc \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-escape-use-collector/source-escape-collection.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/context.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-context/address-resolver.hh \
  include/array-detect-result/analysis-data.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-utils/info.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/array-detect-core/type-field-hash.hh \
  include/array-detect-core/field-analysis.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/array-detect-field-source-tracer/write-operation-trace.hh \
  include/array-detect-field-source-tracer/field-source-variant.hh \
  include/array-detect-utils/info-print.hh

obj/array-detect-escape-evidence-synthesizer/escape-synthesizer.o: \
  src/array-detect-escape-evidence-synthesizer/escape-synthesizer.cc \
  include/array-detect-escape-evidence-synthesizer/escape-synthesizer.hh \
  include/ad-escaped-use/escaped-use.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/context.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-context/address-resolver.hh \
  include/array-detect-escape-use-collector/source-escape-collection.hh \
  include/array-detect-result/analysis-data.hh \
  include/ad-source-escape/source-escape.hh \
  include/ad-field-escape/field-escape.hh

obj/array-detect-ownership-transfer-analyzer/ownership-transfer-analysis.o: \
  src/array-detect-ownership-transfer-analyzer/ownership-transfer-analysis.cc \
  include/array-detect-ownership-transfer-analyzer/ownership-transfer-analysis.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/context.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-context/address-resolver.hh \
  include/array-detect-result/analysis-data.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-utils/info.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/array-detect-core/type-field-hash.hh \
  include/array-detect-core/field-analysis.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/array-detect-field-source-tracer/field-source-variant.hh \
  include/array-detect-gcc-integration/gcc-ext-util.hh \
  include/array-detect-utils/info-print.hh

obj/array-detect-owned-verdict-generator/owned-conclusion.o: \
  src/array-detect-owned-verdict-generator/owned-conclusion.cc \
  include/array-detect-owned-verdict-generator/owned-conclusion.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/context.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-utils/info.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/array-detect-result/analysis-data.hh \
  include/array-detect-core/type-field-hash.hh \
  include/array-detect-core/field-analysis.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/array-detect-field-source-tracer/field-source-variant.hh \
  include/array-detect-gcc-integration/gcc-ext-util.hh \
  include/array-detect-escape-use-collector/source-escape-collection.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-context/address-resolver.hh \
  include/array-detect-escape-evidence-synthesizer/escape-synthesizer.hh \
  include/ad-escaped-use/escaped-use.hh \
  include/ad-source-escape/source-escape.hh \
  include/ad-field-escape/field-escape.hh \
  include/array-detect-ownership-transfer-analyzer/ownership-transfer-analysis.hh \
  include/array-detect-utils/info-print.hh \
  include/array-detect-utils/string-utils.hh

obj/array-detect-capacity-field-associator/capacity-association.o: \
  src/array-detect-capacity-field-associator/capacity-association.cc \
  include/array-detect-capacity-field-associator/capacity-association.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/context.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-utils/info.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/array-detect-result/analysis-data.hh \
  include/array-detect-core/type-field-hash.hh \
  include/array-detect-core/field-analysis.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/array-detect-owned-verdict-generator/owned-conclusion.hh \
  include/array-detect-field-source-tracer/field-source-variant.hh \
  include/array-detect-array-access-collector/array-access-collector.hh \
  include/array-detect-gcc-integration/gcc-ext-util.hh \
  include/array-detect-utils/info-print.hh \
  include/array-detect-bound-condition-analyzer/bound-condition-analyzer.hh \
  include/array-detect-utils/string-utils.hh

obj/array-detect-array-access-collector/array-access-collector.o: \
  src/array-detect-array-access-collector/array-access-collector.cc \
  include/array-detect-array-access-collector/array-access-collector.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/context.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-utils/info.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/array-detect-result/analysis-data.hh \
  include/array-detect-core/type-field-hash.hh \
  include/array-detect-core/field-analysis.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/array-detect-gcc-integration/gcc-ext-util.hh \
  include/array-detect-utils/info-print.hh \
  include/array-detect-field-source-tracer/field-source-variant.hh \
  include/array-detect-utils/string-utils.hh

obj/array-detect-bound-condition-analyzer/bound-condition-analyzer.o: \
  src/array-detect-bound-condition-analyzer/bound-condition-analyzer.cc \
  include/array-detect-bound-condition-analyzer/bound-condition-analyzer.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/context.hh \
  include/array-detect-array-access-collector/array-access-collector.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-utils/info.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/array-detect-result/analysis-data.hh \
  include/array-detect-core/type-field-hash.hh \
  include/array-detect-core/field-analysis.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/array-detect-gcc-integration/gcc-ext-util.hh \
  include/array-detect-utils/info-print.hh \
  include/array-detect-field-source-tracer/field-source-variant.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-context/address-resolver.hh \
  include/array-detect-utils/string-utils.hh

obj/array-detect-unified-result-aggregator/result-aggregator.o: \
  src/array-detect-unified-result-aggregator/result-aggregator.cc \
  include/array-detect-unified-result-aggregator/result-aggregator.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/context.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-utils/info.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/array-detect-result/analysis-data.hh \
  include/array-detect-core/type-field-hash.hh \
  include/array-detect-core/field-analysis.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/array-detect-owned-verdict-generator/owned-conclusion.hh \
  include/array-detect-field-source-tracer/field-source-variant.hh \
  include/array-detect-capacity-field-associator/capacity-association.hh \
  include/array-detect-array-access-collector/array-access-collector.hh \
  include/array-detect-bound-condition-analyzer/bound-condition-analyzer.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-context/address-resolver.hh \
  include/array-detect-gcc-integration/gcc-ext-util.hh

obj/array-detect-lto-transform/lto-transform.o: \
  src/array-detect-lto-transform/lto-transform.cc \
  include/array-detect-lto-transform/lto-transform.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/context.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-gcc-integration/lto-summary.hh \
  include/array-detect-owned-verdict-generator/owned-conclusion.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-utils/info.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/array-detect-result/analysis-data.hh \
  include/array-detect-core/type-field-hash.hh \
  include/array-detect-core/field-analysis.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/array-detect-field-source-tracer/field-source-variant.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-context/address-resolver.hh \
  include/array-detect-context/context-init.hh \
  include/array-detect-pipeline-orchestrator/pipeline.hh

obj/ad-escaped-use/escaped-use.o: \
  src/ad-escaped-use/escaped-use.cc \
  include/ad-escaped-use/escaped-use.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/context.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-context/address-resolver.hh \
  include/array-detect-escape-use-collector/source-escape-collection.hh \
  include/array-detect-result/analysis-data.hh \
  include/array-detect-utils/info-print.hh \
  include/array-detect-field-source-tracer/field-source-variant.hh

obj/ad-source-escape/source-escape.o: \
  src/ad-source-escape/source-escape.cc \
  include/ad-source-escape/source-escape.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/context.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-context/address-resolver.hh \
  include/ad-escaped-use/escaped-use.hh \
  include/array-detect-escape-use-collector/source-escape-collection.hh \
  include/array-detect-result/analysis-data.hh \
  include/array-detect-utils/info-print.hh \
  include/array-detect-field-source-tracer/field-source-variant.hh

obj/ad-field-escape/field-escape.o: \
  src/ad-field-escape/field-escape.cc \
  include/ad-field-escape/field-escape.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-context/context.hh \
  include/array-detect-context/array-detect-context-gcc.hh \
  include/array-detect-utils/prelude.hh \
  include/array-detect-context/state.hh \
  config/array-detect-state.txt \
  include/array-detect-context/address-resolver.hh \
  include/ad-source-escape/source-escape.hh \
  include/ad-escaped-use/escaped-use.hh \
  include/array-detect-escape-use-collector/source-escape-collection.hh \
  include/array-detect-result/analysis-data.hh \
  include/array-detect-core/array-detector.hh \
  include/array-detect-utils/info.hh \
  include/array-detect-context/array-detect-context-gcc-interface.hh \
  include/array-detect-core/type-field-hash.hh \
  include/array-detect-core/field-analysis.hh \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-core/type-field-hash-traits.hh \
  include/array-detect-core/type-field-hashmap-traits.hh \
  include/array-detect-utils/info-print.hh \
  include/array-detect-field-source-tracer/field-source-variant.hh \
  include/array-detect-gcc-integration/gcc-ext-util.hh

obj/ad-field-wrapper/field-wrapper.o: \
  src/ad-field-wrapper/field-wrapper.cc \
  include/ad-field-wrapper/field-wrapper.hh \
  include/array-detect-gcc-integration/gcc-common.hh \
  include/array-detect-utils/prelude.hh

clean:
	rm -rf obj out

test-xercese: out/plugin-array-detect.dylib
	@echo "((cxx . \"/opt/homebrew/bin/g++-15\") (cflags \"-fplugin=out/plugin-array-detect.dylib\"))" | racket test-script/test-xercese.rkt

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
	mkdir -p obj/ad-escaped-use
	mkdir -p obj/ad-source-escape
	mkdir -p obj/ad-field-escape
	mkdir -p obj/ad-field-wrapper
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

