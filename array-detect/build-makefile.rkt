#!/usr/bin/env racket
#lang racket

(require file/glob)
(require "lib-config.rkt")
(require "exe-config.rkt")

;; ============================================================================
;; Platform Detection
;; ============================================================================

(define os-type (system-type 'os))

(define is-macos?
  (eq? os-type 'macosx))

(define is-linux?
  (eq? os-type 'unix))

;; ============================================================================
;; Compiler and Directory Setup
;; ============================================================================

;; 编译器路径检测：优先使用自定义配置，否则自动检测
(define (detect-compiler)
  (cond
    ;; 1. 使用自定义路径（如果设置了）
    [gcc-custom-path
     (let ([custom-cxx (if (absolute-path? gcc-custom-path)
                          gcc-custom-path
                          (find-executable-path gcc-custom-path))])
       (or custom-cxx
           (raise-user-error "Custom GCC path '~a' not found" gcc-custom-path)))]
    ;; 2. 自动检测
    [else
     (or (find-executable-path "g++-15")
         (find-executable-path "g++")
         (raise-user-error "Cannot find g++ compiler"))]))

(define cxx (detect-compiler))

(define object-dir "obj")
(module+ main (make-directory* object-dir))

(define out-dir "out")
(module+ main (make-directory* out-dir))

;; Platform-specific shared library extension
(define so-ext (if is-macos? "dylib" "so"))
(define output-so-path (build-path out-dir (format "plugin-array-detect.~a" so-ext)))

;; ============================================================================
;; Module Structure
;; ============================================================================

(define modules '("array-detect-core"
                  "array-detect-field-write-collector"
                  "array-detect-context"
                  "array-detect-gcc-integration"
                  "array-detect-utils"
                  "array-detect-pipeline-orchestrator"
                  "array-detect-field-source-tracer"
                  "array-detect-escape-use-collector"
                  "array-detect-escape-evidence-synthesizer"
                  "array-detect-ownership-transfer-analyzer"
                  "array-detect-owned-verdict-generator"
                  "array-detect-capacity-field-associator"
                  "array-detect-array-access-collector"
                  "array-detect-bound-condition-analyzer"
                  "array-detect-unified-result-aggregator"
                  "array-detect-lto-transform"))

(define s "src")
(define i "include")
(define c "config")

;; ============================================================================
;; Source and Object File Collection
;; ============================================================================

(define (find-rel x) (find-relative-path (current-directory) x))
(define sources
  (append-map
    (lambda (m)
      (map find-rel (glob (build-path s m "*.cc"))))
    modules))

(define (find-rel/src x) (find-relative-path "src" x))
(define (related-obj x) (build-path object-dir (path-replace-extension x ".o")))
(define targets (map (compose related-obj find-rel/src) sources))

;; ============================================================================
;; Compiler Path Detection
;; ============================================================================

(define (plugin-path-getter)
  (string-trim (with-output-to-string (lambda () (system* cxx "-print-file-name=plugin")))))
(define plugin-path (plugin-path-getter))

;; ============================================================================
;; Platform-Specific Linker Flags
;; ============================================================================

(define platform-linker-flags
  (if is-macos?
      '("-undefined" "dynamic_lookup")
      '()))  ; Linux doesn't need special undefined symbol handling

;; ============================================================================
;; Base Compiler Arguments
;; ============================================================================

(define base-args
  (append
    `("-fPIC"
      "-fno-rtti"
      "-shared"
      "-I" ,c
      "-I" ,plugin-path
      "-I" ,(path->string (build-path plugin-path "include"))
      ;; Add include paths for each module
      "-I" ,(path->string (build-path "include/array-detect-core"))
      "-I" ,(path->string (build-path "include/array-detect-field-write-collector"))
      "-I" ,(path->string (build-path "include/array-detect-context"))
      "-I" ,(path->string (build-path "include/array-detect-gcc-integration"))
      "-I" ,(path->string (build-path "include/array-detect-utils"))
      "-I" ,(path->string (build-path "include/array-detect-result"))
      "-I" ,(path->string (build-path "include/array-detect-pipeline-orchestrator"))
      "-I" ,(path->string (build-path "include/array-detect-field-source-tracer"))
      "-I" ,(path->string (build-path "include/array-detect-escape-use-collector"))
      "-I" ,(path->string (build-path "include/array-detect-escape-evidence-synthesizer"))
      "-I" ,(path->string (build-path "include/array-detect-ownership-transfer-analyzer"))
      "-I" ,(path->string (build-path "include/array-detect-owned-verdict-generator"))
      "-I" ,(path->string (build-path "include/array-detect-capacity-field-associator"))
      "-I" ,(path->string (build-path "include/array-detect-array-access-collector"))
      "-I" ,(path->string (build-path "include/array-detect-bound-condition-analyzer"))
      "-I" ,(path->string (build-path "include/array-detect-unified-result-aggregator"))
      "-I" ,(path->string (build-path "include/array-detect-lto-transform")))
    platform-linker-flags
    '("-Wall"
      "-Wextra"
      "-std=c++17"
      "-g"
      "-O2")))

;; ============================================================================
;; Library Dependencies - Platform-Specific
;; ============================================================================

;; 通用函数：从 include 和 lib 路径生成编译器参数
;; 参数可以是路径字符串或 #f（表示跳过）
(define (make-lib-args include-path lib-path)
  (append
    (if include-path `("-I" ,include-path) '())
    (if lib-path `("-L" ,lib-path) '())))

;; GMP library
(define (gmp/args)
  (with-handlers ([exn? (lambda (_) '())])
    (cond
      ;; 1. 使用自定义路径（如果至少设置了一个）
      [(or gmp-custom-include-path gmp-custom-lib-path)
       (make-lib-args gmp-custom-include-path gmp-custom-lib-path)]
      ;; 2. 使用 pkg-config 自动检测
      [else
       (append
         (string-split (string-trim (with-output-to-string
           (lambda () (system "pkg-config --libs gmp")))))
         (string-split (string-trim (with-output-to-string
           (lambda () (system "pkg-config --cflags gmp"))))))])))


;; MPC library
(define (mpc/args)
  (with-handlers ([exn? (lambda (_) '())])
    (cond
      ;; 1. 使用自定义路径（如果至少设置了一个）
      [(or mpc-custom-include-path mpc-custom-lib-path)
       (make-lib-args mpc-custom-include-path mpc-custom-lib-path)]
      ;; 2. macOS: 使用 Homebrew 自动检测
      [is-macos?
       (let ([mpc-directory
              (string-trim (with-output-to-string
                (lambda () (system "brew --prefix libmpc"))))])
         `("-I" ,(path->string (build-path mpc-directory "include"))
           "-L" ,(path->string (build-path mpc-directory "lib"))))]
      ;; 3. Linux: 使用 pkg-config 自动检测
      [else
       (append
         (string-split (string-trim (with-output-to-string
           (lambda () (system "pkg-config --libs mpc")))))
         (string-split (string-trim (with-output-to-string
           (lambda () (system "pkg-config --cflags mpc"))))))])))


;; MPFR library
(define (mpfr/args)
  (with-handlers ([exn? (lambda (_) '())])
    (cond
      ;; 1. 使用自定义路径（如果至少设置了一个）
      [(or mpfr-custom-include-path mpfr-custom-lib-path)
       (make-lib-args mpfr-custom-include-path mpfr-custom-lib-path)]
      ;; 2. macOS: 使用 Homebrew 自动检测
      [is-macos?
       (let ([mpfr-directory
              (string-trim (with-output-to-string
                (lambda () (system "brew --prefix mpfr"))))])
         `("-I" ,(path->string (build-path mpfr-directory "include"))
           "-L" ,(path->string (build-path mpfr-directory "lib"))))]
      ;; 3. Linux: 使用 pkg-config 自动检测
      [else
       (append
         (string-split (string-trim (with-output-to-string
           (lambda () (system "pkg-config --libs mpfr")))))
         (string-split (string-trim (with-output-to-string
           (lambda () (system "pkg-config --cflags mpfr"))))))])))


;; Combine all arguments
(define args^ (append base-args (gmp/args) (mpc/args) (mpfr/args)))

;; ============================================================================
;; Makefile Generation Functions
;; ============================================================================

(define (write-compiles)
  (for ([s sources] [t targets])
    (printf "~a:~n" t)
    (printf "\t")
    (printf "~a -c" cxx)
    (for ([a args^]) (printf " ~s" a))
    (printf " ~s" (path->string s))
    (printf " -o ~s" (path->string t))
    (printf "~n~n")))

(define (write-clean)
  (printf "clean:~n")
  (printf "\trm -rv ~a~n" (build-path object-dir "*"))
  (printf "\trm -rv ~a~n" (build-path out-dir "*"))
  (printf "~n"))

(define (write-plugin)
  (printf "~a:~n" output-so-path)
  (printf "\t~a" cxx)
  (for ([a args^]) (printf " ~s" a))
  (for ([o targets]) (printf " ~s" (path->string o)))
  (printf " -o ~s" (path->string output-so-path))
  (printf "~n~n"))

(define (write-test-xercese)
  (define test-dir (simplify-path (build-path (current-directory) "../test/test-xercese")))
  (define abs-plugin-path (simplify-path (build-path (current-directory) output-so-path)))
  (define rel-plugin-path (path->string (find-relative-path test-dir abs-plugin-path)))
  (printf "test: ~a~n" output-so-path)
  (define plugin-arg (format "-fplugin=~a" rel-plugin-path))
  (define input `((cxx . ,(path->string cxx)) (cflags ,plugin-arg)))
  (printf "\t@(cd ../test/test-xercese && mkdir -p out && echo ~s | racket build-xercese.rkt)~n"
          (~s input))
  (printf "~n"))

(define (calc-dependency filename)
  (define c (make-custodian))
  (with-handlers ([exn:fail? (lambda (_e) (custodian-shutdown-all c) (raise _e))])
    (parameterize ([current-custodian c])
      (match-define `(,i ,o ,p ,i2 ,h) (apply process* (append `(,cxx . ,args^) `("-M" ,filename))))
      (h 'wait)
      (define is (sequence->list (in-lines i)))
      (append*
        (match (h 'status)
          ['done-ok
            (for/list ([j (in-naturals)] [f (in-list is)])
              (define f^ (cond [(equal? (+ j 1) (length is)) f]
                               [else (string-trim f " \\" #:left? #f)]))
              (define f^^ (cond [(equal? j 0)
                  (define loc (string-find f^ ": "))
                  (cond [loc (substring f^ (+ loc 2))] [else ""])]
                [else (string-trim f^ #:right? #f #:repeat? #t)]))
              (string-split f^^))]
          ['done-error
            (for ([el (in-lines i2)]) (eprintf "\t~a~n" el)) (eprintf "~n")
            (raise-user-error 'calc-dependency "failed to calc '~a' by ~a'" filename cxx)])))))

(define (exists-up? path)
  (match-define-values (base name must-be-dir?) (split-path path))
  (match* (base name)
    [(_ 'up) #t]
    [((or #f 'relative) _) #f]
    [(_ _) (exists-up? base)]))

(define (write-deps2)
  (for ([s sources] [t targets])
    (define ds (calc-dependency s))
    (define ds^ (map (compose find-rel simple-form-path) ds))
    (define ds^^ (filter (compose not exists-up?) ds^))
    (printf "~a: \\~n" t)
    (for ([d ds^^])
      (printf "  ~a \\~n" d))
    (printf "~n"))
  (printf "~a:" output-so-path)
  (for ([t targets]) (printf " ~a" t))
  (printf "~n~n"))

(define (write-prepare)
  (printf "prepare:~n")
  (for ([m modules])
    (printf "\tmkdir -p ~a~n" (build-path object-dir m)))
  (printf "\tmkdir -p ~a~n" (build-path out-dir))
  (printf "~n"))

(define a-tests '("test-simple-ptr-field"
                  "test-simple-virtual-call"
                  "test-simple-ptr-copy-escape"
                  "test-bound-check"
                  "test-bound-read"
                  ))

(define (write-test name)
  (define test-dir (simplify-path (build-path (current-directory) "../test" name)))
  (define abs-plugin-path (simplify-path (build-path (current-directory) output-so-path)))
  (define rel-plugin-path (path->string (find-relative-path test-dir abs-plugin-path)))
  (printf "~a: ~a~n" name output-so-path)
  (define plugin-arg (format "-fplugin=~a" rel-plugin-path))
  (define input `((cxx . ,(path->string cxx)) (cflags ,plugin-arg)))
  (printf "\t@(cd ../test/~a && mkdir -p out && echo ~s | racket build.rkt)~n"
          name (~s input))
  (printf "~n"))

(define (write-lto-test name)
  (define test-dir (simplify-path (build-path (current-directory) "../test" name)))
  (define abs-plugin-path (simplify-path (build-path (current-directory) output-so-path)))
  (define rel-plugin-path (path->string (find-relative-path test-dir abs-plugin-path)))
  (printf "~a: ~a~n" name output-so-path)
  (define plugin-arg (format "-fplugin=~a" rel-plugin-path))
  (define input `((cxx . ,(path->string cxx)) (cflags ,plugin-arg "-flto")))
  (printf "\t@(cd ../test/~a && mkdir -p out && echo ~s | racket build.rkt)~n"
          name (~s input))
  (printf "~n"))

(define (write-tests)
  (for ([a a-tests]) (write-test a)))

(define a-lto-tests '("test-lto"))

(define (write-lto-tests)
  (for ([a a-lto-tests]) (write-lto-test a)))

;; ============================================================================
;; Platform Info (for debugging)
;; ============================================================================

(define (write-platform-info)
  (printf "# Generated Makefile for ~a~n" (if is-macos? "macOS" "Linux"))
  (printf "# Compiler: ~a~n" cxx)
  (printf "# Shared library extension: .~a~n" so-ext)
  (printf "# Platform-specific flags: ~a~n"
    (if (null? platform-linker-flags) "none" (string-join platform-linker-flags " ")))
  (printf "~n"))


;; ============================================================================
;; Main Entry Point
;; ============================================================================

(define (write-makefile)
  (call-with-atomic-output-file "Makefile" (lambda (o _p)
    (parameterize ([current-output-port o])
      (write-platform-info)
      (write-plugin)
      (write-compiles)
      (write-deps2)
      (write-clean)
      (write-test-xercese)
      (write-prepare)
      (write-tests)
      (write-lto-tests)
    ))))

(module+ main (write-makefile))
