#lang racket

(require file/glob)

(define cxx (or (find-executable-path "g++-15") (raise-user-error "Cannot find g++ compiler")))

(define object-dir "obj")
(module+ main (make-directory* object-dir))

(define out-dir "out")
(module+ main (make-directory* out-dir))

(define output-so-path (build-path out-dir "plugin-array-detect.dylib"))

;; New modular structure
(define modules '("array-detect-core" "array-detect-context" "array-detect-gcc" "array-detect-utils"))
(define s "src")
(define i "include")
(define c "config")

;; Build paths for each module
(define (module-src-path m) (build-path s m))
(define (module-include-path m) (build-path i m))

;; Include dependencies for headers
(define include-related/includes (let ()
  (hash
    ;; Core module headers
    (build-path (module-include-path "array-detect-core") "array-detector.hh")
      (list (build-path (module-include-path "array-detect-utils") "prelude.hh")
            (build-path (module-include-path "array-detect-context") "context.hh")
            (build-path (module-include-path "array-detect-context") "state.hh")
            (build-path (module-include-path "array-detect-core") "array-detector-op0.hh")
            (build-path (module-include-path "array-detect-utils") "info.hh"))

    (build-path (module-include-path "array-detect-core") "array-detector-op0.hh")
      (list (build-path (module-include-path "array-detect-utils") "prelude.hh")
            (build-path (module-include-path "array-detect-context") "context.hh")
            (build-path (module-include-path "array-detect-context") "state.hh"))

    ;; Context module headers
    (build-path (module-include-path "array-detect-context") "context-init.hh")
      (list (build-path (module-include-path "array-detect-utils") "prelude.hh")
            (build-path (module-include-path "array-detect-context") "context.hh")
            (build-path (module-include-path "array-detect-context") "state.hh"))

    (build-path (module-include-path "array-detect-context") "state.hh")
      (list (build-path c "cutie-state.txt"))

    ;; Utils module headers
    (build-path (module-include-path "array-detect-utils") "info.hh")
      (list (build-path (module-include-path "array-detect-gcc") "gcc-common.hh"))

    (build-path (module-include-path "array-detect-gcc") "gcc-ext-util.hh")
      (list (build-path (module-include-path "array-detect-gcc") "gcc-common.hh")
            (build-path (module-include-path "array-detect-utils") "prelude.hh"))
  )))

;; Include dependencies for source files
(define include-related/srcs (let ()
  (hash
    ;; Core module sources
    (build-path (module-src-path "array-detect-core") "array-detector.cc")
      (list (build-path (module-include-path "array-detect-utils") "prelude.hh")
            (build-path (module-include-path "array-detect-core") "array-detector.hh")
            (build-path (module-include-path "array-detect-context") "state.hh"))

    (build-path (module-src-path "array-detect-core") "array-detector-op0.cc")
      (list (build-path (module-include-path "array-detect-utils") "prelude.hh")
            (build-path (module-include-path "array-detect-context") "state.hh")
            (build-path (module-include-path "array-detect-core") "array-detector-op0.hh")
            (build-path (module-include-path "array-detect-context") "context-init.hh")
            (build-path (module-include-path "array-detect-gcc") "gcc-ext-util.hh"))

    ;; Context module sources
    (build-path (module-src-path "array-detect-context") "context.cc")
      (list (build-path (module-include-path "array-detect-context") "context.hh"))

    (build-path (module-src-path "array-detect-context") "context-init.cc")
      (list (build-path (module-include-path "array-detect-utils") "prelude.hh")
            (build-path (module-include-path "array-detect-context") "state.hh")
            (build-path (module-include-path "array-detect-context") "context.hh")
            (build-path (module-include-path "array-detect-context") "context-init.hh"))

    (build-path (module-src-path "array-detect-context") "state.cc")
      (list (build-path (module-include-path "array-detect-context") "state.hh"))

    ;; GCC module sources
    (build-path (module-src-path "array-detect-gcc") "context-init-gcc.cc")
      (list (build-path (module-include-path "array-detect-gcc") "gcc-common.hh")
            (build-path (module-include-path "array-detect-context") "context.hh")
            (build-path (module-include-path "array-detect-context") "context-init.hh"))

    (build-path (module-src-path "array-detect-gcc") "gcc-ext-util.cc")
      (list (build-path (module-include-path "array-detect-gcc") "gcc-ext-util.hh")
            (build-path (module-include-path "array-detect-utils") "info.hh"))

    (build-path (module-src-path "array-detect-gcc") "plugin-top.cc")
      (list (build-path (module-include-path "array-detect-gcc") "gcc-common.hh")
            (build-path (module-include-path "array-detect-utils") "prelude.hh")
            (build-path (module-include-path "array-detect-context") "state.hh")
            (build-path (module-include-path "array-detect-context") "context.hh")
            (build-path (module-include-path "array-detect-context") "context-init.hh")
            (build-path (module-include-path "array-detect-core") "array-detector.hh")
            (build-path (module-include-path "array-detect-core") "array-detector-op0.hh"))

    ;; Utils module sources
    (build-path (module-src-path "array-detect-utils") "info-print.cc")
      (list (build-path (module-include-path "array-detect-utils") "info-print.hh"))
  )))

;; Collect all source files from all modules
(define (find-rel x) (find-relative-path (current-directory) x))
(define sources
  (append-map
    (lambda (module)
      (map find-rel (glob (build-path (module-src-path module) "*.cc"))))
    modules))

(define (find-rel/src x) (find-relative-path "src" x))
(define (related-obj x) (build-path object-dir (path-replace-extension x ".o")))
(define targets (map (compose related-obj find-rel/src) sources))

;; Compiler arguments
(define (plugin-path-getter)
  (string-trim (with-output-to-string (lambda () (system* cxx "-print-file-name=plugin")))))
(define plugin-path (plugin-path-getter))

(define args `(
  "-fPIC"
  "-fno-rtti"
  "-shared"
  "-I"
  ,plugin-path
  "-I"
  ,(path->string (build-path plugin-path "include"))
  "-I"
  ,i
  "-I"
  ,c
  ;; Add include paths for each module using include as base
  "-I"
  ,(path->string (build-path "include" "array-detect-core"))
  "-I"
  ,(path->string (build-path "include" "array-detect-context"))
  "-I"
  ,(path->string (build-path "include" "array-detect-gcc"))
  "-I"
  ,(path->string (build-path "include" "array-detect-utils"))
  "-undefined" "dynamic_lookup"
  "-O2"
  "-Wall"
  "-Wextra"
  "-std=c++17"
))

;; Library dependencies
(define (gmp/args)
  (with-handlers ([exn? (lambda (_) '())])
    (append
      (string-split (string-trim (with-output-to-string
        (lambda () (system "pkg-config --libs gmp")))))
      (string-split (string-trim (with-output-to-string
        (lambda () (system "pkg-config --cflags gmp")))))
    )))

(define (mpc/args)
  (with-handlers ([exn? (lambda (_) '())])
    (define mpc-directory
      (string-trim (with-output-to-string (lambda () (system "brew --prefix libmpc")))))
    `("-I"
      ,(path->string (build-path mpc-directory "include"))
      "-L"
      ,(path->string (build-path mpc-directory "lib")))
  ))

