#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Representa o sentido de circulação
 * de uma aeronave na taxiway.
 */
typedef enum {
    HORARIO,
    ANTI_HORARIO
} Sentido;


/*
 * Representa uma aeronave circulando
 * no sistema de solo.
 */
typedef struct {
    char identificacao[20];

    /* Nó atual da aeronave. */
    int posicao;

    /* Portão de destino. */
    int destino;

    /* Sentido de circulação. */
    Sentido sentido;

    /* 1 = em movimento, 0 = parada. */
    int em_movimento;

} Aviao;


/*
 * Representa um nó da lista duplamente encadeada.
 *
 * Nesta primeira versão, cada nó representa
 * uma posição do circuito de taxiways.
 */
typedef struct No {
    int id;

    /*
     * Indica se existe uma aeronave ocupando
     * atualmente este nó.
     */
    int ocupado;

    /*
     * Ponteiro para a aeronave que ocupa o nó.
     *
     * NULL significa que o nó está livre.
     */
    Aviao *aviao;

    /* Próximo nó do circuito. */
    struct No *proximo;

    /* Nó anterior do circuito. */
    struct No *anterior;

} No;


/*
 * Representa um portão de desembarque.
 */
typedef struct {
    int id;

    /* 0 = livre, 1 = ocupado. */
    int ocupado;

    /*
     * Aeronave atualmente estacionada.
     */
    Aviao *aviao;

} Portao;


/*
 * Representa o aeroporto carregado
 * a partir do arquivo de configuração.
 */
typedef struct {

    char nome[50];

    int pistas;

    int quantidade_taxiways;

    int quantidade_portoes;

    /*
     * Primeiro nó da lista duplamente
     * encadeada.
     */
    No *inicio;

    /*
     * Último nó da lista.
     *
     * Ter o ponteiro para o final facilita
     * futuras inserções.
     */
    No *fim;

    /*
     * Vetor de portões.
     */
    Portao *portoes;

} Aeroporto;


/*
 * Inicializa a estrutura do aeroporto.
 */
int inicializar_aeroporto(Aeroporto *aeroporto) {

    /*
     * Teste de integridade do ponteiro.
     */
    if (aeroporto == NULL) {
        return 0;
    }

    aeroporto->nome[0] = '\0';

    aeroporto->pistas = 0;

    aeroporto->quantidade_taxiways = 0;

    aeroporto->quantidade_portoes = 0;

    aeroporto->inicio = NULL;

    aeroporto->fim = NULL;

    aeroporto->portoes = NULL;

    return 1;
}


/*
 * Cria um novo nó para a lista.
 */
No *criar_no(int id) {

    No *novo = malloc(sizeof(No));

    /*
     * Teste de integridade após malloc.
     */
    if (novo == NULL) {
        return NULL;
    }

    novo->id = id;

    novo->ocupado = 0;

    novo->aviao = NULL;

    novo->proximo = NULL;

    novo->anterior = NULL;

    return novo;
}


/*
 * Adiciona um novo nó ao final da
 * lista duplamente encadeada.
 */
int adicionar_no(Aeroporto *aeroporto, int id) {

    /*
     * Verificação do ponteiro.
     */
    if (aeroporto == NULL) {
        return 0;
    }

    No *novo = criar_no(id);

    /*
     * Verifica se a alocação foi realizada.
     */
    if (novo == NULL) {
        return 0;
    }

    /*
     * Caso a lista esteja vazia.
     */
    if (aeroporto->inicio == NULL) {

        aeroporto->inicio = novo;

        aeroporto->fim = novo;

        return 1;
    }

    /*
     * O novo nó será colocado depois
     * do último nó.
     */
    if (aeroporto->fim == NULL) {

        /*
         * Situação inconsistente:
         * inicio existe, mas fim não.
         */
        free(novo);

        return 0;
    }

    novo->anterior = aeroporto->fim;

    aeroporto->fim->proximo = novo;

    aeroporto->fim = novo;

    return 1;
}


/*
 * Inicializa os portões do aeroporto.
 */
