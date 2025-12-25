#!/usr/bin/env racket
#lang racket

(provide gmp-custom-path
         mpc-custom-path
         mpfr-custom-path)

;; ============================================================================
;; 库路径自定义配置
;; ============================================================================
;; 如果需要自定义库的位置，请在这里设置路径
;; 设置为 #f 表示使用自动检测
;; 路径应该是库的根目录（包含 include/ 和 lib/ 子目录）
;; ============================================================================

;; GMP 库路径
;; 示例 (macOS Homebrew): "/opt/homebrew/opt/gmp"
;; 示例 (Linux 自定义): "/usr/local/gmp-6.3.0"
;; 示例 (自动检测): #f
(define gmp-custom-path #f)

;; MPC 库路径
;; 示例 (macOS Homebrew): "/opt/homebrew/opt/libmpc"
;; 示例 (Linux 自定义): "/usr/local/mpc-1.3.1"
;; 示例 (自动检测): #f
(define mpc-custom-path #f)

;; MPFR 库路径
;; 示例 (macOS Homebrew): "/opt/homebrew/opt/mpfr"
;; 示例 (Linux 自定义): "/usr/local/mpfr-4.2.1"
;; 示例 (自动检测): #f
(define mpfr-custom-path #f)

;; ============================================================================
;; 配置说明
;; ============================================================================
;;
;; 1. 自动检测模式（默认）
;;    - 将所有路径设置为 #f
;;    - macOS: 使用 Homebrew 或 pkg-config
;;    - Linux: 使用 pkg-config
;;
;; 2. 自定义路径模式
;;    - 设置具体的路径字符串
;;    - 路径应该指向库的根目录
;;    - 示例：
;;      (define gmp-custom-path "/opt/custom/gmp")
;;      构建系统会使用：
;;        -I/opt/custom/gmp/include
;;        -L/opt/custom/gmp/lib
;;
;; 3. 混合模式
;;    - 可以只自定义部分库的路径
;;    - 其他库仍使用自动检测
;;    - 示例：
;;      (define gmp-custom-path "/custom/gmp")
;;      (define mpc-custom-path #f)  ; 自动检测
;;      (define mpfr-custom-path #f) ; 自动检测
;;
;; ============================================================================
