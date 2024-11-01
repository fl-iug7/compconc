package main

import (
	"fmt"
	"math"
	"sync"
)

// Função para verificar se um número é primo
func ehPrimo(n int) bool {
	if n <= 1 {
		return false
	}

	if n == 2 {
		return true
	}

	if n%2 == 0 {
		return false
	}

	for i := 3; i <= int(math.Sqrt(float64(n)))+1; i += 2 {
		if n%i == 0 {
			return false
		}
	}

	return true
}

// Goroutine worker que recebe números para testar primalidade e registra o resultado
func worker(jobs <-chan int, results chan<- int, wg *sync.WaitGroup) {
	defer wg.Done()
	for n := range jobs {
		if ehPrimo(n) {
			results <- n // Envia o número primo encontrado para o canal de resultados
		}
	}
}

func main() {
	var N, M int
	fmt.Print("Digite o valor de N (limite superior): ")
	fmt.Scan(&N)
	fmt.Print("Digite o valor de M (número de goroutines): ")
	fmt.Scan(&M)

	// Canal para enviar números a serem verificados
	jobs := make(chan int, N)
	// Canal para receber os números primos
	results := make(chan int, N)

	var wg sync.WaitGroup

	// Cria M goroutines workers
	for i := 0; i < M; i++ {
		wg.Add(1)
		go worker(jobs, results, &wg)
	}

	// Envia números de 1 a N para o canal jobs
	go func() {
		for i := 1; i <= N; i++ {
			jobs <- i
		}
		close(jobs) // Fecha o canal jobs quando todos os números forem enviados
	}()

	// Lê os resultados em outra goroutine para evitar deadlock
	go func() {
		wg.Wait()
		close(results)
	}()

	// Coleta os números primos recebidos no canal results
	var primos []int
	for primo := range results {
		primos = append(primos, primo)
	}

	// Exibe o total e a lista de números primos encontrados
	fmt.Printf("Quantidade total de números primos de 1 a %d: %d\n", N, len(primos))
	fmt.Println("Números primos encontrados:", primos)
}
