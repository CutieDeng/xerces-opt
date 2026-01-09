#lang racket

(require "../../array-detect/test-script/test-lib.rkt")
(require "../../array-detect/test-script/test-lib2.rkt")

(define ut-compile-task-files
  '(("vcall.cc" "test-out/vcall.o")))

(define (pre root)
  (system* (find-executable-path "mkdir") "-p" (build-path root "test-out")))

(provide config)

(define config
  (test-config
    "test-simple-virtual-call"
    pre
    (lambda (_) (void))
    (get-ut-compile-task-files/list ut-compile-task-files '())
    #f
    (lazy #f)))
