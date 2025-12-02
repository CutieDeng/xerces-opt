#!/usr/bin/env racket
#lang racket

(require file/glob)

(define cxx
  (or (find-executable-path "g++-15")
      (raise-user-error "Cannot find g++ compiler"))
)

(define out-dir "out")

(define output-so-path (build-path out-dir "plugin-array-detect.dylib"))

(define (plugin-path-getter)
  (string-trim (with-output-to-string (lambda () (system* cxx "-print-file-name=plugin")))))
(define plugin-path (plugin-path-getter))

(define args `(
  "-fPIC"
  "-fno-rtti"
  "-shared"
  "-o"
  ,output-so-path
  "array-detect.cc"
  "-I"
  ,plugin-path
  "-I"
  ,(path->string (build-path plugin-path "include"))
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
      ,(build-path mpc-directory "include")
      "-L"
      ,(build-path mpc-directory "lib"))
  ))

(define (mpfr/args)
  (with-handlers ([exn? (lambda (_) '())])
    (define mpfr-directory
      (string-trim (with-output-to-string (lambda () (system "brew --prefix mpfr")))))
    `("-I"
      ,(build-path mpfr-directory "include")
      "-L"
      ,(build-path mpfr-directory "lib"))
  ))

(set! args (append args (gmp/args) (mpc/args) (mpfr/args)))


(define (clean)
  (for ([f (glob (build-path out-dir "*"))])
    (delete-file f)
    (eprintf "+ rm ~a~n" f)
  )
)

(define (run-xercese)
  (parameterize ([current-directory (build-path 'up "test" "test-xercese")])
    (system* (find-executable-path "racket") "build-xercese.rkt")
  )
)

(define (run)
  (and
    (clean)
    (make-directory* out-dir)
    (apply system* (cons cxx args))
    (eprintf "+ generate plugin lib ~a~n" output-so-path)
    (run-xercese)
  )
)

(module+ main (run))
