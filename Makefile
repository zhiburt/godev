program_name = godev

run: install

test:
	go vet ./...
	go fmt ./...
	go test -v -race -short

install:
	go build -o $(program_name) main.go
