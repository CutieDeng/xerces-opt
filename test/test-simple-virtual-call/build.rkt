#lang racket

(define r (read))

(define cxx (or (dict-ref r 'cxx #f) (find-executable-path "g++-15")))
(define cflags (dict-ref r 'cflags '()))

(define (compile)
  (apply system* (cons cxx (append '("vcall.cc" "-c" "-o" "obj/vcall.o") cflags)))
)

(compile)
