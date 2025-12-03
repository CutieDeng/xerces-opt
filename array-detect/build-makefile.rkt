#lang racket

(require file/glob)

(define cxx (or (find-executable-path "g++-15") (raise-user-error "Cannot find g++ compiler")))

(define object-dir "obj")
(module+ main (make-directory* object-dir))

(define out-dir "out")
(module+ main (make-directory* out-dir))

(define output-so-path (build-path out-dir "plugin-array-detect.dylib"))

(define i "include")
(define s "src")
(define c "config")
(define include-related/includes (let ()
  (hash
    (build-path i "array-detector.hh") (list (build-path i "prelude.hh") (build-path i "context.hh") (build-path i "state.hh") (build-path i "array-detector-op0.hh") (build-path i "info.hh"))
    (build-path i "array-detector-op0.hh") (list (build-path i "prelude.hh") (build-path i "context.hh") (build-path i "state.hh"))
    (build-path i "context-init.hh") (list (build-path i "prelude.hh") (build-path i "context.hh"))
    (build-path i "info-print.hh") (list (build-path i "prelude.hh") (build-path i "array-detector.hh") (build-path i "state.hh"))
    (build-path i "state.hh") (list (build-path c "cutie-state.txt"))
    (build-path i "info.hh") (list (build-path i "gcc-common.hh"))
  )))

(define include-related/srcs (let ()
  (hash
    (build-path s "array-detector.cc") (list (build-path i "prelude.hh") (build-path i "array-detector.hh") (build-path i "state.hh"))
    (build-path s "array-detector-op0.cc") (list (build-path i "prelude.hh") (build-path i "state.hh") (build-path i "array-detector-op0.hh") (build-path i "context-init.hh"))
    (build-path s "context.cc") (list (build-path i "context.hh"))
    (build-path s "context-init.cc") (list (build-path i "prelude.hh") (build-path i "state.hh") (build-path i "context.hh"))
    (build-path s "info-print.cc") (list (build-path i "info-print.hh"))
    (build-path s "plugin-top.cc") (list (build-path i "prelude.hh") (build-path i "state.hh") (build-path i "context.hh") (build-path i "context-init.hh") (build-path i "array-detector.hh") (build-path i "array-detector-op0.hh") (build-path i "gcc-common.hh"))
  )))

(define (find-rel x) (find-relative-path (current-directory) x))
(define sources (map find-rel (glob (build-path s "*"))))

(define (find-rel/src x) (find-relative-path "src" x))
(define (related-obj x) (build-path object-dir (path-replace-extension x ".o")))
(define targets (map (compose related-obj find-rel/src) sources))

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
  "-undefined" "dynamic_lookup"
  "-O2"
  "-Wall"
  "-Wextra"
  "-std=c++17"
))


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
              (values current-deps^ visited-set^ pending-set^)
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

(define (run)
  (call-with-atomic-output-file "Makefile" (lambda (o _p) (parameterize ([current-output-port o])
    (write-plugin)
    (write-compiles)
    (write-deps)
    (write-clean) 
  )))
)

(module+ main (run))
