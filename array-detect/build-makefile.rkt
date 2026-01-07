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
   platform-linker-flags
   dep-jobs
   gmp-include-path
   gmp-lib-path
   mpc-include-path
   mpc-lib-path
   mpfr-include-path
   mpfr-lib-path)
  #:transparent)

(provide
  (struct-out Config)
  make-config-with-cc
  get-default-config
  write-makefile*)

(require file/glob)

;; ============================================================================
;; Helpers
;; ============================================================================

(define (cfg-force v)
  (if (promise? v) (force v) v))

(define (cfg-ref cfg accessor)
  (cfg-force (accessor cfg)))

(define (cfg-cflags cfg)
  (cfg-ref cfg Config-cflags))

(define (cfg-lib-args cfg)
  (append (make-lib-args (cfg-ref cfg Config-gmp-include-path)
                         (cfg-ref cfg Config-gmp-lib-path))
          (make-lib-args (cfg-ref cfg Config-mpc-include-path)
                         (cfg-ref cfg Config-mpc-lib-path))
          (make-lib-args (cfg-ref cfg Config-mpfr-include-path)
                         (cfg-ref cfg Config-mpfr-lib-path))))

;; ============================================================================
;; Compiler Path Detection
;; ============================================================================

(define (find-default-compiler)
  (define gcc-15 (find-executable-path "g++-15"))
  (define gcc (find-executable-path "g++"))
  (cond
    [gcc-15 gcc-15]
    [gcc gcc]
    [else
     (raise-user-error 'build-makefile
                       "Cannot find g++ compiler (g++-15 or g++)")]))

