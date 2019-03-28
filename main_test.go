package main

import (
	"fmt"
	"math"
	"testing"
)

func TestDiv(t *testing.T) {
	cases := []struct {
		params []float64
		result float64
	}{
		{[]float64{1, 2}, 0.5},
		{[]float64{10, 2, 2}, 2.5},
		{[]float64{3, 2}, 1.5},
		{[]float64{1}, 1},
	}

	for i, c := range cases {
		t.Run(fmt.Sprintf("case  [%d]", i), func(t *testing.T) {
			if result := Div(c.params...); math.Floor(result) != math.Floor(c.result) {
				t.Errorf("expected %.4f but was %.4f", c.result, result)
			}
		})
	}
}
