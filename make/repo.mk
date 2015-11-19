PACKAGES := $(wildcard *.deb)

Packages: $(PACKAGES)
	@echo "Updating Package Manifest..."
	$(ONL_V_at) dpkg-scanpackages . > Packages

