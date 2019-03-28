// Test HTTP interface
// todo: redo on gokit
package main

import (
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"os"
	"os/signal"
	"syscall"

	"github.com/godev/introducer"
)

const (
	boltconnection = "bolt://0.0.0.0:7687"
	user           = "neo4j"
	password       = "test"
)

var logmidlware = func(h http.HandlerFunc) http.HandlerFunc {
	return func(req http.ResponseWriter, w *http.Request) {
		defer log.Println("success func")
		h(req, w)
	}
}

/*
curl --header "Content-Type: application/json" \
  --request POST \
  --data '{"Name":"maxim","Email":"zhiburt@gmail.com", "Surname": "zhibur", "id": "123kj23l12j3lk1j23l21j123lk2kkkas"}' \
  http://localhost:8072/add
*/
func main() {
	mux := http.NewServeMux()
	mux.HandleFunc("/add", logmidlware(add))
	server := http.Server{Addr: ":8068", Handler: mux}

	SetupCloseHandler()

	log.Fatal(server.ListenAndServe())
}

type Person struct {
	Name    string
	Surname string
	Email   string
	ID      string
}

func add(w http.ResponseWriter, req *http.Request) {
	intro, err := introducer.NewIntroducer(boltconnection, user, password)
	if err != nil {
		log.Println("was not created service", err)
		w.WriteHeader(http.StatusInternalServerError)
		return
	}

	p := &Person{}
	err = json.NewDecoder(req.Body).Decode(p)
	if err != nil {
		log.Println("json invalid", err)
		w.WriteHeader(http.StatusBadRequest)
		return
	}

	id, err := intro.Know(introducer.NewPerson(p.Name, p.Surname, p.Email, p.ID))
	if err != nil {
		log.Println("service error", err)
		w.WriteHeader(http.StatusInternalServerError)
		return
	}

	jsn, err := json.MarshalIndent(id, "", " ")
	if err != nil {
		w.WriteHeader(http.StatusInternalServerError)
		return
	}

	w.WriteHeader(http.StatusCreated)
	w.Write(jsn)
}

func SetupCloseHandler() {
	c := make(chan os.Signal, 2)
	signal.Notify(c, os.Interrupt, syscall.SIGTERM)
	go func() {
		<-c
		fmt.Println("\r- Ctrl+C pressed in Terminal")
		os.Exit(0)
	}()
}
