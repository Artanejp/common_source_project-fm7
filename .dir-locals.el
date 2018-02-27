;;; vs-set-c-style.el --- Visual Studio like C/C++ style for c-mode
;; Use source/tool/emacs/vs-set-c-style.el .
;; Please copy above to ~/.emacs.d/lisp/ or /usr/local/share/site-lisp etc.
;; And put below threelines to ~/.emacs and uncomment. --- 20180228 K.Ohta
;;; (autoload 'vs-set-c-style "vs-set-c-style")
;;; (add-hook 'c-mode-hook 'vs-set-c-style)
;;; (add-hook 'c++-mode-hook 'vs-set-c-style)

;; Keywords: c, tools
;(autoload 'vs-set-c-style "vs-set-c-style")
;(add-hook 'c-mode-hook 'vs-set-c-style)
;(add-hook 'c++-mode-hook 'vs-set-c-style)
;(add-hook 'c-mode-hook 'vs-set-c-style)
;(add-hook 'c++-mode-hook 'vs-set-c-style)
(
 (c-mode . ((c-style-file  . "vs-set-c-ctyle" )
			(subdirs . t))
		)
 (c++-mode . ((c-style-file . "vs-set-c-style")
			  (subdirs . t))
		)
)
