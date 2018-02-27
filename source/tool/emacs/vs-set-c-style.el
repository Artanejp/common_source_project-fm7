;;; vs-set-c-style.el --- Visual Studio like C/C++ style for c-mode

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

;; Intstall:
;;
;; Adds in your emacs initialize file (~/.emacs.d/init.el)
;;
;;     (autoload 'vs-set-c-style "vs-set-c-style")
;;     (add-hook 'c-mode-hook 'vs-set-c-style)
;;     (add-hook 'c++-mode-hook 'vs-set-c-style)
;;

;;; Code:

(defconst vs-c-style
  `("bsd"
    (tab-width . 4)
    (c-basic-offset . 4)
    (c-recognize-knr-p . nil)
    (c-enable-xemacs-performance-kludge-p . t) ; speed up indentation in XEmacs
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
  "Visual Studio like style")

(defun vs-set-c-style ()
  "Set the current buffer's c-style to Visual Studio like style. "
  (c-add-style "vs" vs-c-style t))

(provide 'vs-set-c-style)
