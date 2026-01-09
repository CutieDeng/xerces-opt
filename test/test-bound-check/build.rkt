#lang racket

(require "../../array-detect/test-script/test-lib.rkt")
(require "../../array-detect/test-script/test-lib2.rkt")

(define ut-compile-task-files
  '(("basic_test.cpp" "test-out/basic_test.o")
    ("complex_test.cpp" "test-out/complex_test.o")))

(define (pre root)
  (system* (find-executable-path "mkdir") "-p" (build-path root "test-out")))

(provide config)

(define config
  (test-config
    "test-bound-check"
    pre
    (lambda (_) (void))
    (get-ut-compile-task-files/list ut-compile-task-files '())
    #f
    (lazy #f)))
