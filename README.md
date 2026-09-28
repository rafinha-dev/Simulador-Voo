# Simulador de Controle Aeroportuário

Projeto desenvolvido em **C** para simular operações básicas de controle aeroportuário e demonstrar a aplicação de diferentes estruturas de dados, principalmente **fila de prioridade, lista duplamente encadeada, fila FIFO e pilha**.

O sistema simula o ciclo de aeronaves desde a aproximação para pouso até a decolagem, incluindo circulação em taxiways, estacionamento em portões e operações de despacho de bagagens.

## Compilação

O projeto utiliza os seguintes arquivos principais:

```text
controle_aereo.c
controle_aereo.h
controle_solo.c
controle_solo.h
despacho_porao.c
despacho_porao.h
simulador.c
```

Em sistemas GNU/Linux com GCC, acesse pelo terminal o diretório do projeto e execute:

```bash
gcc -Wall -Wextra -Wpedantic -std=c11 \
    simulador.c \
    controle_aereo.c \
    controle_solo.c \
    despacho_porao.c \
    -lm \
    -o simulador
```

Após a compilação, será criado o executável:

```text
simulador
```

## Execução

Para executar utilizando o menu interativo de seleção do aeroporto:

```bash
./simulador
```

O programa apresentará os aeroportos disponíveis para a simulação.

Também é possível executar diretamente informando o aeroporto e o tempo máximo da simulação:

```bash
./simulador <aeroporto> <tempo>
```

Exemplo:

```bash
./simulador 2 400
```

Nesse exemplo, o aeroporto correspondente à opção `2` será utilizado durante uma simulação de até `400` segundos simulados.

Os parâmetros utilizados pelos aeroportos estão armazenados no diretório:

```text
aeroportos/
```

## Arquivos de log

Durante a execução, o simulador gera registros das operações realizadas.

### `simulador.log`

Mantém o histórico geral da simulação, permitindo acompanhar os principais acontecimentos do sistema em ordem cronológica.

### `torre_controle.log`

Registra as operações relacionadas ao controle aéreo e de solo, incluindo eventos como:

- geração e entrada de aeronaves no sistema;
- fila de prioridade;
- autorização e realização de pousos;
- utilização das pistas;
- entrada e movimentação nas taxiways;
- chegada aos portões;
- fila de saída;
- autorização de decolagem;
- conclusão do ciclo da aeronave.

### `despacho_porao.log`

Registra as operações relacionadas ao despacho de cargas e bagagens, incluindo:

- operações com o manifesto;
- embarque e descarregamento de itens;
- conferência entre o item físico e o cadastro;
- resultados do double-check;
- alertas relacionados às operações do porão.

Os arquivos de log auxiliam na observação do comportamento das estruturas de dados e na identificação da sequência das operações durante a simulação.

## Propósito

Este projeto possui finalidade acadêmica e foi desenvolvido para estudar a aplicação de **estruturas de dados em C** dentro de um problema simulado.

Entre as principais estruturas utilizadas estão:

- **Fila de prioridade baseada em heap:** organiza as aeronaves que aguardam autorização para pouso de acordo com as regras de prioridade.
- **Lista duplamente encadeada:** representa as posições utilizadas durante a circulação das aeronaves pelas taxiways.
- **Fila FIFO:** organiza aeronaves que aguardam a oportunidade de decolagem.
- **Pilha:** representa o acesso físico ao porão, no qual o último item embarcado é o primeiro acessível durante o descarregamento.

O objetivo principal não é reproduzir integralmente um sistema aeroportuário real, mas utilizar suas operações como contexto para testar e demonstrar o comportamento dessas estruturas.

## Limitações

O sistema é uma **simulação acadêmica** e não deve ser interpretado como implementação de um sistema real de controle de tráfego aéreo.

A infraestrutura dos aeroportos é representada de forma simplificada por arquivos de configuração. Os números e a organização utilizados pelo simulador servem como parâmetros para os testes e não necessariamente representam a infraestrutura operacional real e atual dos aeroportos.

A representação das taxiways também é simplificada. O sistema utiliza uma lista duplamente encadeada de posições, enquanto uma representação de um aeroporto real poderia exigir estruturas mais complexas, grafos são uma sujestão de implementação futura.

Tempos de pouso, circulação, permanência no portão, despacho e decolagem são tempos simulados e não representam necessariamente os tempos de operações aeroportuárias reais, aqui são em segundos.

O controle de conflitos implementado tem como finalidade permitir a experimentação com as estruturas de dados e evitar situações incompatíveis com as regras definidas para o protótipo. Ele não implementa os procedimentos completos utilizados por sistemas reais de gerenciamento de tráfego aéreo.

O manifesto, as bagagens e os demais dados utilizados durante a simulação também são dados fictícios gerados para demonstrar as operações de cadastro, pilha e conferência.

Portanto, o projeto deve ser utilizado exclusivamente para **estudo, demonstração e teste de estruturas de dados e algoritmos em linguagem C**.
