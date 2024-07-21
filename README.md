
# Projeto de Compilador em C

## Índice
1. [Introdução](#introdução)
2. [Dependências](#dependências)
3. [Estrutura do Projeto](#estrutura-do-projeto)
4. [Makefile](#makefile)
5. [Tipos e Sintaxe](#tipos-e-sintaxe)
6. [Estrutura de Arquivos](#estrutura-de-arquivos)
7. [Objetivo do Projeto](#objetivo-do-projeto)
8. [Próximos Passos](#próximos-passos)

## Introdução
Este projeto é um compilador desenvolvido em C por Eduardo Peretto durante a disciplina de Compiladores na UFRGS. O projeto é baseado nos ensinamentos do livro *Engineering a Compiler* de Keith D. Cooper. A implementação do compilador é feita de forma gradativa, abrangendo diferentes módulos como análise léxica, análise sintática, inferência de tipos, tabelas de identificadores, geração de linguagem intermediária, entre outros.

## Dependências
Para o desenvolvimento deste projeto, é necessário a instalação das seguintes ferramentas:
- **FLEX**  
    - [(Documentação)](https://westes.github.io/flex/manual/)
    - Instalação (Ubuntu): ```sudo apt-get install flex```
- **BISON**
    - [(Documentação)](https://www.gnu.org/software/bison/manual/)
    - Instalação (Ubuntu): ```sudo apt-get install bison```
## Estrutura do Projeto
O compilador é dividido em diferentes etapas de implementação, cada uma correspondendo a um módulo específico do processo de compilação.
### Etapas de Implementação
1. **Etapa 1: Analisador Léxico**
   - Utiliza o FLEX para a análise léxica do código.
2. **Etapa 2: Analisador Sintático**
   - Utiliza o BISON para a análise sintática e geração da árvore de sintaxe abstrata (AST).
3. **Etapa 3: Geração da Árvore de Nodos (AST)**
   - Construção da árvore de nodos a partir do código analisado.
4. **Etapa 4: Tabela de Hash e Verificação de Tipos**
   - Criação de uma tabela hash para salvar identificadores e verificação de tipos.
5. **Etapa 5: Geração de Código Intermediário**
   - Geração de código intermediário na linguagem ILOC, conforme proposto no livro *Engineering a Compiler*.
6. **Etapa 6: Geração de Código Assembly**
   - Conversão do código intermediário para código assembly.

## Makefile
O projeto inclui um Makefile com os seguintes targets para facilitar a compilação e execução do código:
- `make`: Compila todo o projeto e gera o executável.
- `make run`: Executa o compilador usando o arquivo `input.txt` como entrada.
- `make clean`: Limpa todos os arquivos gerados pela compilação do projeto.

## Tipos e Sintaxe
Atualmente, a linguagem suportada pelo compilador possui três tipos básicos:
- **bool**
- **float**
- **int**

A sintaxe da linguagem possui algumas particularidades, como o uso de **vírgula** para finalizar uma expressão. 

### Declaração de funções
A declaração de uma função começa com a declaração de parâmetros cobertos por parênteses e **separados por ponto e vírgula**, seguido por um `|`, o tipo da função, uma barra `/`, seguida, finalmente, do nome da função. Após isso, abre-se o escopo utilizando chaves ```{ }```.

**Exemplo Simples:**
```c
(bool b; bool c) | int /a{
    if (b | c) {
        return 0,
    }
    return 1,
}
```
### Declaração de variáveis
A declaração de variáveis é dada pelo tipo, seguido de um ou mais identificadores separados por ponto e vírgula. Não é suportada a inicialização das variáveis junto à declaração.

**Exemplo Simples:**
```c
float a; b; c,
```

## Estrutura de Arquivos
- **main.c**: Arquivo de entrada do código, responsável por receber o input e acionar a análise do compilador.
- **hash_table.c**: Funções relacionadas à tabela hash de identificadores, variáveis e funções, incluindo a verificação de tipos.
- **parser.y**: Arquivo BISON com a análise sintática, geração de nodos e AST, contendo as principais regras de compilação.
- **scanner.l**: Analisador léxico, responsável pela leitura do código e conversão em tokens.
- **tree.c**: Funções relacionadas à geração de nodos e AST.

## Objetivo do Projeto
O objetivo deste projeto é proporcionar um aprendizado prático sobre o funcionamento de um compilador, permitindo a compreensão detalhada de cada etapa do processo de compilação. Por ser um projeto didático, alguns itens ou otimizações comuns em compiladores reais foram intencionalmente deixados de lado, a fim de simplificar a implementação e focar nos conceitos fundamentais de análise léxica, análise sintática, geração de código intermediário e outras etapas essenciais.

Que esse projeto possa servir de inspiração ou aprendizado a demais interessados. Sintam-se convidados a abrir pull requests ou entrar em contato para discutir implementações.

## Próximos Passos
Como desenvolvimento futuro, pretendo utilizar ferramentas visuais como gráficos, e permitir uma customização do compilador de maneira rápida, para observar efeitos causados.
