#include <iostream>
#include <unistd.h>  // Inclua esse cabeçalho para getpid
#include <cmath>
#include <ctime>

int main() {
    pid_t pid = getpid();  // Obtém o PID do processo atual
    std::cout << "PID do processo: " << pid << std::endl;

    // Simulando um cálculo pesado
    while (true) {
        double result = 0.0;
        for (int i = 0; i < 1000000; ++i) {
            result += std::sin(i) * std::cos(i);
        }
        // Apenas para que o cálculo pesado continue rodando
    }

    return 0;
}
