#lang racket

(struct test-config
  (name
    pre-fn
    post-fn
    ut-compile-tasks
    total-compile-task
    run-tasks
  )
)

(struct ut-compile-task
  (source target args)
)

(struct total-compile-task
  (sources target args)
)

(provide (all-defined-out))

(define (parse-ut-compile-task task cxx other-args root-path)
  (match-define (ut-compile-task source target args) task)
  `(,(force cxx) "-c"
    ,@(force other-args)
    ,@(force args)
    ,(build-path (force root-path) (force source))
    "-o"
    ,(build-path (force root-path) (force target))))

(define (parse-total-compile-task task cxx other-args root-path)
  (match-define (total-compile-task sources target args) task)
 `(,(force cxx)
   ,@(force other-args)
   ,@(force args)
   ,@(map (lambda (s) (build-path (force root-path) s)) (force sources))
   "-o"
   ,(build-path (force root-path) (force target))))

(define (run-test config cxx other-args root-path)
  (match-define (test-config name pre-fn post-fn ut-compile-tasks total-compile-task run-tasks) config)
  (eprintf "Running test: ~a~n" name)
  ((force pre-fn) (force root-path))
  (for ([t (force ut-compile-tasks)])
    (define ut-compile-args (parse-ut-compile-task t cxx other-args root-path))
    (eprintf "~a~n" ut-compile-args)
    (apply system* ut-compile-args)
  )
  (cond 
    [(force total-compile-task)
      (define total-compile-args (parse-total-compile-task (force total-compile-task) cxx other-args root-path))
      (when total-compile-args
        (eprintf "~a~n" total-compile-args)
        (apply system* total-compile-args))
    ])
  (define run-fn (force run-tasks))
  (when run-fn
    (run-fn cxx other-args root-path))
  ((force post-fn) (force root-path))
)
