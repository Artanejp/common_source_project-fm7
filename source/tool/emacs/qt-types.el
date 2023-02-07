;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Qt-GNU
;;
;; syntax-highlighting for Qt
 ;; (based on work by Arndt Gulbrandsen, Troll Tech)
; (defun jk/c-mode-common-hook ()
;   "Set up c-mode and related modes.
; 
; Includes support for Qt code (signal, slots and alikes)."
; 
;   ;; base-style
;   (c-set-style "stroustrup")
;   ;; set auto cr mode
;   (c-toggle-auto-hungry-state 1)
; 

 (defun jk/c-mode-common-hook ()
   "Set up c-mode and related modes.

    Includes support for Qt code (signal, slots and alikes)."
    ;; base-style
;   (c-set-style "stroustrup")
;   ;; set auto cr mode
;   (c-toggle-auto-hungry-state 1)
 
   ;; qt keywords and stuff ...
   ;; set up indenting correctly for new qt kewords
   (setq c-protection-key (concat "\\<\\(public\\|public slot\\|protected"
                                  "\\|protected slot\\|private\\|private slot"
                                  "\\)\\>")
         c-C++-access-key (concat "\\<\\(signals\\|public\\|protected\\|private"
                                  "\\|public slots\\|protected slots\\|private slots"
                                  "\\)\\>[ \t]*:"))
   (progn
     ;; modify the colour of slots to match public, private, etc ...
     (font-lock-add-keywords 'nil
                             '(("\\<\\(slots\\|signals\\)\\>" . font-lock-type-face)))
     ;; make new font for rest of qt keywords
     (make-face 'qt-keywords-face)
     (set-face-foreground 'qt-keywords-face "BlueViolet")
     ;; qt keywords
     (font-lock-add-keywords 'nil
                             '(("\\<SIGNAL\\|SLOT\\>" . 'qt-keywords-face)))
     (font-lock-add-keywords 'nil
                             '(("\\<Q[A-Z][A-Za-z]*" . 'font-lock-type-face)))

	 (font-lock-add-keywords 'nil
							 '(("\\<qint[0-9]+\\>" . 'font-lock-type-face)))
	 (font-lock-add-keywords 'nil
							 '(("\\<quint[0-9]+\\>" . 'font-lock-type-face)))

     (make-face 'qt-macros-face)
     (set-face-foreground 'qt-macros-face "Orange")
     (make-face 'qt-functions-face)
     (set-face-foreground 'qt-functions-face "LightSlateBlue")
     (font-lock-add-keywords 'nil
                             '(("\\<Q_OBJECT\\>" . 'qt-macros-face)))
     (font-lock-add-keywords 'nil
                             '(("\\<QT_BEGIN_NAMESPACE\\>" . 'qt-macros-face)))
     (font-lock-add-keywords 'nil
                             '(("\\<QT_END_NAMESPACE\\>" . 'qt-macros-face)))
     (font-lock-add-keywords 'nil
                             '(("\\<Q_DECLARE_METATYPE\\>" . 'qt-macros-face)))
     (font-lock-add-keywords 'nil
                             '(("\\<connect\\>" . 'qt-functions-face)))
     (font-lock-add-keywords 'nil
                             '(("\\<disconnect\\>" . 'qt-functions-face)))
     (font-lock-add-keywords 'nil
                             '(("\\<qobject_cast\\>" . 'qt-macros-face)))
	 ;; Another
     ))

(add-hook 'c-mode-common-hook 'jk/c-mode-common-hook)

