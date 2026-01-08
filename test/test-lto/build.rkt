#!/usr/bin/env racket
#lang racket

(require "../../array-detect/test-script/test-lib.rkt")
(require "../../array-detect/test-script/test-lib2.rkt")

(define ut-compile-task-files
  '(
    ("a.cpp" "out/a.o")
    ("b.cpp" "out/b.o")
  )
)

(define total-compile-task-files
  '(("out/a.o" "out/b.o")
    "out/a"
  )
)

(define (pre root)
  (system* (find-executable-path "mkdir") "-p" (build-path root "out"))
)

(provide config)

(define config
  (test-config
    "test-lto"
    pre
    (lambda (_) (void))
    (get-ut-compile-task-files/list ut-compile-task-files '("-flto"))
    (get-total-compile-task-files total-compile-task-files '("-flto"))
    (lazy #f)
  )
)
