introduce_service_folder_name = introducer

install:
	$(MAKE) -C ./$(introduce_service_folder_name) install

test:
	$(MAKE) -C ./$(introduce_service_folder_name) test