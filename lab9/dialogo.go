package main

import (
	"fmt"
	"time"
)

func tarefa(str chan string) {
	// Array com as respostas da goroutine
	var respostas = [2]string{
		"Oi Main, bom dia, tudo bem?",
		"Certo, entendido.",
	}

	// Loop para receber e responder mensagens
	for i := 0; i < 2; i++ {
		// Recebe a mensagem da main e imprime
		msg := <-str
		fmt.Println("Main fala para goroutine:", msg)

		// Envia a resposta para a main
		str <- respostas[i]
	}

	// Goroutine imprime finalizando
	fmt.Println("Goroutine imprime: finalizando")
}

func main() {
	// Array com as mensagens que a main vai enviar para a goroutine
	var mensagens = [2]string{
		"Olá, Goroutine, bom dia!",
		"Tudo bem! Vou terminar tá?",
	}

	// Cria um canal de comunicação não-bufferizado
	str := make(chan string)

	// Cria uma goroutine que executará a função 'tarefa'
	go tarefa(str)

	// Loop para enviar mensagens e receber respostas
	for i := 0; i < 2; i++ {
		// Envia a mensagem para a goroutine
		str <- mensagens[i]

		// Recebe a resposta da goroutine e imprime
		resposta := <-str
		fmt.Println("Goroutine fala para main:", resposta)
	}

	// Main imprime finalizando
	fmt.Println("Main imprime: finalizando")

	// Pausa para garantir que a goroutine termine antes de o programa encerrar
	time.Sleep(time.Millisecond * 100)
}
