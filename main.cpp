#include <iostream>
#include <regex>
#include <string>
#include <chrono>

void regex_dos(int test_number, int number_of_as) {
    // Regex vulnerável a backtracking excessivo
    std::regex vulnerable_regex("^(a+)+$");

    // String maliciosa: muitos 'a' e um '!' no final
    std::string input(number_of_as, 'a');
    input += "!";

    auto start = std::chrono::high_resolution_clock::now();

    bool match = std::regex_match(input, vulnerable_regex);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Teste " << test_number << " -> std::string input(" << number_of_as << ", 'a')" << std::endl;
    std::cout << "Match: " << match << std::endl;
    std::cout << "Tempo de execucao: " << duration.count() << " ms" << std::endl;
}

int main() {
    // Testes com diferentes tamanhos de entrada
    regex_dos(1, 10);      // 10 'a's
    regex_dos(2, 20);      // 20 'a's
    regex_dos(3, 30);      // 30 'a's
    regex_dos(4, 40);      // 40 'a's
    return 0;
}

