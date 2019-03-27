program_name = godev

run: install
	go test -v -race
	./godev

test:
	go vet ./...
	go fmt ./...
	go test -v -race -short

install:
	go build -o $(program_name) main.go
