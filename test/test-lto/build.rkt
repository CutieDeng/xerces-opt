#!/usr/bin/env racket
#lang racket

(define r (read))

(define cxx (or (dict-ref r 'cxx #f) (find-executable-path "g++-15")))
(define cflags (dict-ref r 'cflags '()))

(make-directory* "obj")
(make-directory* "out")
(delete-directory/files (build-path "out" "result.rktd") #:must-exist? #f)

;; Environment variables for plugin output
(putenv "AD_RESULT_FILE" "out/result.rktd")
; (putenv "AD_DEBUG_FILE" "out/debug.txt")

;; LTO compilation: compile each TU with -flto
(define (compile-lto name)
  (putenv "AD_DEBUG_FILE" (~a (build-path "out" (path-add-extension name ".debug"))))
  (apply system* (cons cxx (append `(,(format "~a.cpp" name) "-c" "-flto" "-o" ,(format "obj/~a.o" name)) cflags))))

;; LTO link: link all object files with -flto to trigger LTRANS
;; The plugin is also needed during link to handle LTRANS phase
(define (link-lto)
  (putenv "AD_DEBUG_FILE" (~a (build-path "out" "main.debug")))
  (apply system* (cons cxx (append cflags `("-flto" "obj/a.o" "obj/b.o" "-o" "obj/test-lto")))))

;; Compile both TUs
(compile-lto "a")
(compile-lto "b")

;; Link with LTO (this triggers LTRANS phase)
(link-lto)

;; Display results
(when (file-exists? "out/result.rktd")
  (printf "=== LTO Analysis Result ===~n")
  (call-with-input-file "out/result.rktd"
    (lambda (in)
      (for ([line (in-lines in)])
        (printf "~a~n" line)))))
