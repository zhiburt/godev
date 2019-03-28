package main

import (
	"fmt"
)

func main() {
	fmt.Println(Div(3, 2))
}

// Div divide some arguments
func Div(numbers ...float64) (res float64) {
	res = numbers[0]
	for _, num := range numbers[1:] {
		res = res / num
	}

	return
}
