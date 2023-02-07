;;; vs-set-c-style.el --- Visual Studio like C/C++ style for c-mode
;; Use source/tool/emacs/csp-qt-c-style.el .
;; Please copy above to ~/.emacs.d/lisp/ or /usr/local/share/site-lisp etc.
;; And put below threelines to ~/.emacs and uncomment. --- 20180228 K.Ohta

;; Important note:
;; If you using editorconfig for elisp,
;; please load this before loading editorconfig. 
;; -- 2023-02-07 K.Ohta.

;; DO NOT UNCOMMENT BELOW if you USE EDITORCONFIG,
;; PLACE below threee lines to .emacs .
;(autoload 'csp-qt-set-c-style "csp-qt-c-style")
;(add-hook 'c-mode-hook 'csp-qt-set-c-style)
;(add-hook 'c++-mode-hook 'csp-qt-set-c-style)

;; Keywords: c, tools
;('csp-qt-set-c-style )
;('vs-set-c-style )
;;(setq c-file-style "csp-qt")
;; (setq c-file-style "vs")
; "FILE" "lconv" "tm" "va_list" "jmp_buf" "istream" "istreambuf" "ostream" "ostreambuf" "ifstream" "ofstream" "fstream" "strstream" "strstreambuf" "istrstream" "ostrstream" "ios" "string" "rope" "list" "slist" "deque" "vector" "bit_vector" "set" "multiset" "map" "multimap" "hash" "hash_set" "hash_multiset" "hash_map" "hash_multimap" "stack" "queue" "priority_queue" "type_info" "iterator" "const_iterator" "reverse_iterator" "const_reverse_iterator" "reference" "const_reference" "int[0-9]+_t" "uint[0-9]+_t" "Qt3D[:alnum:]+" "Q[A-Z][:alnum:]+" "quint[0-9]+" "pair64_[tu]" "pair32_[tu]" "pair16_[tu]" "LPTSTR" "LPCTSTR" "BOOL" "BYTE" "WORD" "[DQ]WORD" "INT[8|16|32|64]" "INT" "UINT[8|16|32|64]" "UINT" "_TCHAR" "SOCKET" "qint[0-9]+"

;(font-lock-add-keywords
; 'c++-mode
; '(("\\sw+_t"
;	"qint[0-9]+"
;	"quint[0-9]+"
;	"BOOL"
;	"BYTE"
;	"WORD"
;	"[DQ]WORD"
;	.
;	font-lock-type-face
;    ))
; )
