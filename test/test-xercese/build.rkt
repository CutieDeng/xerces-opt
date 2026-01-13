#lang racket

(require "../../array-detect/test-script/test-lib.rkt")
(require "../../array-detect/test-script/test-lib2.rkt")

(define test-out "out")

(define ut-compile-task-files
  `(("MemoryManagerImpl.cc" ,(build-path test-out "MemoryManagerImpl.o"))
    ("PlatFormUtils.cc" ,(build-path test-out "PlatFormUtils.o"))
    ("main_test.cpp" ,(build-path test-out "main_test.o"))
    ("XMemory.cc" ,(build-path test-out "XMemory.o")))

(define total-compile-task-files
  `(
    ,(map cadr (force ut-compile-task-files))
    ,(build-path test-out "test-xercese"))

(define base-args '("-std=c++17" "-Wall" "-Wextra" "-g"))

(define (pre root)
  (system* (find-executable-path "mkdir") "-p" (build-path root test-out)))

(define (run _cxx _flags root)
  (system (path->string (build-path root test-out "test-xercese"))))

(provide config)

(define config
  (test-config
    "test-xercese"
    pre
    (lambda (_) (void))
    (get-ut-compile-task-files/list ut-compile-task-files base-args)
    (get-total-compile-task-files total-compile-task-files base-args)
    (lazy run)))
