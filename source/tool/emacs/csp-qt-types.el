;; CSP-Qt
(defun csp-qt-set-c-style ()
  "Set the current buffer's c-style to Common Source code Project/Qt with Visual Studio like style. "
  (progn
    ;; make new font for rest of CSP keywords
    (make-face 'csp-keywords-face)
    (set-face-foreground 'csp-keywords-face "SteelBlue2")
    (make-face 'csp-extend-face)
    (set-face-foreground 'csp-extend-face "OrangeRed1")
    (make-face 'csp-optimizer-face)
    (set-face-foreground 'csp-optimizer-face "Green2")
    (make-face 'csp-macros-face)
    (set-face-foreground 'csp-macros-face "Cyan3")
	
     (make-face 'csp-types-face)
     (set-face-foreground 'csp-types-face "SkyBlue")
	
	(font-lock-add-keywords
	 nil
	 '(
	   ("\\<\\sw+_t\\>" . 'font-lock-type-face)
	   ("\\<LPCTSTR\\>" . 'font-lock-type-face)
	   ("\\<LPTSTR\\>" . 'font-lock-type-face)
	   
	   ("\\<BOOL\\>" . 'font-lock-type-face)
	   
	   ("\\<BYTE\\>" . 'font-lock-type-face)
	   ("\\<WORD\\>" . 'font-lock-type-face)
	   ("\\<DWORD\\>" . 'font-lock-type-face)
	   ("\\<QWORD\\>" . 'font-lock-type-face)
	   
	   ("\\<INT\\>" . 'font-lock-type-face)
	   ("\\<INT[0-9]+\\>" . 'font-lock-type-face)
	   ("\\<UINT\\>" . 'font-lock-type-face)
	   ("\\<UINT[0-9]+\\>" . 'font-lock-type-face)
	   
	   ("\\<SINT\\>" . 'font-lock-type-face)
	   ("\\<SINT[0-9]+\\>" . 'font-lock-type-face)
	   ("\\<REG[0-9]+\\>" . 'font-lock-type-face)
	   
	   ("\\<DLL_PREFIX\\>" . 'csp-keywords-face)
	   ("\\<DLL_PREFIX_I\\>" . 'csp-keywords-face)
	   ("\\<__DECL_VECTORIZED_LOOP\\>" . 'csp-optimizer-face)
	   
	   ("\\<__FASTCALL\\>" . 'csp-optimizer-face)
	   ("___assume_aligned" . 'csp-optimizer-face)
	   ("__DECL_ALIGNED" . 'csp-optimizer-face)
	   
	   ("__LIKELY_IF" . 'csp-extend-face)
	   ("__UNLIKELY_IF" . 'csp-extend-face)

	   ("\\<_RGB[0-9]+\\>" . 'csp-macros-face)
	   ("\\<_RGBA[0-9]+\\>" . 'csp-macros-face)
	   ("[RGBA]_OF_COLOR" . 'csp-macros-face)
	   ("RGBA_COLOR" . 'csp-macros-face)
	   ("RGB_COLOR" . 'csp-macros-face)
	   
	   ("__BIG_ENDIAN__" . 'csp-macros-face)
	   ("__LITTLE_ENDIAN__" . 'font-lock-preprocessor-face)
	   
	   ("_MAX_PATH" . 'font-lock-constant-face)
	   ("\\<MAX_[A-Z0-9_]+\\>" . 'font-lock-constant-face)
	   ("\\<[A-Z0-9_]+_MAX_[A-Z0-9_]+\\>" . 'font-lock-constant-face)
	   
	   ("\\<USE_[A-Z0-9_]+\\>" . 'font-lock-constant-face)
	   ("\\<WITH_[A-Z0-9_]+\\>" . 'font-lock-preprocessor-face)
	   ("\\<WITHOUT_[A-Z0-9_]+\\>" . 'font-lock-preprocessor-face)
	   ("\\<HAS_[A-Z0-9_]+\\>" . 'font-lock-preprocessor-face)
	   
	   ("\\<OSD\\>" . 'csp-types-face)
	   ("\\<OSD_BASE\\>" . 'csp-types-face)
	   ("\\<CSP_[A-Z][a-zA-Z0-9_]+\\>" . 'csp-types-face)
	   ("\\<USING_FLAGS\\>" . 'csp-types-face)
	   ("\\<USING_FLAGS_[A-Z0-9_]+\\>" . 'csp-types-face)
	   ("\\<EMU\\>" . 'csp-types-face)
	   ("\\<EMU_TEMPLATE\\>" . 'csp-types-face)
	   ("\\<VM\\>" . 'csp-types-face)
	   ("\\<VM_TEMPLATE\\>" . 'csp-types-face)
	   ("\\<EmuThreadClass\\>" . 'csp-types-face)
	   ("\\<EmuThreadClass[A-Z][A-Za-z0-9_]+\\>" . 'csp-types-face)
	   
	   )
	 )
							
	)
  )
(add-hook 'c-mode-common-hook 'csp-qt-set-c-style)
