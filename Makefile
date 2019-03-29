program_name = introducer
introducer_folder = introducer/

run:  install
	./$(program_name)

test:
	go vet ./...
	go fmt ./...
	go test -v -race -short ./$(introducer_folder)

install:
	go build -o $(introducer_folder)build/$(program_name) $(introducer_folder)cli/main.go
