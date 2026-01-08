#!/usr/bin/env racket
#lang racket

(require "test-lib.rkt")
(require "../../test/test-simple-ptr-copy-escape/build.rkt")

(module+ main
  (define rconfig (read))
  (define cxx (dict-ref rconfig 'cxx))
  (define cflags (dict-ref rconfig 'cflags '()))
  (run-test config cxx cflags "../test/test-simple-ptr-copy-escape"))