int inicializar_portoes(Aeroporto *aeroporto) {

    if (aeroporto == NULL) {
        return 0;
    }

    if (aeroporto->quantidade_portoes <= 0) {
        return 0;
    }

    /*
     * Reserva memória para os portões.
     */
    aeroporto->portoes =
        malloc(
            sizeof(Portao) *
            aeroporto->quantidade_portoes
        );

    /*
     * Teste de integridade do ponteiro.
     */
    if (aeroporto->portoes == NULL) {
        return 0;
    }

    /*
     * Inicializa todos os portões.
     */
    for (int i = 0;
         i < aeroporto->quantidade_portoes;
         i++) {

        aeroporto->portoes[i].id = i + 1;

        aeroporto->portoes[i].ocupado = 0;

        aeroporto->portoes[i].aviao = NULL;
    }

    return 1;
}


/*
 * Lê o arquivo de configuração do aeroporto.
 */
int carregar_configuracao(
    Aeroporto *aeroporto,
    const char *arquivo
) {

    /*
     * Verifica os ponteiros recebidos.
     */
    if (aeroporto == NULL || arquivo == NULL) {
        return 0;
    }

    FILE *fp = fopen(arquivo, "r");

    /*
     * Verifica se o arquivo foi aberto.
     */
    if (fp == NULL) {

        printf(
            "\nErro ao abrir o arquivo: %s\n",
            arquivo
        );

        return 0;
    }

    char linha[100];

    /*
     * Lê o arquivo linha por linha.
     */
    while (fgets(linha, sizeof(linha), fp) != NULL) {

        /*
         * Remove o '\n' da linha.
         */
        linha[strcspn(linha, "\n")] = '\0';

        /*
         * Lê o nome do aeroporto.
         */
        if (strncmp(linha, "NOME=", 5) == 0) {

            sscanf(
                linha + 5,
                "%49[^\n]",
                aeroporto->nome
            );
        }

        /*
         * Lê a quantidade de pistas.
         */
        else if (strncmp(linha, "PISTAS=", 7) == 0) {

            sscanf(
                linha + 7,
                "%d",
                &aeroporto->pistas
            );
        }

        /*
         * Lê a quantidade de taxiways.
         */
        else if (strncmp(linha, "TAXIWAYS=", 9) == 0) {

            sscanf(
                linha + 9,
                "%d",
                &aeroporto->quantidade_taxiways
            );
        }

        /*
         * Lê a quantidade de portões.
         */
        else if (strncmp(linha, "PORTOES=", 8) == 0) {

            sscanf(
                linha + 8,
                "%d",
                &aeroporto->quantidade_portoes
            );
        }
    }

    fclose(fp);

    /*
     * Verifica se os valores básicos
     * foram realmente carregados.
     */
    if (aeroporto->nome[0] == '\0' ||
        aeroporto->pistas <= 0 ||
        aeroporto->quantidade_taxiways <= 0 ||
        aeroporto->quantidade_portoes <= 0) {

        printf(
            "\nErro: configuração do aeroporto inválida.\n"
        );

        return 0;
    }

    return 1;
}


/*
 * Cria os nós do circuito de taxiways.
 *
 * Nesta versão, cada taxiway será representada
 * por um nó da lista.
 *
 * Posteriormente podemos separar:
 *
 * nó = posição/interseção
 * taxiway = conexão entre posições
 */
int criar_circuito(Aeroporto *aeroporto) {

    if (aeroporto == NULL) {
        return 0;
    }

    if (aeroporto->quantidade_taxiways <= 0) {
        return 0;
    }

    /*
     * Cria a quantidade de nós definida
     * no arquivo do aeroporto.
     */
    for (int i = 1;
         i <= aeroporto->quantidade_taxiways;
         i++) {

        if (!adicionar_no(aeroporto, i)) {

            return 0;
        }
    }

    return 1;
}


/*
 * Exibe as informações da infraestrutura.
 */
void imprimir_aeroporto(
    const Aeroporto *aeroporto
) {

    if (aeroporto == NULL) {
        return;
    }

    printf("\nAeroporto selecionado\n");

    printf(
        "Nome: %s\n",
        aeroporto->nome
    );

    printf(
        "Pistas de pouso/decolagem: %d\n",
        aeroporto->pistas
    );

    printf(
        "Taxiways: %d\n",
        aeroporto->quantidade_taxiways
    );

    printf(
        "Portões: %d\n",
        aeroporto->quantidade_portoes
    );
}


/*
 * Exibe a lista duplamente encadeada.
 */
void imprimir_circuito(
    const Aeroporto *aeroporto
) {

    if (aeroporto == NULL) {
        return;
    }

    printf("\nCircuito de taxiways:\n");

    const No *atual = aeroporto->inicio;

    while (atual != NULL) {

        printf(
            "[N%d]",
            atual->id
        );

        if (atual->proximo != NULL) {
            printf(" <-> ");
        }

        atual = atual->proximo;
    }

    printf("\n");
}


