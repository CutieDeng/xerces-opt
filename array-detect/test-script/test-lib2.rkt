#lang racket

(define (get-total-compile-task-files files args)
  (match-define `(,sources ,target) files)
  (total-compile-task sources target args)
)

(define (get-ut-compile-task-files/list files args)
  (map (lambda (ls) (ut-compile-task (car ls) (cadr ls) args)) files)
)

(require "./test-lib.rkt")

(provide (all-defined-out))
