program_name = introducer
introducer_folder = introducer/

run:  install
	./$(program_name)

test:
	go test -v -race -short

install:
	bash scripts/install_seabolt.sh
	go build -o build/$(program_name) cli/main.go
