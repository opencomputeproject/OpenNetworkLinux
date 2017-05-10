VERSION=1.6
USER=opennetworklinux
REPO=builder8

TOOLS=../../../tools/docker_shell ../../../tools/container-id

build: check_version
	cp $(TOOLS) .
	docker build -t $(USER)/$(REPO):$(VERSION) .
	rm -rf $(notdir $(TOOLS))

#
# Todo: Query remote repository to see if the request version already exists to avoid accidental overwrites
# when a new image is built but the VERSION variable is not updated.
#
check_version:

push:
	docker push $(USER)/$(REPO):$(VERSION)