/*
 * Exibe os portões.
 */
void imprimir_portoes(
    const Aeroporto *aeroporto
) {

    if (aeroporto == NULL) {
        return;
    }

    printf("\nPortões:\n");

    for (int i = 0;
         i < aeroporto->quantidade_portoes;
         i++) {

        printf(
            "Portão %d: %s\n",
            aeroporto->portoes[i].id,
            aeroporto->portoes[i].ocupado
                ? "ocupado"
                : "livre"
        );
    }
}


/*
 * Libera os nós da lista.
 */
void liberar_circuito(Aeroporto *aeroporto) {

    if (aeroporto == NULL) {
        return;
    }

    No *atual = aeroporto->inicio;

    while (atual != NULL) {

        No *proximo = atual->proximo;

        free(atual);

        atual = proximo;
    }

    aeroporto->inicio = NULL;

    aeroporto->fim = NULL;
}


/*
 * Libera todos os recursos utilizados
 * pelo aeroporto.
 */
void destruir_aeroporto(
    Aeroporto *aeroporto
) {

    if (aeroporto == NULL) {
        return;
    }

    liberar_circuito(aeroporto);

    if (aeroporto->portoes != NULL) {

        free(aeroporto->portoes);

        aeroporto->portoes = NULL;
    }
}


/*
 * Exibe o menu de seleção.
 */
int selecionar_aeroporto(void) {

    int opcao;

    printf("\n");
    printf("Sistema de Controle de Solo\n");
    printf("\n");

    printf("Selecione o aeroporto:\n");

    printf("1 - Galeao\n");
    printf("2 - Confins\n");
    printf("3 - Guarulhos\n");
    printf("4 - Brasilia\n");

    printf("\nOpcao: ");

    if (scanf("%d", &opcao) != 1) {

        return 0;
    }

    return opcao;
}


/*
 * Retorna o caminho do arquivo
 * correspondente ao aeroporto escolhido.
 */
const char *obter_arquivo(int opcao) {

    switch (opcao) {

        case 1:
            return "aeroportos/galeao.cfg";

        case 2:
            return "aeroportos/confins.cfg";

        case 3:
            return "aeroportos/guarulhos.cfg";

        case 4:
            return "aeroportos/brasilia.cfg";

        default:
            return NULL;
    }
}


/*
 * Programa principal.
 */
int main(void) {

    Aeroporto aeroporto;

    /*
     * Inicializa a estrutura vazia.
     */
    if (!inicializar_aeroporto(&aeroporto)) {

        printf(
            "Erro ao inicializar o sistema.\n"
        );

        return 1;
    }

    /*
     * Solicita ao usuário o aeroporto.
     */
    int opcao = selecionar_aeroporto();

    if (opcao < 1 || opcao > 4) {

        printf(
            "\nOpcao de aeroporto invalida.\n"
        );

        return 1;
    }

    /*
     * Obtém o arquivo correspondente.
     */
    const char *arquivo =
        obter_arquivo(opcao);

    /*
     * Teste de integridade do ponteiro.
     */
    if (arquivo == NULL) {

        printf(
            "\nErro: arquivo do aeroporto "
            "nao encontrado.\n"
        );

        return 1;
    }

    printf(
        "\nCarregando: %s\n",
        arquivo
    );

    /*
     * Carrega os dados do arquivo.
     */
    if (!carregar_configuracao(
            &aeroporto,
            arquivo
        )) {

        destruir_aeroporto(&aeroporto);

        return 1;
    }

    /*
     * Cria a lista de taxiways.
     */
    if (!criar_circuito(&aeroporto)) {

        printf(
            "\nErro ao criar o circuito.\n"
        );

        destruir_aeroporto(&aeroporto);

        return 1;
    }

    /*
     * Cria os portões.
     */
    if (!inicializar_portoes(&aeroporto)) {

        printf(
            "\nErro ao inicializar os portoes.\n"
        );

        destruir_aeroporto(&aeroporto);

        return 1;
    }

    /*
     * Mostra o resultado da inicialização.
     */
    imprimir_aeroporto(&aeroporto);

    imprimir_circuito(&aeroporto);

    imprimir_portoes(&aeroporto);

    /*
     * Libera toda a memória utilizada.
     */
    destruir_aeroporto(&aeroporto);

    return 0;
}
