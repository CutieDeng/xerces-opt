#!/usr/bin/env racket
#lang racket

(struct Config
  (cc
   cflags
   modules
   output-so
   src-path
   include-path
   src-config-path
   object-dir
   out-dir
   tests
   lto-tests
   platform-name
   so-ext
   platform-linker-flags)
  #:transparent)

(provide
  (struct-out Config)
  make-config-with-cc
  write-makefile*)

(require file/glob)
(require "lib-config.rkt")
(require "exe-config.rkt")

;; ============================================================================
;; Compiler Path Detection
;; ============================================================================

(define (plugin-path-getter cc)
  (string-trim
    (with-output-to-string
      (lambda () (system* cc "-print-file-name=plugin")))))

;; ============================================================================
;; Library Dependencies - Platform-Specific
;; ============================================================================

(define (make-lib-args include-path lib-path)
  (append
    (if include-path `("-I" ,include-path) '())
    (if lib-path `("-L" ,lib-path) '())))

(define (gmp/args is-macos?)
  (with-handlers ([exn? (lambda (_) '())])
    (cond
      [(or gmp-custom-include-path gmp-custom-lib-path)
       (make-lib-args gmp-custom-include-path gmp-custom-lib-path)]
      [else
       (append
         (string-split (string-trim (with-output-to-string
           (lambda () (system "pkg-config --libs gmp")))))
         (string-split (string-trim (with-output-to-string
           (lambda () (system "pkg-config --cflags gmp"))))))])))

(define (mpc/args is-macos?)
  (with-handlers ([exn? (lambda (_) '())])
    (cond
      [(or mpc-custom-include-path mpc-custom-lib-path)
       (make-lib-args mpc-custom-include-path mpc-custom-lib-path)]
      [is-macos?
       (let ([mpc-directory
              (string-trim (with-output-to-string
                (lambda () (system "brew --prefix libmpc"))))])
         `("-I" ,(~a (build-path mpc-directory "include"))
           "-L" ,(~a (build-path mpc-directory "lib"))))]
      [else
       (append
         (string-split (string-trim (with-output-to-string
           (lambda () (system "pkg-config --libs mpc")))))
         (string-split (string-trim (with-output-to-string
           (lambda () (system "pkg-config --cflags mpc"))))))])))

(define (mpfr/args is-macos?)
  (with-handlers ([exn? (lambda (_) '())])
    (cond
      [(or mpfr-custom-include-path mpfr-custom-lib-path)
       (make-lib-args mpfr-custom-include-path mpfr-custom-lib-path)]
      [is-macos?
       (let ([mpfr-directory
              (string-trim (with-output-to-string
                (lambda () (system "brew --prefix mpfr"))))])
         `("-I" ,(~a (build-path mpfr-directory "include"))
           "-L" ,(~a (build-path mpfr-directory "lib"))))]
      [else
       (append
         (string-split (string-trim (with-output-to-string
           (lambda () (system "pkg-config --libs mpfr")))))
         (string-split (string-trim (with-output-to-string
           (lambda () (system "pkg-config --cflags mpfr"))))))])))

;; ============================================================================
;; Makefile Generation Helpers
;; ============================================================================

(define (collect-sources cfg)
  (define (find-rel x) (find-relative-path (current-directory) x))
  (append-map
    (lambda (m)
      (map find-rel (glob (build-path (Config-src-path cfg) m "*.cc"))))
    (Config-modules cfg)))

(define (collect-targets cfg sources)
  (define (find-rel/src x) (find-relative-path (Config-src-path cfg) x))
  (define (related-obj x)
    (build-path (Config-object-dir cfg) (path-replace-extension x ".o")))
  (map (compose related-obj find-rel/src) sources))

(define (write-platform-info cfg)
  (printf "# Generated Makefile for ~a~n" (Config-platform-name cfg))
  (printf "# Compiler: ~a~n" (Config-cc cfg))
  (printf "# Shared library extension: .~a~n" (Config-so-ext cfg))
  (printf "# Platform-specific flags: ~a~n"
          (if (null? (Config-platform-linker-flags cfg))
              "none"
              (string-join (Config-platform-linker-flags cfg) " ")))
  (printf "~n"))

(define (write-compiles cfg sources targets)
  (for ([s sources] [t targets])
    (printf "~a:~n" t)
    (printf "\t~a -c" (Config-cc cfg))
    (for ([a (Config-cflags cfg)]) (printf " ~s" a))
    (printf " ~s" (~a s))
    (printf " -o ~s" (~a t))
    (printf "~n~n")))

(define (write-clean cfg)
  (printf "clean:~n")
  (printf "\trm -rv ~a~n" (build-path (Config-object-dir cfg) "*"))
  (printf "\trm -rv ~a~n" (build-path (Config-out-dir cfg) "*"))
  (printf "~n"))

(define (write-plugin cfg targets)
  (printf "~a:~n" (Config-output-so cfg))
  (printf "\t~a" (Config-cc cfg))
  (for ([a (Config-cflags cfg)]) (printf " ~s" a))
  (for ([o targets]) (printf " ~s" (~a o)))
  (printf " -o ~s" (~a (Config-output-so cfg)))
  (printf "~n~n"))

(define (write-test-xercese cfg)
  (define test-dir (simplify-path (build-path (current-directory) "../test/test-xercese")))
  (define abs-plugin-path (simplify-path (build-path (current-directory) (Config-output-so cfg))))
  (define rel-plugin-path (~a (find-relative-path test-dir abs-plugin-path)))
  (printf "test: ~a~n" (Config-output-so cfg))
  (define plugin-arg (format "-fplugin=~a" rel-plugin-path))
  (define input `((gcc-bin . ,(~a (Config-cc cfg))) (cflags ,plugin-arg)))
  (printf "\t@(cd ../test/test-xercese && mkdir -p out && echo ~s | racket build-xercese.rkt)~n"
          (~s input))
  (printf "~n"))

(define (normalize-deps-output lines)
  (define buf
    (string-join
      (for/list ([line lines])
        (string-trim line " \\" #:left? #f))
      " "))
  (define loc (string-find buf ": "))
  (if loc
      (string-trim (substring buf (+ loc 2)))
      ""))

(define (dependency-paths cfg filename target)
  (define c (make-custodian))
  (with-handlers ([exn:fail? (lambda (_e) (custodian-shutdown-all c) (raise _e))])
    (parameterize ([current-custodian c])
      (match-define `(,i ,_o ,_p ,i2 ,h)
        (apply process* (append `(,(Config-cc cfg) . ,(Config-cflags cfg))
                                `("-MM" "-MT" ,(~a target) ,filename))))
      (h 'wait)
      (define is (sequence->list (in-lines i)))
      (match (h 'status)
        ['done-ok
         (define dep-str (normalize-deps-output is))
         (filter (lambda (s) (not (string=? s "")))
                 (string-split dep-str))]
        ['done-error
         (for ([el (in-lines i2)]) (eprintf "\t~a~n" el))
         (eprintf "~n")
         (raise-user-error 'calc-dependency
                           "failed to calc '~a' by ~a'"
                           filename
                           (Config-cc cfg))]))))

(define (write-deps2 cfg sources targets)
  (for ([s sources] [t targets])
    (define deps (dependency-paths cfg s t))
    (define cwd (simplify-path (current-directory)))
    (define (in-project? p)
      (define abs
        (simplify-path
          (if (relative-path? p)
              (build-path cwd p)
              p)))
      (string-prefix? (~a abs) (~a cwd)))
    (define deps-in-project
      (filter in-project? (map simple-form-path deps)))
    (printf "~a:" t)
    (for ([d deps-in-project])
      (printf " \\\n  ~a" d))
    (printf "~n")
    (printf "~n"))
  (printf "~a:" (Config-output-so cfg))
  (for ([t targets]) (printf " ~a" t))
  (printf "~n~n"))

(define (write-prepare cfg)
  (printf "prepare:~n")
  (for ([m (Config-modules cfg)])
    (printf "\tmkdir -p ~a~n" (build-path (Config-object-dir cfg) m)))
  (printf "\tmkdir -p ~a~n" (build-path (Config-out-dir cfg)))
  (printf "~n"))

(define (write-test cfg name)
  (define test-dir (simplify-path (build-path (current-directory) "../test" name)))
  (define abs-plugin-path (simplify-path (build-path (current-directory) (Config-output-so cfg))))
  (define rel-plugin-path (~a (find-relative-path test-dir abs-plugin-path)))
  (printf "~a: ~a~n" name (Config-output-so cfg))
  (define plugin-arg (format "-fplugin=~a" rel-plugin-path))
  (define input `((cxx . ,(~a (Config-cc cfg))) (cflags ,plugin-arg)))
  (printf "\t@(cd ../test/~a && mkdir -p out && echo ~s | racket build.rkt)~n"
          name
          (~s input))
  (printf "~n"))

(define (write-lto-test cfg name)
  (define test-dir (simplify-path (build-path (current-directory) "../test" name)))
  (define abs-plugin-path (simplify-path (build-path (current-directory) (Config-output-so cfg))))
  (define rel-plugin-path (~a (find-relative-path test-dir abs-plugin-path)))
  (printf "~a: ~a~n" name (Config-output-so cfg))
  (define plugin-arg (format "-fplugin=~a" rel-plugin-path))
  (define input `((cxx . ,(~a (Config-cc cfg))) (cflags ,plugin-arg "-flto")))
  (printf "\t@(cd ../test/~a && mkdir -p out && echo ~s | racket build.rkt)~n"
          name
          (~s input))
  (printf "~n"))

(define (write-tests cfg)
  (for ([a (Config-tests cfg)]) (write-test cfg a)))

(define (write-lto-tests cfg)
  (for ([a (Config-lto-tests cfg)]) (write-lto-test cfg a)))

;; ============================================================================
;; Global Config
;; ============================================================================

(define (make-config-with-cc cc-path)
  (define os-type (system-type 'os))
  (define is-macos? (eq? os-type 'macosx))
  (define gcc-bin cc-path)
  (define object-dir "obj")
  (define out-dir "out")
  (define so-ext (if is-macos? "dylib" "so"))
  (define output-so-path (build-path out-dir (format "plugin-array-detect.~a" so-ext)))
  (define modules
    '("array-detect-core"
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
  (define src-path "src")
  (define include-path "include")
  (define src-config-path "config")
  (define platform-linker-flags
    (if is-macos?
        '("-undefined" "dynamic_lookup")
        '()))
  (define plugin-path (plugin-path-getter gcc-bin))
  (define module-include-paths
    (for/list ([m modules])
      (~a (build-path include-path m))))
  (define extra-include-paths
    (list (~a (build-path include-path "array-detect-result"))))
  (define base-args
    (append
      `("-fPIC"
        "-fno-rtti"
        "-shared"
        "-I" ,(~a src-config-path)
        "-I" ,plugin-path
        "-I" ,(~a (build-path plugin-path "include")))
      (append-map (lambda (p) `("-I" ,p))
                  (append module-include-paths extra-include-paths))
      platform-linker-flags
      '("-Wall"
        "-Wextra"
        "-std=c++17"
        "-g"
        "-O2")))
  (Config
    gcc-bin
    (append base-args (gmp/args is-macos?) (mpc/args is-macos?) (mpfr/args is-macos?))
    modules
    output-so-path
    src-path
    include-path
    src-config-path
    object-dir
    out-dir
    '("test-simple-ptr-field"
      "test-simple-virtual-call"
      "test-simple-ptr-copy-escape"
      "test-bound-check"
      "test-bound-read")
    '("test-lto")
    (if is-macos? "macOS" "Linux")
    so-ext
    platform-linker-flags))

(define cfg (make-config-with-cc (find-executable-path "g++-15")))

;; ============================================================================
;; Main Entry Point
;; ============================================================================

(define (write-makefile* cfg)
  (make-directory* (Config-object-dir cfg))
  (make-directory* (Config-out-dir cfg))
  (define sources (collect-sources cfg))
  (define targets (collect-targets cfg sources))
  (call-with-atomic-output-file "Makefile"
    (lambda (o _p)
      (parameterize ([current-output-port o])
        (write-platform-info cfg)
        (write-plugin cfg targets)
        (write-compiles cfg sources targets)
        (write-deps2 cfg sources targets)
        (write-clean cfg)
        (write-test-xercese cfg)
        (write-prepare cfg)
        (write-tests cfg)
        (write-lto-tests cfg)))))

(module+ main
  (define (write-makefile cfg)
    (write-makefile* cfg))
  (write-makefile cfg))
