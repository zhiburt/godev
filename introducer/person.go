package introducer

// NewPerson returns object of Person interface
func NewPerson(nm, srnm, eml, id string) Person {
	return user{nm, srnm, eml, id}
}

// User is implementation of Person
type user struct {
	name    string
	surname string
	email   string
	id      string
}

func (u user) Firstname() string {
	return u.name
}

func (u user) Surname() string {
	return u.surname
}

func (u user) ID() string {
	return u.id
}

func (u user) Email() string {
	return u.email
}
