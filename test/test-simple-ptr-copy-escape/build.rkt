#lang racket

(define r (read))

(define cxx (or (dict-ref r 'cxx #f) (find-executable-path "g++-15")))
(define cflags (dict-ref r 'cflags '()))

(define (compile)
  (make-directory* "obj")
  (apply system* (cons cxx (append '("ptr-copy.cc" "-c" "-o" "obj/ptr-copy.o") cflags)))
)

(compile)
