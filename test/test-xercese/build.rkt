#lang racket

(require "../../array-detect/test-script/test-lib.rkt")
(require "../../array-detect/test-script/test-lib2.rkt")

(define ut-compile-task-files
  '(("MemoryManagerImpl.cc" "test-out/MemoryManagerImpl.o")
    ("PlatFormUtils.cc" "test-out/PlatFormUtils.o")
    ("main_test.cpp" "test-out/main_test.o")
    ("XMemory.cc" "test-out/XMemory.o")))

(define total-compile-task-files
  '(("test-out/MemoryManagerImpl.o"
     "test-out/PlatFormUtils.o"
     "test-out/main_test.o"
     "test-out/XMemory.o")
    "test-out/test-xercese"))

(define base-args '("-std=c++17" "-Wall" "-Wextra" "-g"))

(define (pre root)
  (system* (find-executable-path "mkdir") "-p" (build-path root "test-out")))

(define (run _cxx _flags root)
  (system (path->string (build-path root "test-out" "test-xercese"))))

(provide config)

(define config
  (test-config
    "test-xercese"
    pre
    (lambda (_) (void))
    (get-ut-compile-task-files/list ut-compile-task-files base-args)
    (get-total-compile-task-files total-compile-task-files base-args)
    (lazy run)))
