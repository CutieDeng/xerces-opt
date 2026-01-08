#!/usr/bin/env racket
#lang racket

(define r (read))

(define cxx (or (dict-ref r 'cxx #f) (find-executable-path "g++-15")))
(define cflags (dict-ref r 'cflags '()))

(make-directory* "obj")
(make-directory* "out")
(delete-directory/files (build-path "out" "result.rktd") #:must-exist? #f)
(delete-directory/files (build-path "out" "aggregated.rktd") #:must-exist? #f)

;; Environment variables for plugin output
;; AD_RESULT_FILE: per-TU results (append mode, used by analysis phase)
;; AD_AGGREGATED_FILE: aggregated LTRANS results (overwrite mode)
(putenv "AD_RESULT_FILE" "out/result.rktd")
(putenv "AD_AGGREGATED_FILE" "out/aggregated.rktd")

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

;; Display aggregated results
(when (file-exists? "out/aggregated.rktd")
  (printf "=== LTO Aggregated Results ===~n")
  (call-with-input-file "out/aggregated.rktd"
    (lambda (in)
      (for ([line (in-lines in)])
        (printf "~a~n" line)))))

;; Also show per-TU results if available
(when (file-exists? "out/result.rktd")
  (printf "~n=== Per-TU Results (from analysis phase) ===~n")
  (call-with-input-file "out/result.rktd"
    (lambda (in)
      (for ([line (in-lines in)])
        (printf "~a~n" line)))))
