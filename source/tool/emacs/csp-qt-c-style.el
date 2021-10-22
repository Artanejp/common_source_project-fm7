;;; csp-qt-c-style.el --- Common Source code Project/Qt with Visual Studio like C/C++ style for c-mode

;; Keywords: c, tools

;;
;; It is free software; you can redistribute it and/or modify it under the
;; terms of either:
;;
;; a) the GNU General Public License as published by the Free Software
;; Foundation; either version 1, or (at your option) any later version, or
;;
;; b) the "Artistic License".

;;; Commentary:
;; ToDo:
;;
;; Add supports for QT types and macros.
;;
;; Install:
;;
;; Adds in your emacs initialize file (~/.emacs.d/init.el or ~/.emacs)
;;
;;     (autoload 'csp-qt-set-c-style "csp-qt-c-style")
;;     (add-hook 'c-mode-hook 'csp-qt-set-c-style)
;;     (add-hook 'c++-mode-hook 'csp-qt-set-c-style)
;;

;;; Code:
(defconst csp-qt-set-c-style
  `("bsd"
    (tab-width . 4)
    (c-basic-offset . 4)
    (c-recognize-knr-p . nil)
    (c-enable-xemacs-performance-kludge-p . t) ; speed up indentation in XEmacs
	(c-noise-macro-names . (
							"__DECL_VECTORIZED_LOOP"
							"__FASTCALL"
							"INLINE"
							"MEMCALL"
							"IOINPCALL"
							"IOOUTCALL"
							"QT_\\sw+"
							"Q_\\sw+"
							)
						 )
	(c-font-lock-extra-types . (
								"\\sw+_t"
								"_TCHAR"
								)
							 )
	(c++-font-lock-extra-types . (
								"\\sw+_t"
								"_TCHAR"
								)
							 )
    (c-offsets-alist . ((statement-case-open . +)
                        (case-label . 0)))
    (c-hanging-braces-alist . ((defun-open          before after)
                               (defun-close 	    before after)
                               (class-open  	    before after)
                               (class-close 	    before)
                               (namespace-open      before after)
                               (namespace-close     before after)
			       (inline-open  	    before after)
                               (inline-close 	    before after)
                               (block-open   	    before after)
                               (block-close  	    before after)
                               (statement-case-open after)
                               (substatement-open   before after)
			       ))
    (c-hanging-colons-alist . ((case-label after)
			       (label after)
			       (access-label after)
                               (member-init-intro before)
			       ))
    (c-hanging-semi&comma-criteria . (c-semi&comma-inside-parenlist))
    (c-cleanup-list . (defun-close-semi
    		       list-close-comma
    		       scope-operator
    		       compact-empty-funcall))
    )
  "Common Source Code/Qt with Visual Studio like style")

(defun csp-qt-set-c-style ()
  "Set the current buffer's c-style to Common Source code Project/Qt with Visual Studio like style. "
  (font-lock-add-keywords nil
						  '(("\\<__DECL_VECTORIZED_LOOP\\>" . font-lock-preprocessor-face)
							("__LIKELY_IF" . font-lock-keyword-face)
							("__UNLIKELY_IF" . font-lock-keyword-face)
							("___assume_aligned" . font-lock-type-face)
							("__DECL_ALIGNED" . font-lock-type-face)
							("^\\(public\\|protected\\|private\\|signals\\|public\\s-slots\\|protected\\s-slots\\|private\\s-slots\\):" . font-lock-keyword-face)
							("\\<\\(public\\|protected\\|private\\)\\>" . font-lock-keyword-face)

							;("_T" . font-lock-string-face)
							("\\<\\sw+_t\\>" . font-lock-type-face)
							("\\<_TCHAR\\>" . font-lock-type-face)
							("\\<SOCKET\\>" . font-lock-type-face)
							
							("\\<LPCTSTR\\>" . font-lock-type-face)
							("\\<LPTSTR\\>" . font-lock-type-face)
							
							("\\<BOOL\\>" . font-lock-type-face)
							
							("\\<BYTE\\>" . font-lock-type-face)
							("\\<WORD\\>" . font-lock-type-face)
							("\\<DWORD\\>" . font-lock-type-face)
							("\\<QWORD\\>" . font-lock-type-face)

							("\\<INT\\>" . font-lock-type-face)
							("\\<INT[0-9]+\\>" . font-lock-type-face)
							("\\<UINT\\>" . font-lock-type-face)
							("\\<UINT[0-9]+\\>" . font-lock-type-face)
							
							("\\<SINT\\>" . font-lock-type-face)
							("\\<SINT[0-9]+\\>" . font-lock-type-face)
							("\\<REG[0-9]+\\>" . font-lock-type-face)

							("\\<Q_\\sw+\\>" . font-lock-preprocessor-face)
							("\\<QT_\\sw+\\>" . font-lock-preprocessor-face)
							
							("\\<Q\\sw+\\>" . font-lock-type-face)
							
							("\\<__FASTCALL\\>" . font-lock-keyword-face)
							("\\<INLINE\\>" . font-lock-keyword-face)
							("\\<MEMCALL\\>" . font-lock-keyword-face)
							("\\<IOINPCALL\\>" . font-lock-keyword-face)
							("\\<IOOUTCALL\\>" . font-lock-keyword-face)
							("\\<DLL_PREFIX\\>" . font-lock-keyword-face)
							("\\<DLL_PREFIX_I\\>" . font-lock-keyword-face)

							
							("\\<TRUE\\>" . font-lock-constant-face)
							("\\<FALSE\\>" . font-lock-constant-face)
							("\\<_MAX_PATH\\>" . font-lock-constant-face)					
							("\\<STATE_VERSION\\>" . font-lock-constant-face)					
							("\\<FRAMES_PER_SEC\\>" . font-lock-constant-face)					
							("\\<LINES_PER_FRAME\\>" . font-lock-constant-face)					
							("\\<SCREEN_\\sw+\\>" . font-lock-constant-face)											   ("\\<_USE_\\sw+\\>" . font-lock-constant-face)
							("\\<_HAS_\\sw+\\>" . font-lock-constant-face)
							("\\<_SUPPORT_\\sw+\\>" . font-lock-constant-face)
							
						    ("\\<USE_\\sw+\\>" . font-lock-preprocessor-face)
							("\\<HAS_\\sw+\\>" . font-lock-preprocessor-face)
							("\\<SUPPORT_\\sw+\\>" . font-lock-preprocessor-face)
							("\\<\\sw+_ADDR_MAX\\>" . font-lock-constant-face)
							("\\<\\sw+_SIZE\\>" . font-lock-constant-face)
							("\\<ONE_BOARD_MICRO_COMPUTER\\>" . font-lock-preprocessor-face)
							("\\<_ONE_BOARD_MICRO_COMPUTER\\>" . font-lock-constant-face)
							
							("\\<CONFIG_\\sw+\\>" . font-lock-preprocessor-face)
							("\\<CSP_LOG_\\sw+\\>" . font-lock-preprocessor-face)
							("\\<SIG_\\sw+\\>" . font-lock-constant-face)
							("\\<MAX_\\sw+\\>" . font-lock-constant-face)
							("\\<EVENT_\\sw+\\>" . font-lock-constant-face)
							("\\<SCSI_\\sw+\\>" . font-lock-constant-face)
							("\\<FILEIO_\\sw+\\>" . font-lock-constant-face)
							("\\<CPU_CLOCKS\\>" . font-lock-constant-face)
							("\\<MEDIA_TYPE_\\sw+\\>" . font-lock-constant-face)
							
							)
						  )
   (c-add-style "csp-qt" csp-qt-set-c-style t)
)

(provide 'csp-qt-set-c-style)
