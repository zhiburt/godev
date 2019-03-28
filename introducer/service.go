package introducer

import (
	"log"
	"time"

	"github.com/neo4j/neo4j-go-driver/neo4j"
)

type (
	// Person is presentation of person
	// this data will be storaged
	Person interface {
		ID() string
		Email() string
		Firstname() string
		Surname() string
	}

	// Introducer is interface that  provide relationship Person A with B
	Introducer interface {
		Introduce(Person, Person) error
		Know(Person) (int64, error)
		Close() error
	}
)

// Communicator is implementation of Introducer
type Communicator struct {
	session  neo4j.Session
	driver   neo4j.Driver
	username string
	Introducer
}

// NewIntroducer  return introducer
func NewIntroducer(connstr, usr, pass string) (Introducer, error) {
	var (
		communicator Communicator
		err          error
	)

	communicator.driver, err = neo4j.NewDriver(connstr, neo4j.BasicAuth(usr, pass, ""))
	if err != nil {
		return communicator, err
	}

	communicator.username = usr
	communicator.session, err = communicator.driver.Session(neo4j.AccessModeWrite)

	return communicator, err
}

// Introduce will create relationship person1 with person2 by ID
func (comm Communicator) Introduce(person1, person2 Person) error {
	_, err := comm.session.WriteTransaction(func(txn neo4j.Transaction) (interface{}, error) {
		defer txn.Close()

		_, err := txn.Run(`
			MATCH (first:person),(second:person)
			WHERE first.person_id = $person_id AND second.person_id = $person_id2
			CREATE UNIQUE (first)-[r:FRIEND{since: $since}]->(second)
			RETURN first, second, r`,
			map[string]interface{}{
				"person_id":  person1.ID(),
				"person_id2": person2.ID(),
				"since":      neo4j.DateOf(time.Now()),
			},
		)
		return nil, err
	})

	return err
}

// Know will create node person and
// return node's id and error if that exists
func (comm Communicator) Know(person Person) (int64, error) {
	id, err := comm.session.WriteTransaction(func(txn neo4j.Transaction) (interface{}, error) {
		result, err := txn.Run(`
			CREATE (p:person)
			SET p.person_id = $person_id, p.name = $name, p.surname = $surname, p.email = $email
			RETURN p`,
			map[string]interface{}{
				"person_id": person.ID(),
				"name":      person.Firstname(),
				"surname":   person.Surname(),
				"email":     person.Email(),
			},
		)
		if err != nil {
			return 0, err
		}

		if result.Next() {
			log.Println("record id showed")
			return result.Record().GetByIndex(0).(neo4j.Node).Id(), nil
		}

		log.Println("result")
		return 0, result.Err()
	})
	if err != nil {
		return 0, err
	}

	return id.(int64), nil
}

// Close all connections
func (comm Communicator) Close() error {
	if err := comm.driver.Close(); err != nil {
		return err
	}
	if err := comm.session.Close(); err != nil {
		return err
	}

	return nil
}
