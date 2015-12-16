help:
	@echo
	@echo "Targets:"
	@echo "  build-clean : Clean local build files (extracts and packages). Leaves the debian packages."
	@echo "        clean : Clean all (build-clean and remove all packages). This will clear the entire repo."
	@echo

build-clean:
	@find packages -name Packages -delete
	@find packages -name Packages.gz -delete
	@rm -rf extracts

clean: build-clean
	@find packages -name "*.deb" -delete
