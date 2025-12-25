#!/usr/bin/env racket
#lang racket

(require file/glob)

(define cxx (or (find-executable-path "g++-15") (raise-user-error "Cannot find g++ compiler")))
(define object-dir "obj")
(module+ main (make-directory* object-dir))

(define out-dir "out")
(module+ main (make-directory* out-dir))

(define output-so-path (build-path out-dir "plugin-array-detect.dylib"))

;; New modular structure
(define modules '("array-detect-core" "array-detect-collection" "array-detect-context" "array-detect-gcc" "array-detect-utils" "array-detect-field-analysis" "array-detect-pipeline" "array-detect-write-trace" "array-detect-source-escape-collection" "array-detect-escape-synthesizer" "array-detect-ownership-transfer"))
(define s "src")
(define i "include")
(define c "config")

;; Collect all source files from all modules
(define (find-rel x) (find-relative-path (current-directory) x))
(define sources
  (append-map
    (lambda (m)
      (map find-rel (glob (build-path s m "*.cc"))))
    modules))

(define (find-rel/src x) (find-relative-path "src" x))
(define (related-obj x) (build-path object-dir (path-replace-extension x ".o")))
(define targets (map (compose related-obj find-rel/src) sources))

;; Compiler arguments
(define (plugin-path-getter)
  (string-trim (with-output-to-string (lambda () (system* cxx "-print-file-name=plugin")))))
(define plugin-path (plugin-path-getter))

;; 基础编译选项
(define args `(
  "-fPIC"
  "-fno-rtti"
  "-shared"
  "-I"
  ,c
  "-I"
  ,plugin-path
  "-I"
  ,(path->string (build-path plugin-path "include"))
  ;; Add include paths for each module using include as base
  "-I"
  ,(path->string (build-path "include/array-detect-core"))
  "-I"
  ,(path->string (build-path "include/array-detect-collection"))
  "-I"
  ,(path->string (build-path "include/array-detect-context"))
  "-I"
  ,(path->string (build-path "include/array-detect-gcc"))
  "-I"
  ,(path->string (build-path "include/array-detect-utils"))
  "-I"
  ,(path->string (build-path "include/array-detect-result"))
  "-I"
  ,(path->string (build-path "include/array-detect-field-analysis"))
  "-I"
  ,(path->string (build-path "include/array-detect-pipeline"))
  "-I"
  ,(path->string (build-path "include/array-detect-write-trace"))
  "-I"
  ,(path->string (build-path "include/array-detect-source-escape-collection"))
  "-I"
  ,(path->string (build-path "include/array-detect-escape-synthesizer"))
  "-I"
  ,(path->string (build-path "include/array-detect-ownership-transfer"))
  "-undefined" "dynamic_lookup"
  "-Wall"
  "-Wextra"
  "-std=c++17"
  "-g"
  "-O2"
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

(define (write-clean)
  (printf "clean:~n")
  (printf "\trm -rv ~a~n" (build-path object-dir "*"))
  (printf "\trm -rv ~a~n" (build-path out-dir "*"))
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

(define (write-test-xercese)
  (printf "test: ~a~n" output-so-path)
  (printf "\tcd ../test/test-xercese; ")
  (printf "racket build-xercese.rkt < ~s~n" (path->string (path->complete-path (build-path "test-script/test-simple-virtual-call.rktd"))))
  (printf "~n")
)

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
              (define f^ (cond [(equal? (+ j 1) (length is)) f] [else (string-trim f " \\" #:left? #f)]))
              (define f^^ (cond [(equal? j 0)
                  (define loc (string-find f^ ": "))
                  (cond [loc (substring f^ (+ loc 2))] [else ""])
                ]
                [else (string-trim f^ #:right? #f #:repeat? #t)]))
              (string-split f^^)
            )]
          ['done-error
            (for ([el (in-lines i2)]) (eprintf "\t~a~n" el)) (eprintf "~n")
            (raise-user-error 'calc-dependency "failed to calc '~a' by ~a'" filename cxx)]))
    )
  )
)

(define (exists-up? path)
  (match-define-values (base name must-be-dir?) (split-path path))
  (match* (base name)
    [(_ 'up) #t]
    [((or #f 'relative) _) #f]
    [(_ _) (exists-up? base)]
  )
)

(define (write-deps2)
  (for ([s sources] [t targets])
    (define ds (calc-dependency s))
    (define ds^ (map (compose find-rel simple-form-path) ds))
    (define ds^^ (filter (compose not exists-up?) ds^))
    (printf "~a: \\~n" t)
    (for ([d ds^^])
      (printf "  ~a \\~n" d))
    (printf "~n")
  )
  (printf "~a:" output-so-path)
  (for ([t targets]) (printf " ~a" t))
  (printf "~n~n")
)

(define (write-prepare)
  (printf "prepare:~n")
  (for ([m modules])
    (printf "\tmkdir -p ~a~n" (build-path object-dir m))
  )
  (printf "\tmkdir -p ~a~n" (build-path object-dir "array-detect-write-trace"))
  (printf "\tmkdir -p ~a~n" (build-path object-dir "array-detect-source-escape-collection"))
  (printf "\tmkdir -p ~a~n" (build-path out-dir))
  (printf "~n")
)

(define a-tests '("test-simple-ptr-field" "test-simple-virtual-call" "test-simple-ptr-copy-escape"))

(define (write-test name)
  (printf "~a: ~a~n" name output-so-path)
  (printf "\tcd ../test/~a; " name)
  (printf "racket build.rkt < ~s~n" (path->string (path->complete-path (build-path "test-script/test-simple-virtual-call.rktd"))))
  (printf "~n")
)

(define (write-tests)
  (for ([a a-tests]) (write-test a))
)

(define (write-makefile)
  (call-with-atomic-output-file "Makefile" (lambda (o _p) (parameterize ([current-output-port o])
    (write-plugin)
    (write-compiles)
    (write-deps2)
    (write-clean)
    (write-test-xercese)
    (write-prepare)
    (write-tests)
  ))))

(module+ main (write-makefile))
