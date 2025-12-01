#lang racket

(require racket/string)
(require racket/list)
(require racket/file)
(require file/glob)

(define out-dir "out")
(define sources '(
  "MemoryManagerImpl.cc"
  "PlatFormUtils.cc"
  "main_test.cpp"
  "XMemory.cc"
))
(define bin-dir "bin")
(define out-main "test-xercese")

;; 编译配置
(define compiler-exe "g++-15")
(define compiler-path (find-executable-path compiler-exe))
(define cxxflags `(
  "-std=c++17" "-Wall" "-Wextra" "-g" "-I."))

;; 清理之前的编译产物
(define (clean)
  (for
    ([f
      (append (glob (build-path out-dir "*.o"))
        (list (build-path bin-dir out-main)))])
    (when (file-exists? f)
      (eprintf "- rm ~a~n" f)
      (delete-file f))))

(define (pre-build)
  (make-directory* out-dir)
  (make-directory* bin-dir))

;; 编译单个源文件
(define (compile-source src-non-path)
  (define obj (path-replace-extension (build-path out-dir src-non-path) ".o"))
  (eprintf "+ g++(compile) ~a~n" src-non-path)
  (define rst (apply system* `(,compiler-path ,@cxxflags ,src-non-path "-c" "-o" ,(path->string obj))))
  (cond [(not rst) (eprintf "- g++ ~a (failed)~n") src-non-path])
  rst
)

;; 链接目标文件
(define (link-objects)
  (define objs (directory-list out-dir #:build? #t))
  (define objs-s (map path->string objs))
  (define out-main-p (build-path bin-dir out-main))
  (eprintf "+ g++(build) ~a~n" (path->string out-main-p))
  (define rst (apply system* `(,compiler-path ,@cxxflags ,@objs-s "-o" ,(path->string out-main-p))))
  (cond [(not rst) (eprintf "- g++ -o ~a (failed)~n" (path->string out-main-p))])
  rst
)

;; 运行测试程序
(define (run-test)
  (eprintf "+ test~n")
  (system (path->string (build-path bin-dir out-main)))
)

;; 主流程
(module+ main
  (and
    (clean)
    (pre-build)
    (for-each compile-source sources)
    (link-objects)
    (run-test)
  )
)