(define (plugin-path-getter cc)
  (define out
    (string-trim
      (with-output-to-string
        (lambda () (system* cc "-print-file-name=plugin")))))
  (if (string=? out "")
      (raise-user-error 'build-makefile "Failed to locate GCC plugin path")
      out))

;; ============================================================================
;; Library Dependencies - Platform-Specific
;; ============================================================================

(define (make-lib-args include-path lib-path)
  (append
    (if include-path `("-I" ,include-path) '())
    (if lib-path `("-L" ,lib-path) '())))

(define (pkg-config-first-flag cmd prefix)
  (with-handlers ([exn? (lambda (_) #f)])
    (define flags
      (string-split
        (string-trim
          (with-output-to-string (lambda () (system cmd))))))
    (define hit (findf (lambda (s) (string-prefix? prefix s)) flags))
    (and hit (substring hit (string-length prefix)))))

(define (brew-prefix name)
  (with-handlers ([exn? (lambda (_) #f)])
    (string-trim
      (with-output-to-string (lambda () (system (format "brew --prefix ~a" name)))))))

(define (gmp/args)
  (with-handlers ([exn? (lambda (_) (values #f #f))])
    (values
      (pkg-config-first-flag "pkg-config --cflags gmp" "-I")
      (pkg-config-first-flag "pkg-config --libs gmp" "-L"))))

(define (mpc/args is-macos?)
  (with-handlers ([exn? (lambda (_) (values #f #f))])
    (if is-macos?
        (let ([mpc-directory (brew-prefix "libmpc")])
          (values
            (and mpc-directory (~a (build-path mpc-directory "include")))
            (and mpc-directory (~a (build-path mpc-directory "lib")))))
        (values
          (pkg-config-first-flag "pkg-config --cflags mpc" "-I")
          (pkg-config-first-flag "pkg-config --libs mpc" "-L")))))

(define (mpfr/args is-macos?)
  (with-handlers ([exn? (lambda (_) (values #f #f))])
    (if is-macos?
        (let ([mpfr-directory (brew-prefix "mpfr")])
          (values
            (and mpfr-directory (~a (build-path mpfr-directory "include")))
            (and mpfr-directory (~a (build-path mpfr-directory "lib")))))
        (values
          (pkg-config-first-flag "pkg-config --cflags mpfr" "-I")
          (pkg-config-first-flag "pkg-config --libs mpfr" "-L")))))

;; ============================================================================
;; Makefile Generation Helpers
;; ============================================================================

(define (collect-sources cfg)
  (define (find-rel x) (find-relative-path (current-directory) x))
  (append-map
    (lambda (m)
      (define files (glob (build-path (cfg-ref cfg Config-src-path) m "*.cc")))
      (map find-rel (sort files path<?)))
    (cfg-ref cfg Config-modules)))

(define (collect-targets cfg sources)
  (define (find-rel/src x) (find-relative-path (cfg-ref cfg Config-src-path) x))
  (define (related-obj x)
    (build-path (cfg-ref cfg Config-object-dir) (path-replace-extension x ".o")))
  (map (compose related-obj find-rel/src) sources))

(define (write-platform-info cfg)
  (printf "# Generated Makefile for ~a~n" (cfg-ref cfg Config-platform-name))
  (printf "# Compiler: ~a~n" (cfg-ref cfg Config-cc))
  (printf "# Shared library extension: .~a~n" (cfg-ref cfg Config-so-ext))
  (printf "# Platform-specific flags: ~a~n"
          (if (null? (cfg-ref cfg Config-platform-linker-flags))
              "none"
              (string-join (cfg-ref cfg Config-platform-linker-flags) " ")))
  (printf "~n"))

(define (write-phony cfg)
  (define phony
    (append
      '("all" "clean" "prepare" "test-xercese")
      (cfg-ref cfg Config-tests)
      (cfg-ref cfg Config-lto-tests)))
  (printf ".PHONY: ~a~n~n" (string-join phony " ")))

(define (write-compiles cfg sources targets)
  (for ([s sources] [t targets])
    (printf "~a:~n" (~a t))
    (printf "\t~a -c" (cfg-ref cfg Config-cc))
    (for ([a (cfg-cflags cfg)]) (printf " ~s" a))
    (for ([a (cfg-lib-args cfg)]) (printf " ~s" a))
    (printf " ~s" (~a s))
    (printf " -o ~s" (~a t))
    (printf "~n~n")))

(define (write-clean cfg)
  (printf "clean:~n")
  (printf "\trm -rf ~a ~a~n"
          (cfg-ref cfg Config-object-dir)
          (cfg-ref cfg Config-out-dir))
  (printf "~n"))

(define (write-plugin cfg targets)
  (printf "all: ~a~n~n" (cfg-ref cfg Config-output-so))
  (printf "~a:" (cfg-ref cfg Config-output-so))
  (for ([o targets]) (printf " ~a" (~a o)))
  (printf "~n")
  (printf "\t~a" (cfg-ref cfg Config-cc))
  (for ([a (cfg-cflags cfg)]) (printf " ~s" a))
  (for ([a (cfg-lib-args cfg)]) (printf " ~s" a))
  (for ([o targets]) (printf " ~s" (~a o)))
  (printf " -o ~s" (~a (cfg-ref cfg Config-output-so)))
  (printf "~n~n"))

(define (write-test-xercese cfg)
  (define test-dir (simplify-path (build-path (current-directory) "../test/test-xercese")))
  (define abs-plugin-path (simplify-path (build-path (current-directory) (cfg-ref cfg Config-output-so))))
  (define rel-plugin-path (~a (find-relative-path test-dir abs-plugin-path)))
  (printf "test-xercese: ~a~n" (cfg-ref cfg Config-output-so))
  (define plugin-arg (format "-fplugin=~a" rel-plugin-path))
  (define input `((cxx . ,(~a (cfg-ref cfg Config-cc))) (cflags ,plugin-arg)))
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

(define (dependency-paths cc flags filename target)
  (define c (make-custodian))
  (with-handlers ((exn:fail?
                   (lambda (_e)
                     (custodian-shutdown-all c)
                     (raise _e))))
    (parameterize ((current-custodian c))
      (match-define `(,i ,_o ,_p ,i2 ,h)
        (apply process* (append `(,cc . ,flags)
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
                           cc)]))))

(define (deps-in-project deps)
  (define cwd (simplify-path (current-directory)))
  (define (in-project? p)
    (define abs
      (simplify-path
        (if (relative-path? p)
            (build-path cwd p)
            p)))
    (string-prefix? (~a abs) (~a cwd)))
  (filter in-project? (map simple-form-path deps)))

(define (calc-deps-parallel cfg sources targets)
  (define total (length sources))
  (cond
    [(zero? total) '()]
    [else
     (define job-count
       (let ([v (cfg-ref cfg Config-dep-jobs)])
         (if (and (integer? v) (> v 0)) v 1)))
     (define workers (max 1 (min total job-count)))
     (define cc (cfg-ref cfg Config-cc))
     (define flags (append (cfg-cflags cfg) (cfg-lib-args cfg)))
     (define src-vec (list->vector sources))
     (define tgt-vec (list->vector targets))
     (define dep-vec (make-vector total))
     (define next-index (box 0))
     (define lock (make-semaphore 1))
     (define (get-next-index)
       (semaphore-wait lock)
       (define i (unbox next-index))
       (set-box! next-index (add1 i))
       (semaphore-post lock)
       i)
     (define (worker)
       (let loop ()
           (define i (get-next-index))
           (when (< i total)
             (define deps
               (with-handlers ([exn? (lambda (e) e)])
                 (dependency-paths cc flags
                                   (vector-ref src-vec i)
                                   (vector-ref tgt-vec i))))
             (vector-set! dep-vec i deps)
             (loop))))
     (define threads
       (for/list ([i (in-range workers)])
         (thread worker)))
     (for ([t threads]) (thread-wait t))
     (define dep-list (vector->list dep-vec))
     (define exn (findf exn? dep-list))
     (when exn (raise exn))
     dep-list]))

(define (write-deps cfg targets deps-list)
  (for ([t targets] [deps deps-list])
    (define deps-in-proj (deps-in-project deps))
    (printf "~a:" t)
    (for ([d deps-in-proj])
      (printf " \\\n  ~a" d))
    (printf "~n~n")))

(define (write-prepare cfg)
  (printf "prepare:~n")
  (for ([m (cfg-ref cfg Config-modules)])
    (printf "\tmkdir -p ~a~n" (build-path (cfg-ref cfg Config-object-dir) m)))
  (printf "\tmkdir -p ~a~n" (build-path (cfg-ref cfg Config-out-dir)))
  (printf "~n"))

(define (write-test cfg name)
  (define test-dir (simplify-path (build-path (current-directory) "../test" name)))
  (define abs-plugin-path (simplify-path (build-path (current-directory) (cfg-ref cfg Config-output-so))))
  (define rel-plugin-path (~a (find-relative-path test-dir abs-plugin-path)))
  (printf "~a: ~a~n" name (cfg-ref cfg Config-output-so))
  (define plugin-arg (format "-fplugin=~a" rel-plugin-path))
  (define input `((cxx . ,(~a (cfg-ref cfg Config-cc))) (cflags ,plugin-arg)))
  (printf "\t@(cd ../test/~a && mkdir -p out && echo ~s | racket build.rkt)~n"
          name
          (~s input))
  (printf "~n"))

(define (write-lto-test cfg name)
  (define test-dir (simplify-path (build-path (current-directory) "../test" name)))
  (define abs-plugin-path (simplify-path (build-path (current-directory) (cfg-ref cfg Config-output-so))))
  (define rel-plugin-path (~a (find-relative-path test-dir abs-plugin-path)))
  (printf "~a: ~a~n" name (cfg-ref cfg Config-output-so))
  (define plugin-arg (format "-fplugin=~a" rel-plugin-path))
  (define input `((cxx . ,(~a (cfg-ref cfg Config-cc))) (cflags ,plugin-arg "-flto")))
  (printf "\t@(cd ../test/~a && mkdir -p out && echo ~s | racket build.rkt)~n"
          name
          (~s input))
  (printf "~n"))

(define (write-tests cfg)
  (for ([a (cfg-ref cfg Config-tests)]) (write-test cfg a)))

(define (write-lto-tests cfg)
  (for ([a (cfg-ref cfg Config-lto-tests)]) (write-lto-test cfg a)))

;; ============================================================================
;; Global Config
;; ============================================================================

(define (make-config-with-cc cc-path dep-jobs)
  (define os-type (system-type 'os))
  (define is-macos? (eq? os-type 'macosx))
  (define gcc-bin
    (or cc-path
        (raise-user-error 'build-makefile
                          "Cannot find g++ compiler (g++-15 or g++)")))
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
  (define module-include-paths
    (for/list ([m modules])
      (~a (build-path include-path m))))
  (define extra-include-paths
    (list (~a (build-path include-path "array-detect-result"))))
  (define base-args-promise
    (delay
      (define plugin-path (plugin-path-getter gcc-bin))
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
          "-O2"))))
  (letrec ([gmp-promise (delay (let-values ([(i l) (gmp/args)]) (cons i l)))]
           [mpc-promise (delay (let-values ([(i l) (mpc/args is-macos?)]) (cons i l)))]
           [mpfr-promise (delay (let-values ([(i l) (mpfr/args is-macos?)]) (cons i l)))]
           [cfg
            (Config
              (delay gcc-bin)
              (delay (force base-args-promise))
              (delay modules)
              (delay output-so-path)
              (delay src-path)
              (delay include-path)
              (delay src-config-path)
              (delay object-dir)
              (delay out-dir)
              (delay '("test-simple-ptr-field"
                       "test-simple-virtual-call"
                       "test-simple-ptr-copy-escape"
                       "test-bound-check"
                       "test-bound-read"
                       "test-malloc-size"
                       "test-ownership-transfer"
                       "test-phi-ice"
                       "test-trivial-assignment"))
              (delay '("test-lto"))
              (delay (if is-macos? "macOS" "Linux"))
              (delay so-ext)
              (delay platform-linker-flags)
              (delay dep-jobs)
              (delay (car (force gmp-promise)))
              (delay (cdr (force gmp-promise)))
              (delay (car (force mpc-promise)))
              (delay (cdr (force mpc-promise)))
              (delay (car (force mpfr-promise)))
              (delay (cdr (force mpfr-promise))))])
    cfg))

;; ============================================================================
;; Main Entry Point
;; ============================================================================

(define (write-makefile* cfg)
  (make-directory* (cfg-ref cfg Config-object-dir))
  (make-directory* (cfg-ref cfg Config-out-dir))
  (define sources (collect-sources cfg))
  (define targets (collect-targets cfg sources))
  (define deps-list (calc-deps-parallel cfg sources targets))
  (call-with-atomic-output-file "Makefile"
    (lambda (o _p)
      (parameterize ([current-output-port o])
        (write-platform-info cfg)
        (write-phony cfg)
        (write-plugin cfg targets)
        (write-compiles cfg sources targets)
        (write-deps cfg targets deps-list)
        (write-clean cfg)
        (write-test-xercese cfg)
        (write-prepare cfg)
        (write-tests cfg)
        (write-lto-tests cfg)))))

(module+ main
  (define cfg (get-default-config))
  (write-makefile* cfg))

;; ============================================================================
;; Lazy Default Config
;; ============================================================================

(define default-config-promise
  (delay (make-config-with-cc (find-default-compiler) 4)))

(define (get-default-config)
  (force default-config-promise))
