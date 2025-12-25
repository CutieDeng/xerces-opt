#!/usr/bin/env racket
#lang racket

(provide gmp-custom-include-path
         gmp-custom-lib-path
         mpc-custom-include-path
         mpc-custom-lib-path
         mpfr-custom-include-path
         mpfr-custom-lib-path)

;; ============================================================================
;; 库路径自定义配置
;; ============================================================================
;; 如果需要自定义库的位置，请在这里设置路径
;; 设置为 #f 表示使用自动检测
;; ============================================================================

;; GMP 库路径
;; Include 路径（包含 gmp.h 的目录）
;; 示例: "/opt/gmp-6.3.0/include"
;; 自动检测: #f
(define gmp-custom-include-path #f)

;; Lib 路径（包含 libgmp.so/libgmp.a 的目录）
;; 示例: "/opt/gmp-6.3.0/lib"
;; 自动检测: #f
(define gmp-custom-lib-path #f)

;; MPC 库路径
;; Include 路径（包含 mpc.h 的目录）
;; 示例: "/opt/mpc-1.3.1/include"
;; 自动检测: #f
(define mpc-custom-include-path #f)

;; Lib 路径（包含 libmpc.so/libmpc.a 的目录）
;; 示例: "/opt/mpc-1.3.1/lib"
;; 自动检测: #f
(define mpc-custom-lib-path #f)

;; MPFR 库路径
;; Include 路径（包含 mpfr.h 的目录）
;; 示例: "/opt/mpfr-4.2.1/include"
;; 自动检测: #f
(define mpfr-custom-include-path #f)

;; Lib 路径（包含 libmpfr.so/libmpfr.a 的目录）
;; 示例: "/opt/mpfr-4.2.1/lib"
;; 自动检测: #f
(define mpfr-custom-lib-path #f)

;; ============================================================================
;; 配置说明
;; ============================================================================
;;
;; 1. 自动检测模式（默认）
;;    - 将所有路径设置为 #f
;;    - macOS: 使用 Homebrew 或 pkg-config
;;    - Linux: 使用 pkg-config
;;
;; 2. 完整自定义模式
;;    - 同时设置 include 和 lib 路径
;;    - 示例：
;;      (define gmp-custom-include-path "/opt/gmp/include")
;;      (define gmp-custom-lib-path "/opt/gmp/lib")
;;
;; 3. 部分自定义模式
;;    - 只设置 include 或 lib 路径
;;    - 另一个使用自动检测
;;    - 示例：
;;      (define gmp-custom-include-path "/custom/include")
;;      (define gmp-custom-lib-path #f)  ; 使用自动检测
;;
;; 4. 混合模式
;;    - 不同库使用不同配置
;;    - 示例：
;;      ;; GMP 完全自定义
;;      (define gmp-custom-include-path "/opt/gmp/include")
;;      (define gmp-custom-lib-path "/opt/gmp/lib")
;;      ;; MPC 使用自动检测
;;      (define mpc-custom-include-path #f)
;;      (define mpc-custom-lib-path #f)
;;      ;; MPFR 只自定义 include
;;      (define mpfr-custom-include-path "/usr/local/include")
;;      (define mpfr-custom-lib-path #f)
;;
;; 5. 系统库路径
;;    - 如果库在标准系统路径，可能不需要显式指定
;;    - 但仍可以指定以确保使用特定版本
;;    - 示例：
;;      (define gmp-custom-include-path "/usr/include")
;;      (define gmp-custom-lib-path "/usr/lib64")
;;
;; ============================================================================
;; 路径格式说明
;; ============================================================================
;;
;; Include 路径要求：
;;   - 应该是包含头文件的目录
;;   - 例如：如果 gmp.h 位于 /opt/gmp/include/gmp.h
;;     则设置为 "/opt/gmp/include"
;;
;; Lib 路径要求：
;;   - 应该是包含库文件的目录
;;   - 例如：如果 libgmp.so 位于 /opt/gmp/lib/libgmp.so
;;     则设置为 "/opt/gmp/lib"
;;   - 支持 lib, lib64, lib/x86_64-linux-gnu 等不同命名
;;
;; ============================================================================
