# Makefile
IMAGE_NAME = appimage-builder-rgb-d-overlay
CONTAINER_NAME = appimage-builder-rgb-d-overlay
APPIMAGE_NAME = viam-camera-rgb-d-overlay--aarch64.AppImage

appimage-aarch64: clean
	docker build -t $(IMAGE_NAME) .
	- docker container stop $(CONTAINER_NAME)
	- docker container rm $(CONTAINER_NAME)
	docker run --name $(CONTAINER_NAME) $(IMAGE_NAME)
	docker cp $(CONTAINER_NAME):/app/$(APPIMAGE_NAME) ./$(APPIMAGE_NAME)
	chmod +x ./${APPIMAGE_NAME}
	tar -czf module.tar.gz run.sh $(APPIMAGE_NAME)

clean:
	rm -f $(APPIMAGE_NAME)
	rm -f module.tar.gz
	- docker container stop $(CONTAINER_NAME)
	- docker container rm $(CONTAINER_NAME)
