;;; vs-set-c-style.el --- Visual Studio like C/C++ style for c-mode
;; Use source/tool/emacs/csp-qt-c-style.el .
;; Please copy above to ~/.emacs.d/lisp/ or /usr/local/share/site-lisp etc.
;; And put below threelines to ~/.emacs and uncomment. --- 20180228 K.Ohta
;;; (autoload 'csp-qt-set-c-style "csp-qt-c-style")
;;; (add-hook 'c-mode-hook 'csp-qt-set-c-style)
;;; (add-hook 'c++-mode-hook 'csp-qt-set-c-style)

;; Keywords: c, tools
 ('csp-qt-set-c-style )
 ('vs-set-c-style )
 (setq c-file-style "csp-qt")
; (setq c-file-style "vs")

