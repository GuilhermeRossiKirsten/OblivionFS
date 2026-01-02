# ReDoS - Regex Denial of Service

## O que é Backtracking?

Algums regex usam **backtracking**: o motor tenta várias combinações possíveis até decidir se a string casa ou não.

Um atacante pode mandar uma string maliciosa, feita para forçar o motor a tentar milhares/milhões de caminhos, causando lentidão extrema → isso é um **ReDoS (Regex Denial of Service)**.

## Exemplo Clássico de Regex Vulnerável

```regex
^(a+)+$
```

Ela parece simples, mas é perigosa.

## Código C++ Simples

```cpp
#include <iostream>
#include <regex>
#include <string>
#include <chrono>

int main() {
    // Regex vulnerável a backtracking excessivo
    std::regex vulnerable_regex("^(a+)+$");

    // String maliciosa: muitos 'a' e um '!' no final
    std::string input(30, 'a');
    input += "!";

    auto start = std::chrono::high_resolution_clock::now();

    bool match = std::regex_match(input, vulnerable_regex);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Match: " << match << std::endl;
    std::cout << "Tempo de execucao: " << duration.count() << " ms" << std::endl;

    return 0;
}
```

## O que está Acontecendo Aqui

### 1️⃣ A regex

```regex
^(a+)+$
```

Significa:

- Um ou mais grupos (`+`)
- Cada grupo tem um ou mais `a`
- Até o fim da string

**O problema** é que existem muitas formas diferentes de dividir os `a` em grupos.

### 2️⃣ A string maliciosa

```cpp
std::string input(30, 'a');
input += "!";
```

Exemplo:

```
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa!
```

👉 Parece quase válida, mas falha no final.

### 3️⃣ O backtracking explosivo

Quando o `!` aparece:

O motor tenta:

- "E se esse `a` fosse do grupo anterior?"
- "E se fosse de outro grupo?"
- "E se eu dividir assim?"
- "E assim?"

Ele tenta **todas as combinações possíveis** antes de concluir que não casa.

📈 **Tempo cresce de forma exponencial** conforme o número de `a`.

## Por que isso é um Exploit?

Em um backend real:

```cpp
regex_match(user_input, regex);
```

Um atacante pode:

- Enviar inputs gigantes
- Fazer sua API travar
- Consumir CPU
- Derrubar o serviço

➡️ **Ataque de negação de serviço usando regex.**

## Como Perceber que uma Regex é Perigosa

🚨 **Sinais clássicos:**

Quantificadores aninhados:

```regex
(a+)+
(.*)+
(.+)+
```

- Regex aparentemente simples, mas lenta com strings grandes
- Falha perto do final da string

## Explicação Visual do Backtracking

### O que é Backtracking?

**Backtracking** é quando o motor de regex:

1. Tenta um caminho
2. Se falha, **VOLTA** (backtrack)
3. Tenta outro caminho
4. Repete até encontrar sucesso ou esgotar todas as opções

### Exemplo Visual com `"aaa"` e regex `(a+)+`:

```
String: a  a  a
        ↓  ↓  ↓

Possíveis divisões em grupos:

1. [aaa]                    → 1 grupo com 3 a's
2. [aa][a]                  → 2 grupos: 2 a's, depois 1 a
3. [a][aa]                  → 2 grupos: 1 a, depois 2 a's
4. [a][a][a]                → 3 grupos: 1 a cada

TODAS funcionam! Mas o motor tenta várias até achar uma.
```

### Agora com `"aaaa!"` - NENHUMA divisão funciona (por causa do `!`):

```
String: a  a  a  a  !
        ↓  ↓  ↓  ↓  ↓

O motor tenta:
1. [aaaa] → Espera '$' mas encontra '!' → FALHA, backtrack
2. [aaa][a] → Espera '$' mas encontra '!' → FALHA, backtrack
3. [aa][aa] → Espera '$' mas encontra '!' → FALHA, backtrack
4. [aa][a][a] → Espera '$' mas encontra '!' → FALHA, backtrack
5. [a][aaa] → Espera '$' mas encontra '!' → FALHA, backtrack
6. [a][aa][a] → Espera '$' mas encontra '!' → FALHA, backtrack
7. [a][a][aa] → Espera '$' mas encontra '!' → FALHA, backtrack
8. [a][a][a][a] → Espera '$' mas encontra '!' → FALHA, backtrack

Todas as 2^n combinações são testadas!
```

### Crescimento Exponencial

Com N caracteres `'a'`:

- Existem 2^(N-1) formas de dividir em grupos

| N° de a's | Tentativas      | Tempo aprox. |
| --------- | --------------- | ------------ |
| 10        | 512             | < 1ms        |
| 20        | 524.288         | ~100ms       |
| 30        | 536.870.912     | ~minutos     |
| 40        | 549.755.813.888 | **dias!**    |

## Por que isso é um Ataque?

Um atacante pode enviar:

```http
POST /api/validate
{
  "username": "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa!"
}
```

E derrubar seu servidor! 💥

## Regex Segura vs. Vulnerável

```regex
❌ ^(a+)+$    → VULNERÁVEL (crescimento exponencial)
✅ ^a+$       → SEGURA (crescimento linear)
```

A segunda faz a **mesma validação**, mas sem backtracking explosivo!

## Como Compilar e Executar

```bash
g++ -std=c++11 redos_example.cpp -o redos_example
./redos_example
```

## Como Evitar ReDoS

### 1. Evite quantificadores aninhados

```regex
❌ (a+)+
❌ (.*)+
❌ (.+)+
✅ a+
✅ .*
✅ .+
```

### 2. Limite o tamanho do input

```cpp
if (input.length() > MAX_SIZE) return false;
```

### 3. Use ferramentas de análise

- https://devina.io/redos-checker
- https://regex101.com (mostra steps)

### 4. Teste com strings maliciosas

Sempre teste suas regex com strings grandes que **quase** casam mas falham no final.