(define (mpfr/args)
  (with-handlers ([exn? (lambda (_) '())])
    (define mpfr-directory
      (string-trim (with-output-to-string (lambda () (system "brew --prefix mpfr")))))
    `("-I"
      ,(path->string (build-path mpfr-directory "include"))
      "-L"
      ,(path->string (build-path mpfr-directory "lib")))
  ))

(define args^ (append args (gmp/args) (mpc/args) (mpfr/args)))

;; Build rules generation
(define (write-compiles)
  (for ([s sources] [t targets])
    (printf "~a:~n" t)
    (printf "\t")
    (printf "~a -c" cxx)
    (for ([a args^]) (printf " ~s" a))
    (printf " ~s" (path->string s))
    (printf " -o ~s" (path->string t))
    (printf "~n~n")
  )
)

(define (write-deps)
  (define includes (make-hash))
  (for ([(k v) (in-hash include-related/includes)])
    (dict-set! includes k (append (dict-ref includes k '()) v)))
  (for ([(k v) (in-hash include-related/srcs)])
    (dict-set! includes k (append (dict-ref includes k '()) v)))
  (for ([t targets] [s sources])
    (define depend
      (let calc-dep-sub ([current-deps (set)] [visited-set (set)] [pending-set (set s)])
        (cond
          [(set-empty? pending-set) current-deps]
          [else
            (call-with-values (thunk (for/fold ([current-deps current-deps] [visited-set visited-set] [pending-set^ (set)]) ([p (in-set pending-set)])
              (define ds (list->set (dict-ref includes p '())))
              (define current-deps^ (set-union current-deps ds))
              (define visited-set^ (set-union visited-set ds))
              (define pending-set^^ (set-union pending-set^ (set-subtract ds visited-set)))
              (values current-deps^ visited-set^ pending-set^^)
            )) calc-dep-sub)
          ])
      )
    )
    (printf "~a: " t)
    (for ([d (in-set depend)]) (printf "~a " d))
    (printf "~a~n" s)
  )
  (printf "~a:" output-so-path)
  (for ([t targets]) (printf " ~a" t))
  (printf "~n~n")
)

(define (write-clean)
  (printf "clean:~n")
  (printf "\trm -rv ~a~n" (path->string (build-path object-dir "*")))
  (printf "\trm -rv ~a~n" (path->string (build-path out-dir "*")))
  (printf "~n")
)

(define (write-plugin)
  (printf "~a:~n" output-so-path)
  (printf "\t~a" cxx)
  (for ([a args^]) (printf " ~s" a))
  (for ([o targets]) (printf " ~s" (path->string o)))
  (printf " -o ~s" (path->string output-so-path))
  (printf "~n~n")
)

(define (write-test)
  (printf "test: ~a~n" output-so-path)
  (printf "\tcd ../test/test-xercese; ")
  (printf "racket build-xercese.rkt~n")
  (printf "~n")
)

(define (run)
  (call-with-atomic-output-file "Makefile" (lambda (o _p) (parameterize ([current-output-port o])
    (write-plugin)
    (write-compiles)
    (write-deps)
    (write-clean)
    (write-test)
  ))))

(module+ main (run))
