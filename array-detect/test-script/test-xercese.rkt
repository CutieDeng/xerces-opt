#!/usr/bin/env racket
#lang racket

(require "test-lib.rkt")
(require "../../test/test-xercese/build.rkt")

(module+ main
  (define rconfig (read))
  (define cxx (dict-ref rconfig 'cxx))
  (define cflags (dict-ref rconfig 'cflags '()))
  (define root-path "../test/test-xercese")
  ;; 添加 test-xercese 特定的 include 路径
  (define cflags-with-include (append cflags (list (string-append "-I" root-path))))
  (run-test config cxx cflags-with-include root-path))
