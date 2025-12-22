#lang racket

(define r (read))

(define cxx (or (dict-ref r 'cxx #f) (find-executable-path "g++-15")))
(define cflags (dict-ref r 'cflags '()))

(define (compile)
  (make-directory* "obj")
  (apply system* (cons cxx (append '("ptr-field.cc" "-c" "-o" "obj/ptr-field.o") cflags)))
)

(compile)
