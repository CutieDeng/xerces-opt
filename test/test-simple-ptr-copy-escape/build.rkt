#lang racket

(require "../../array-detect/test-script/test-lib.rkt")
(require "../../array-detect/test-script/test-lib2.rkt")

(define ut-compile-task-files
  '(("ptr-copy.cc" "test-out/ptr-copy.o")))

(define (pre root)
  (system* (find-executable-path "mkdir") "-p" (build-path root "test-out")))

(provide config)

(define config
  (test-config
    "test-simple-ptr-copy-escape"
    pre
    (lambda (_) (void))
    (get-ut-compile-task-files/list ut-compile-task-files '())
    #f
    (lazy #f)))
