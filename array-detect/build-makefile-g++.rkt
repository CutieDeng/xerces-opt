#!/usr/bin/env racket
#lang racket

(require "build-makefile.rkt")

(define cfg
  (struct-copy Config
               (make-config-with-cc (find-executable-path "g++"))
               ))

(module+ main
  (write-makefile* cfg))
