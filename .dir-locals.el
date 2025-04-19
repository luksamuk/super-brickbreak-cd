((nil
  (eval . (let ((root (projectile-project-root)))
	    (let ((includes (list "/opt/psn00bsdk/include/libpsn00b"
				  (concat root "include")))
		  (neotreebuf (seq-filter (lambda (buf) (equal (buffer-name buf) " *NeoTree*"))
					  (buffer-list))))
	      (setq-local flycheck-clang-include-path includes)
	      (setq-local flycheck-gcc-include-path includes)
	      ;; (dap-register-debug-template
	      ;;  "PSX Debug"
	      ;;  (list :name "PSX -- Engine debug"
	      ;; 	     :type "gdbserver"
	      ;; 	     :request "attach"
	      ;; 	     :gdbpath "/usr/bin/gdb-multiarch"
	      ;; 	     :target ":3333"
	      ;; 	     :cwd root))
	      )))))

