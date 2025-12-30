#lang racket

(define r (read))

(define cxx (or (dict-ref r 'cxx #f) (find-executable-path "g++-15")))
(define cflags (dict-ref r 'cflags '()))

(define (compile name)
  (make-directory* "obj")
  (apply system* (cons cxx (append `(,(format "~a.cpp" name) "-c" "-o" ,(format "obj/~a.o" name)) cflags))))

(compile "basic_test")
(compile "complex_test")
