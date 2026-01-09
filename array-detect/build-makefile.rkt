#!/usr/bin/env racket
#lang racket

(struct Config
  (
    name
    cc
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
;; Compiler Path Detection
;; ============================================================================

(define (get-cc-plugin-path cc)
  (delay
    (define out
      (string-trim
        (with-output-to-string
          (lambda () (system* (force cc) "-print-file-name=plugin")))))
    (match out
      ["" (raise-user-error 'build-makefile "Failed to locate GCC plugin path")]  
      [_ out])
  )
)

;; ============================================================================
;; Library Dependencies - Platform-Specific
;; ============================================================================

(define (make-lib-args include-path lib-path)
  `("-I" ,(~a (force include-path)) "-L" ,(~a (force lib-path))))

(define (pkg-config-first-flag cmd prefix)
  (with-handlers ([exn? (lambda (_) #f)])
    (define flags
      (string-split
        (string-trim
          (with-output-to-string (lambda () (system cmd))))))
    (define hit (findf (lambda (s) (string-prefix? prefix s)) flags))
    (and hit (substring hit (string-length prefix)))
  ))

(define (brew-prefix name)
  (with-handlers ([exn? (lambda (_) #f)])
    (string-trim
      (with-output-to-string
        (lambda () (system (format "brew --prefix ~a" name)))))
  ))

(define (gmp/brew)
  (delay
    (define gmp-root (brew-prefix "gmp"))
    (values
      (build-path gmp-root "include")
      (build-path gmp-root "lib")))
)

(define (mpc/brew)
  (delay
    (define mpc-root (brew-prefix "mpc"))
    (values
      (build-path mpc-root "include")
      (build-path mpc-root "lib")))
)

(define (mpfr/brew)
  (delay
    (define mpfr-root (brew-prefix "mpfr"))
    (values
      (build-path mpfr-root "include")
      (build-path mpfr-root "lib")))
)

(define (gmp/pkg-config)
  (delay
    (define gmp-i (pkg-config-first-flag "pkg-config --cflags gmp" "-I"))
    (define gmp-l (pkg-config-first-flag "pkg-config --libs gmp" "-L"))
    (values gmp-i gmp-l))
)

(define (mpc/pkg-config)
  (delay
    (define mpc-i (pkg-config-first-flag "pkg-config --cflags mpc" "-I"))
    (define mpc-l (pkg-config-first-flag "pkg-config --libs mpc" "-L"))
    (values mpc-i mpc-l))
)

(define (mpfr/pkg-config)
  (delay
    (define mpfr-i (pkg-config-first-flag "pkg-config --cflags mpfr" "-I"))
    (define mpfr-l (pkg-config-first-flag "pkg-config --libs mpfr" "-L"))
    (values mpfr-i mpfr-l))
)

;; ============================================================================
;; Makefile Generation Helpers
;; ============================================================================

(define (collect-sources config)
  (match-define (Config _ _ _ modules _ src-path _ _ _ _ _ _ _ _ _ _ _ _ _ _ _) config)
  (define (find-rel x) (find-relative-path (current-directory) x))
  (append-map
    (lambda (m)
      (define files (glob (build-path src-path m "*.cc")))
      (map find-rel (sort files path<?)))
    modules))

(define (collect-targets config sources)
  (match-define (Config _ _ _ _ _ src-path _ _ object-dir _ _ _ _ _ _ _ _ _ _ _ _) config)
  (define (find-rel/src x) (find-relative-path src-path x))
  (define (related-obj x) (build-path object-dir (path-replace-extension x ".o")))
  (map (compose related-obj find-rel/src) sources))

(define (write-platform-info cfg)
  (match-define (Config _ cc _ _ _ _ _ _ _ _ _ _ so-ext platform-linker-flags _ _ _ _ _ _ _) cfg)
  (define platform-name (system-type 'os))
  (printf "# Generated Makefile for ~a~n" (force platform-name))
  (printf "# Compiler: ~a~n" (force cc))
  (printf "# Shared library extension: .~a~n" (force so-ext))
  (match (force platform-linker-flags)
    ['() (void)]
    [x
      (printf "# Platform-specific flags:  ")
      (for ([xi x] [i (in-naturals)]) (when (> i 0) (printf " ")) (printf "~a" xi)) 
      (printf "~n")
    ])
  (printf "~n")
)

(define (write-phony config)
  (match-define (Config _ _ _ _ _ _ _ _ _ _ tests lto-tests _ _ _ _ _ _ _ _ _) config)
  (define lto-names (map car (force lto-tests)))
  (printf ".PHONY: ")
  (for ([a (in-sequences (in-list '("all" "clean" "prepare" "test-xercese")) (in-list (force tests)) (in-list lto-names))] [i (in-naturals)])
    (when (> i 0) (printf " ")) 
    (printf "~a" a)
  )
  (printf "~n~n")
)

(define (config-calc-all-args config)
  (match-define (Config _ _ cflags _ _ _ _ _ _ _ _ _ _ platform-linker-flags _ gmp-include gmp-lib mpc-include mpc-lib mpfr-include mpfr-lib) config)
  (define gmp-args (make-lib-args gmp-include gmp-lib))
  (define mpc-args (make-lib-args mpc-include mpc-lib))
  (define mpfr-args (make-lib-args mpfr-include mpfr-lib))
  (in-sequences (in-list (force cflags)) (in-list (force platform-linker-flags))(in-list (force gmp-args)) (in-list (force mpc-args)) (in-list (force mpfr-args)))
)

(define (write-compiles config sources targets)
  (match-define (Config _ cc _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _) config)
  (for ([s sources] [t targets])
    (printf "~a:~n" (~a t))
    (printf "\t")
    (printf "~a -c" cc)
    (for ([a (config-calc-all-args config)])
      (printf " ~s" a))
    (printf " ~s" (~a s))
    (printf " -o ~s" (~a t))
    (printf "~n~n")
  )
)

(define (write-clean config)
  (match-define (Config _ _ _ _ _ _ _ _ object-dir out-dir _ _ _ _ _ _ _ _ _ _ _) config)
  (printf "clean:~n")
  (printf "\trm -rf ~a ~a~n" (force object-dir) (force out-dir))
  (printf "~n")
)

(define (write-plugin config targets)
  (match-define (Config _ cc _ _ output-so _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _) config)
  (printf "all: ~a~n~n" (force output-so))
  (printf "~a:" (force output-so))
  (for ([o targets]) (printf " ~a" (~a o)))
  (printf "~n")
  (printf "\t")
  (printf "~a" (force cc))
  (for ([a (config-calc-all-args config)]) (printf " ~s" a))
  (for ([o targets]) (printf " ~s" (~a o)))
  (printf " -o ~s" (~a (force output-so)))
  (printf "~n~n")
)

(define (write-test-xercese config)
  (match-define (Config _ cc _ _ output-so _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _) config)
  (define plugin-arg (format "-fplugin=~a" output-so))
  (printf "test-xercese: ~a~n" output-so)
  (define input `((cxx . ,(~a (force cc))) (cflags ,plugin-arg)))
  (printf "\t@echo ~s | racket test-script/test-xercese.rkt~n" (~s input))
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
  (define cwd-str (~a cwd))
  (define (abs-path p)
    (simplify-path
      (if (relative-path? p)
          (build-path cwd p)
          p)))
  (for/list ([p deps]
             #:when (let ([abs (abs-path p)])
                      (string-prefix? (~a abs) cwd-str)))
    (find-relative-path cwd (abs-path p))))

(define (calc-deps-parallel config sources targets)
  (match-define (Config _ cc _ _ _ _ _ _ _ _ _ _ _ _ dep-jobs _ _ _ _ _ _) config)
  (define flags (sequence->list (config-calc-all-args config)))
  (define total (length (force sources)))
  (cond
    [(zero? total) '()]
    [else
     (define job-count (force dep-jobs))
     (define workers (max 1 (min total job-count)))
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

(define (write-deps config targets deps-list)
  (for ([t targets] [deps deps-list])
    (define deps-in-proj (deps-in-project deps))
    (printf "~a:" t)
    (for ([d deps-in-proj])
      (printf " \\\n  ~a" d))
    (printf "~n~n")))

(define (write-prepare config)
  (match-define (Config _ _ _ modules _ _ _ _ object-dir out-dir _ _ _ _ _ _ _ _ _ _ _) config) 
  (printf "prepare:~n")
  (for ([m (force modules)])
    (printf "\tmkdir -p ~a~n" (build-path (force object-dir) m)))
  (printf "\tmkdir -p ~a~n" (build-path (force out-dir)))
  (printf "~n"))

(define (write-test-impl cc output-so name)
  (define plugin-arg (format "-fplugin=~a" output-so))
  (printf "~a: ~a~n" name output-so)
  (define input `((cxx . ,(~a (force cc))) (cflags . ,(list plugin-arg))))
  (printf "\t@echo ~s | racket test-script/~a.rkt~n" (~s input) name)
  (printf "~n"))

(define (write-lto-test-impl cc output-so name script-file)
  (define plugin-arg (format "-fplugin=~a" output-so))
  (printf "~a: ~a~n" name output-so)
  (define input `((cxx . ,(~a (force cc))) (cflags . ,(list plugin-arg "-flto"))))
  (printf "\t@echo ~s | racket ~a~n" (~s input) (build-path "test-script" script-file))
  (printf "~n"))

(define (write-tests config)
  (match-define (Config _ cc _ _ output-so _ _ _ _ _ tests _ _ _ _ _ _ _ _ _ _) config)
  (for ([a (force tests)]) (write-test-impl (force cc) (force output-so) a))
)

(define (write-lto-tests config)
  (match-define (Config _ cc _ _ output-so _ _ _ _ _ _ lto-tests _ _ _ _ _ _ _ _ _) config)
  (for ([a (force lto-tests)])
    (write-lto-test-impl (force cc) (force output-so) (car a) (cdr a))))

;; ============================================================================
;; Global Config
;; ============================================================================

(define (make-config-with-cc name cc-path dep-jobs)
  (define os-type (system-type 'os))
  (define is-macos? (eq? os-type 'macosx))
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
      "array-detect-lto-transform"
      ;; 新模块 (基础数据结构)
      "ad-field-write"
      "ad-write-source"
      "ad-source-use"
      "ad-ownership-move"
      ;; 新模块 (拆分自 escape-evidence-synthesizer)
      "ad-escaped-use"
      "ad-source-escape"
      "ad-field-escape"
      ;; 新模块 (wrapper 独立)
      "ad-field-wrapper"
      ;; 新模块 (控制流)
      "ad-pipeline"
      "ad-driver"))
  (define src-path "src")
  (define include-path "include")
  (define src-config-path "config")
  (define platform-linker-flags
    (if is-macos?
        '("-undefined" "dynamic_lookup")
        '()))
  (define macos-warning-flags
    (if is-macos?
        '("-Wno-deprecated-declarations"
          "-Wa,-Wno-overriding-deployment-version")
        '()))
  (define module-include-paths
    (for/list ([m modules])
      (~a (build-path include-path m))))
  (define extra-include-paths
    (list (~a (build-path include-path "array-detect-result"))))
  (define base-args-promise
    (delay
      (define plugin-path (force (get-cc-plugin-path cc-path)))
      `(
        "-fPIC"
        "-fno-rtti"
        "-shared"
        "-I" ,(~a (force src-config-path))
        "-I" ,(~a (force plugin-path))
        "-I" ,(~a (build-path (force plugin-path) "include"))
        ,@(append-map (lambda (p) `("-I" ,p))
                    (append module-include-paths extra-include-paths))
        ,@platform-linker-flags
        "-Wall"
        "-Wextra"
        ,@macos-warning-flags
        "-std=c++17"
        "-g"
        "-O2")))
  (define gmp (gmp/brew))
  (define mpc (mpc/brew))
  (define mpfr (mpfr/brew))
  (define tests
    '("test-simple-ptr-field"
      "test-simple-virtual-call"
      "test-simple-ptr-copy-escape"
      "test-bound-check"
      "test-bound-read"
      "test-malloc-size"
      "test-ownership-transfer"
      "test-phi-ice"
      "test-trivial-assignment"))
(define lto-tests
  `(
    ,@(map (lambda (t) (cons (format "~a-lto" t) (format "~a.rkt" t))) tests)
    ("test-lto" . "test-lto.rkt")
    ("test-xercese-lto" . "test-xercese.rkt")
  ))
  (Config
    name
    cc-path
    base-args-promise
    modules
    output-so-path
    src-path
    include-path
    src-config-path
    object-dir
    out-dir
    tests
    lto-tests
    so-ext
    platform-linker-flags
    dep-jobs
    (delay (match-define-values (i _) (force gmp)) i)
    (delay (match-define-values (_ l) (force gmp)) l)
    (delay (match-define-values (i _) (force mpc)) i)
    (delay (match-define-values (_ l) (force mpc)) l)
    (delay (match-define-values (i _) (force mpfr)) i)
    (delay (match-define-values (_ l) (force mpfr)) l)
  )
)

;; ============================================================================
;; Main Entry Point
;; ============================================================================

(define (write-makefile* cfg)
  (make-directory* (force (Config-object-dir cfg)))
  (make-directory* (force (Config-out-dir cfg)))
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
  (delay (make-config-with-cc "plugin-array-detect" (find-executable-path "g++-15") 4)))

(define (get-default-config)
  (force default-config-promise))
