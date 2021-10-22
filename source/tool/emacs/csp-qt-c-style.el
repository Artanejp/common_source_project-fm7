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
	(c-access-key . (
					 "\\<\\(signals\\|public\\|protected\\|private\\|public slots\\|protected slots\\|private slots\\):"
					 )
				  )
	(c-noise-macro-names . (
							"__DECL_VECTORIZED_LOOP"
							"__FASTCALL"
							"QT_BEGIN_NAMESPACE"
							"QT_END_NAMESPACE"
							"Q_OBJECT"
							)
						 )
	(c-font-lock-extra-types . (
								"\\sw+_t"
								)
							 )
	(c++-font-lock-extra-types . (
								"\\sw+_t"
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
						  '(("__DECL_VECTORIZED_LOOP" . font-lock-preprocessor-face)
							("__LIKELY_IF" . font-lock-keyword-face)
							("__UNLIKELY_IF" . font-lock-keyword-face)
							("___assume_aligned" . font-lock-type-face)
							("__DECL_ALIGNED" . font-lock-type-face)
							("\\<\\(signals\\|public slots\\|protected slots\\|private slots\\):" . font-lock-keyword-face)

							;("_T" . font-lock-string-face)
							("\\sw+_t" . font-lock-type-face)
							("QObject" . font-lock-type-face)
							("__FASTCALL" . font-lock-type-face)
							("DLL_PREFIX" . font-lock-type-face)
							("DLL_PREFIX_I" . font-lock-type-face)
							("QT_BEGIN_NAMESPACE" . font-lock-preprocessor-face)
							("QT_END_NAMESPACE" . font-lock-preprocessor-face)
							("Q_OBJECT" . font-lock-preprocessor-face)
							)
						  )
   (c-add-style "csp-qt" csp-qt-set-c-style t)
)

(provide 'csp-qt-set-c-style)
