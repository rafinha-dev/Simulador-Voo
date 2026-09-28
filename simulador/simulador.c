#include "controle_aereo.h"
#include "controle_solo.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define CAPACIDADE_FILA 10
#define CAPACIDADE_ATIVOS 100

#define TEMPO_POUSO 5
#define TEMPO_TAXIWAY 3
#define TEMPO_DECOLAGEM 5

#define INTERVALO_GERACAO_MIN 3
#define INTERVALO_GERACAO_MAX 6

#define TEMPO_PORTAO_MIN 8
#define TEMPO_PORTAO_MAX 15

typedef struct {

    Aviao **dados;

    int tamanho;
    int capacidade;

} ListaAtivos;

static volatile sig_atomic_t sistema_ativo = 1;

static FILE *arquivo_log = NULL;

static unsigned long contador_voos = 1;

static time_t proxima_geracao = 0;

static void encerrar_sistema(int sinal)
{
    (void)sinal;

    sistema_ativo = 0;
}

static void formatar_hora(
    time_t instante,
    char *saida,
    size_t tamanho
)
{
    struct tm *info;

    if (saida == NULL ||
        tamanho == 0) {

        return;
    }

    info = localtime(&instante);

    if (info == NULL) {

        snprintf(
            saida,
            tamanho,
            "--/--/---- --:--:--"
        );

        return;
    }

    strftime(
        saida,
        tamanho,
        "%d/%m/%Y %H:%M:%S",
        info
    );
}

static void registrar_log(
    const char *mensagem
)
{
    time_t agora;
    char hora[32];

    if (mensagem == NULL) {
        return;
    }

    agora = time(NULL);

    formatar_hora(
        agora,
        hora,
        sizeof(hora)
    );

    printf(
        "[%s] %s\n",
        hora,
        mensagem
    );

    if (arquivo_log != NULL) {

        fprintf(
            arquivo_log,
            "[%s] %s\n",
            hora,
            mensagem
        );

        fflush(arquivo_log);
    }
}

static const char *texto_sentido(
    Sentido sentido
)
{
    if (sentido == HORARIO) {
        return "horario";
    }

    return "anti-horario";
}

static const char *texto_estado(
    EstadoAviao estado
)
{
    switch (estado) {

        case APROXIMANDO:
            return "aproximando";

        case AGUARDANDO_POUSO:
            return "aguardando pouso";

        case POUSANDO:
            return "pousando";

        case AGUARDANDO_TAXIWAY:
            return "aguardando taxiway";

        case TAXIANDO:
            return "taxiando";

        case NO_PORTAO:
            return "no portao";

        case AGUARDANDO_DECOLAGEM:
            return "aguardando decolagem";

        case DECOLANDO:
            return "decolando";

        case CONCLUIDO:
            return "concluido";

        default:
            return "desconhecido";
    }
}

static int adicionar_ativo(
    ListaAtivos *ativos,
    Aviao *aviao
)
{
    if (ativos == NULL ||
        aviao == NULL ||
        ativos->dados == NULL) {

        return 0;
    }

    if (ativos->tamanho >= ativos->capacidade) {
        return 0;
    }

    ativos->dados[
        ativos->tamanho
    ] = aviao;

    ativos->tamanho++;

    return 1;
}

static void remover_ativo(
    ListaAtivos *ativos,
    int indice
)
{
    int i;

    if (ativos == NULL ||
        ativos->dados == NULL ||
        indice < 0 ||
        indice >= ativos->tamanho) {

        return;
    }

    for (
        i = indice;
        i < ativos->tamanho - 1;
        i++
    ) {

        ativos->dados[i] =
            ativos->dados[i + 1];
    }

    ativos->tamanho--;

    ativos->dados[
        ativos->tamanho
    ] = NULL;
}

static void agendar_geracao(
    time_t agora
)
{
    proxima_geracao =
        agora +
        INTERVALO_GERACAO_MIN +
        rand() %
        (
            INTERVALO_GERACAO_MAX -
            INTERVALO_GERACAO_MIN +
            1
        );
}

static void gerar_aviao_automaticamente(
    FilaPrioridade *fila,
    time_t agora
)
{
    Aviao *aviao;
    char mensagem[256];

    if (fila == NULL ||
        agora < proxima_geracao) {

        return;
    }

    aviao =
        malloc(sizeof(Aviao));

    if (aviao == NULL) {

        registrar_log(
            "ERRO: memoria insuficiente para gerar aeronave."
        );

        agendar_geracao(agora);

        return;
    }

    *aviao =
        gerar_aviao(contador_voos++);

    aviao->chegada = agora;

    aviao->estado =
        AGUARDANDO_POUSO;

    if (!inserir_aviao(
            fila,
            aviao
        )) {

        free(aviao);

        registrar_log(
            "ERRO: nao foi possivel inserir aeronave na fila de pouso."
        );

        agendar_geracao(agora);

        return;
    }

    snprintf(
        mensagem,
        sizeof(mensagem),
        "Aeronave %s gerada: combustivel=%d%%, prioridade=%d, sentido=%s.",
        aviao->identificacao,
        aviao->combustivel,
        aviao->prioridade,
        texto_sentido(aviao->sentido)
    );

    registrar_log(mensagem);

    if (aviao->prioridade == 1) {

        snprintf(
            mensagem,
            sizeof(mensagem),
            "Liberado para emergencia: %s entrou no topo da fila de pouso.",
            aviao->identificacao
        );

        registrar_log(mensagem);
    }

    agendar_geracao(agora);
}

static int tentar_iniciar_pouso(
    Aeroporto *aeroporto,
    FilaPrioridade *fila,
    ListaAtivos *ativos,
    time_t agora
)
{
    Aviao *aviao;
    int pista;
    char mensagem[256];

    if (aeroporto == NULL ||
        fila == NULL ||
        ativos == NULL ||
        fila_vazia(fila)) {

        return 0;
    }

    aviao =
        consultar_proximo_aviao(fila);

    if (aviao == NULL) {
        return 0;
    }

    pista =
        obter_pista_livre(aeroporto);

    if (pista < 0) {

        aviao->estado =
            AGUARDANDO_POUSO;

        return 0;
    }

    aviao =
        remover_proximo_aviao(fila);

    if (aviao == NULL) {
        return 0;
    }

    aviao->pista = pista;

    aviao->estado =
        POUSANDO;

    aviao->inicio_pouso =
        agora;

    aviao->fim_pouso =
        agora + TEMPO_POUSO;

    if (!ocupar_pista(
            aeroporto,
            pista,
            aviao,
            aviao->fim_pouso
        )) {

        aviao->estado =
            AGUARDANDO_POUSO;

        aviao->pista = -1;

        inserir_aviao(
            fila,
            aviao
        );

        return 0;
    }

    if (!adicionar_ativo(
            ativos,
            aviao
        )) {

        aeroporto->
            pistas_operacao[pista - 1].
            ocupada = 0;

        aeroporto->
            pistas_operacao[pista - 1].
            aviao = NULL;

        aeroporto->
            pistas_operacao[pista - 1].
            liberacao = 0;

        aviao->estado =
            AGUARDANDO_POUSO;

        aviao->pista = -1;

        inserir_aviao(
            fila,
            aviao
        );

        registrar_log(
            "ERRO: limite de aeronaves ativas atingido."
        );

        return 0;
    }

    snprintf(
        mensagem,
        sizeof(mensagem),
        "Pista %d liberada para pouso da aeronave %s (prioridade %d).",
        pista,
        aviao->identificacao,
        aviao->prioridade
    );

    registrar_log(mensagem);

    return 1;
}

static void concluir_pousos(
    Aeroporto *aeroporto,
    ListaAtivos *ativos,
    time_t agora
)
{
    int i;
    char mensagem[256];

    if (aeroporto == NULL ||
        ativos == NULL) {

        return;
    }

    for (
        i = 0;
        i < ativos->tamanho;
        i++
    ) {

        Aviao *aviao =
            ativos->dados[i];

        int entrada;

        if (aviao == NULL ||
            aviao->estado != POUSANDO ||
            aviao->fim_pouso > agora) {

            continue;
        }

        entrada =
            aviao->sentido == HORARIO
                ? aeroporto->inicio->id
                : aeroporto->fim->id;

        if (taxiway_livre(
                aeroporto,
                entrada
            )) {

            ocupar_taxiway(
                aeroporto,
                entrada,
                aviao
            );

            aviao->posicao_taxiway =
                entrada;

            aviao->entrada_taxiway =
                agora;

            aviao->proxima_movimentacao =
                agora + TEMPO_TAXIWAY;

            aviao->estado =
                TAXIANDO;

            snprintf(
                mensagem,
                sizeof(mensagem),
                "Aeronave %s pousou e entrou na taxiway N%d.",
                aviao->identificacao,
                entrada
            );

            registrar_log(mensagem);

        } else {

            aviao->estado =
                AGUARDANDO_TAXIWAY;

            aviao->posicao_taxiway =
                -1;

            aviao->proxima_movimentacao =
                agora;

            snprintf(
                mensagem,
                sizeof(mensagem),
                "Pouso concluido para %s, mas a entrada da taxiway esta bloqueada.",
                aviao->identificacao
            );

            registrar_log(mensagem);
        }
    }
}

static int existe_sentido_oposto(
    Aeroporto *aeroporto,
    Aviao *aviao
)
{
    No *no;

    if (aeroporto == NULL ||
        aviao == NULL) {

        return 0;
    }

    no =
        aeroporto->inicio;

    while (no != NULL) {

        if (
            no->ocupado &&
            no->aviao != NULL &&
            no->aviao != aviao &&
            no->aviao->sentido != aviao->sentido &&
            (
                no->aviao->estado == TAXIANDO ||
                no->aviao->estado == AGUARDANDO_TAXIWAY
            )
        ) {

            return 1;
        }

        no =
            no->proximo;
    }

    return 0;
}

static int proxima_posicao(
    Aeroporto *aeroporto,
    Aviao *aviao
)
{
    if (aeroporto == NULL ||
        aviao == NULL ||
        aeroporto->inicio == NULL ||
        aeroporto->fim == NULL) {

        return -1;
    }

    if (aviao->sentido == HORARIO) {

        if (
            aviao->posicao_taxiway >=
            aeroporto->fim->id
        ) {

            return -1;
        }

        return aviao->posicao_taxiway + 1;
    }

    if (
        aviao->posicao_taxiway <=
        aeroporto->inicio->id
    ) {

        return -1;
    }

    return aviao->posicao_taxiway - 1;
}

static void mover_taxiways(
    Aeroporto *aeroporto,
    ListaAtivos *ativos,
    time_t agora
)
{
    int i;

    char mensagem[256];

    if (aeroporto == NULL ||
        ativos == NULL) {

        return;
    }

    for (
        i = 0;
        i < ativos->tamanho;
        i++
    ) {

        Aviao *aviao =
            ativos->dados[i];

        int destino;
        int portao;

        if (aviao == NULL) {
            continue;
        }

        if (
            aviao->estado ==
            AGUARDANDO_TAXIWAY
        ) {

            int entrada =
                aviao->sentido == HORARIO
                    ? aeroporto->inicio->id
                    : aeroporto->fim->id;

            if (taxiway_livre(
                    aeroporto,
                    entrada
                )) {

                ocupar_taxiway(
                    aeroporto,
                    entrada,
                    aviao
                );

                aviao->posicao_taxiway =
                    entrada;

                aviao->entrada_taxiway =
                    agora;

                aviao->proxima_movimentacao =
                    agora + TEMPO_TAXIWAY;

                aviao->estado =
                    TAXIANDO;

                snprintf(
                    mensagem,
                    sizeof(mensagem),
                    "Aeronave %s entrou na taxiway N%d apos liberar o bloqueio.",
                    aviao->identificacao,
                    entrada
                );

                registrar_log(mensagem);
            }

            continue;
        }

        if (
            aviao->estado != TAXIANDO ||
            aviao->proxima_movimentacao > agora
        ) {

            continue;
        }

        destino =
            proxima_posicao(
                aeroporto,
                aviao
            );

        if (destino < 0) {

            portao =
                obter_portao_livre(
                    aeroporto
                );

            if (portao < 0) {

                aviao->proxima_movimentacao =
                    agora + 1;

                continue;
            }

            if (!ocupar_portao(
                    aeroporto,
                    portao,
                    aviao
                )) {

                aviao->proxima_movimentacao =
                    agora + 1;

                continue;
            }

            liberar_taxiway(
                aeroporto,
                aviao->posicao_taxiway
            );

            aviao->portao =
                portao;

            aviao->posicao_taxiway =
                -1;

            aviao->chegada_portao =
                agora;

            aviao->saida_portao =
                agora +
                TEMPO_PORTAO_MIN +
                rand() %
                (
                    TEMPO_PORTAO_MAX -
                    TEMPO_PORTAO_MIN +
                    1
                );

            aviao->estado =
                NO_PORTAO;

            snprintf(
                mensagem,
                sizeof(mensagem),
                "Aeronave %s chegou ao portao %d; taxiway liberada.",
                aviao->identificacao,
                portao
            );

            registrar_log(mensagem);

            continue;
        }

        if (!taxiway_livre(
                aeroporto,
                destino
            )) {

            if (
                existe_sentido_oposto(
                    aeroporto,
                    aviao
                ) &&
                !aviao->alerta_conflito_registrado
            ) {

                snprintf(
                    mensagem,
                    sizeof(mensagem),
                    "ALERTA: movimento de %s bloqueado; foi detectada aeronave em sentido contrario.",
                    aviao->identificacao
                );

                registrar_log(mensagem);

                registrar_log(
                    "SISTEMA DE SOLO interrompeu o movimento conflitante para evitar contramao."
                );

                aviao->alerta_conflito_registrado =
                    1;
            }

            aviao->proxima_movimentacao =
                agora + 1;

            continue;
        }

        liberar_taxiway(
            aeroporto,
            aviao->posicao_taxiway
        );

        ocupar_taxiway(
            aeroporto,
            destino,
            aviao
        );

        aviao->posicao_taxiway =
            destino;

        aviao->proxima_movimentacao =
            agora + TEMPO_TAXIWAY;

        aviao->alerta_conflito_registrado =
            0;

        snprintf(
            mensagem,
            sizeof(mensagem),
            "Aeronave %s avancou para taxiway N%d.",
            aviao->identificacao,
            destino
        );

        registrar_log(mensagem);
    }
}

static void iniciar_decolagens(
    Aeroporto *aeroporto,
    ListaAtivos *ativos,
    time_t agora
)
{
    int i;
    char mensagem[256];

    if (aeroporto == NULL ||
        ativos == NULL) {

        return;
    }

    for (
        i = 0;
        i < ativos->tamanho;
        i++
    ) {

        Aviao *aviao =
            ativos->dados[i];

        int pista;

        if (
            aviao == NULL ||
            (
                aviao->estado != NO_PORTAO &&
                aviao->estado != AGUARDANDO_DECOLAGEM
            ) ||
            aviao->saida_portao > agora
        ) {

            continue;
        }

        pista =
            obter_pista_livre(
                aeroporto
            );

        if (pista < 0) {

            aviao->estado =
                AGUARDANDO_DECOLAGEM;

            continue;
        }

        aviao->pista =
            pista;

        aviao->inicio_decolagem =
            agora;

        aviao->fim_decolagem =
            agora + TEMPO_DECOLAGEM;

        if (!ocupar_pista(
                aeroporto,
                pista,
                aviao,
                aviao->fim_decolagem
            )) {

            aviao->pista = -1;

            continue;
        }

        if (!liberar_portao(
                aeroporto,
                aviao->portao
            )) {

            aeroporto->
                pistas_operacao[pista - 1].
                ocupada = 0;

            aeroporto->
                pistas_operacao[pista - 1].
                aviao = NULL;

            aeroporto->
                pistas_operacao[pista - 1].
                liberacao = 0;

            aviao->pista = -1;

            continue;
        }

        aviao->portao = -1;

        aviao->estado =
            DECOLANDO;

        snprintf(
            mensagem,
            sizeof(mensagem),
            "Pista %d ocupada para decolagem da aeronave %s.",
            pista,
            aviao->identificacao
        );

        registrar_log(mensagem);
    }
}

static void concluir_decolagens(
    Aeroporto *aeroporto,
    ListaAtivos *ativos,
    time_t agora
)
{
    int i;
    char mensagem[256];

    if (aeroporto == NULL ||
        ativos == NULL) {

        return;
    }

    for (
        i = 0;
        i < ativos->tamanho;
        i++
    ) {

        Aviao *aviao =
            ativos->dados[i];

        if (
            aviao == NULL ||
            aviao->estado != DECOLANDO ||
            aviao->fim_decolagem > agora
        ) {

            continue;
        }

        aviao->estado =
            CONCLUIDO;

        aviao->pista = -1;

        snprintf(
            mensagem,
            sizeof(mensagem),
            "Aeronave %s concluiu a decolagem e deixou o aeroporto.",
            aviao->identificacao
        );

        registrar_log(mensagem);
    }
}

static void limpar_concluidos(
    ListaAtivos *ativos
)
{
    int i = 0;

    if (ativos == NULL) {
        return;
    }

    while (i < ativos->tamanho) {

        if (
            ativos->dados[i] != NULL &&
            ativos->dados[i]->estado == CONCLUIDO
        ) {

            free(
                ativos->dados[i]
            );

            remover_ativo(
                ativos,
                i
            );

        } else {

            i++;
        }
    }
}

static const char *arquivo_do_aeroporto(
    int opcao
)
{
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

static int selecionar_aeroporto(void)
{
    int opcao;

    printf(
        "Selecione o aeroporto:\n"
    );

    printf(
        "1 - Galeao\n"
        "2 - Confins\n"
        "3 - Guarulhos\n"
        "4 - Brasilia\n"
    );

    printf(
        "Opcao: "
    );

    if (scanf(
            "%d",
            &opcao
        ) != 1) {

        return 0;
    }

    return opcao;
}

static void mostrar_status(
    const Aeroporto *aeroporto,
    const ListaAtivos *ativos,
    const FilaPrioridade *fila
)
{
    int i;
    int ocupadas = 0;

    if (aeroporto == NULL ||
        ativos == NULL ||
        fila == NULL) {

        return;
    }

    for (
        i = 0;
        i < aeroporto->pistas;
        i++
    ) {

        if (
            aeroporto->
            pistas_operacao[i].
            ocupada
        ) {

            ocupadas++;
        }
    }

    printf(
        "\n--- STATUS ---\n"
    );

    printf(
        "Fila de pouso: %d aeronave(s)\n",
        fila->tamanho
    );

    printf(
        "Aeronaves ativas: %d\n",
        ativos->tamanho
    );

    printf(
        "Pistas ocupadas: %d/%d\n",
        ocupadas,
        aeroporto->pistas
    );

    for (
        i = 0;
        i < ativos->tamanho;
        i++
    ) {

        Aviao *aviao =
            ativos->dados[i];

        if (aviao != NULL) {

            printf(
                "  %s - %s",
                aviao->identificacao,
                texto_estado(
                    aviao->estado
                )
            );

            if (
                aviao->posicao_taxiway > 0
            ) {

                printf(
                    " - N%d",
                    aviao->posicao_taxiway
                );
            }

            if (aviao->portao > 0) {

                printf(
                    " - portao %d",
                    aviao->portao
                );
            }

            printf("\n");
        }
    }
}

int main(void)
{
    Aeroporto aeroporto;

    FilaPrioridade fila;

    ListaAtivos ativos;

    const char *arquivo_config;

    int opcao;

    time_t agora;
    time_t ultimo_status;

    srand(
        (unsigned int)time(NULL)
    );

    signal(
        SIGINT,
        encerrar_sistema
    );

    opcao =
        selecionar_aeroporto();

    arquivo_config =
        arquivo_do_aeroporto(
            opcao
        );

    if (arquivo_config == NULL) {

        fprintf(
            stderr,
            "Aeroporto invalido.\n"
        );

        return 1;
    }

    inicializar_aeroporto(
        &aeroporto
    );

    if (
        !carregar_configuracao(
            &aeroporto,
            arquivo_config
        ) ||
        !criar_circuito(
            &aeroporto
        ) ||
        !inicializar_portoes(
            &aeroporto
        ) ||
        !inicializar_pistas(
            &aeroporto
        )
    ) {

        fprintf(
            stderr,
            "Erro ao inicializar o aeroporto.\n"
        );

        destruir_aeroporto(
            &aeroporto
        );

        return 1;
    }

    inicializar_fila(
        &fila,
        CAPACIDADE_FILA
    );

    if (fila.dados == NULL) {

        fprintf(
            stderr,
            "Erro ao inicializar fila de prioridade.\n"
        );

        destruir_aeroporto(
            &aeroporto
        );

        return 1;
    }

    ativos.dados =
        calloc(
            CAPACIDADE_ATIVOS,
            sizeof(Aviao *)
        );

    ativos.tamanho = 0;
    ativos.capacidade =
        CAPACIDADE_ATIVOS;

    if (ativos.dados == NULL) {

        fprintf(
            stderr,
            "Erro ao alocar lista de aeronaves ativas.\n"
        );

        destruir_fila(
            &fila
        );

        destruir_aeroporto(
            &aeroporto
        );

        return 1;
    }

    arquivo_log =
        fopen(
            "simulador.log",
            "a"
        );

    if (arquivo_log == NULL) {

        fprintf(
            stderr,
            "Aviso: nao foi possivel abrir simulador.log.\n"
        );
    }

    imprimir_aeroporto(
        &aeroporto
    );

    imprimir_circuito(
        &aeroporto
    );

    registrar_log(
        "SIMULADOR iniciado."
    );

    registrar_log(
        "Geracao automatica de aeronaves ativada."
    );

    agora =
        time(NULL);

    agendar_geracao(
        agora
    );

    ultimo_status =
        agora;

    while (sistema_ativo) {

        agora =
            time(NULL);

        gerar_aviao_automaticamente(
            &fila,
            agora
        );

        tentar_iniciar_pouso(
            &aeroporto,
            &fila,
            &ativos,
            agora
        );

        concluir_pousos(
            &aeroporto,
            &ativos,
            agora
        );

        mover_taxiways(
            &aeroporto,
            &ativos,
            agora
        );

        iniciar_decolagens(
            &aeroporto,
            &ativos,
            agora
        );

        concluir_decolagens(
            &aeroporto,
            &ativos,
            agora
        );

        atualizar_pistas(
            &aeroporto,
            agora
        );

        limpar_concluidos(
            &ativos
        );

        if (
            agora - ultimo_status >= 10
        ) {

            mostrar_status(
                &aeroporto,
                &ativos,
                &fila
            );

            ultimo_status =
                agora;
        }

        sleep(1);
    }

    registrar_log(
        "SIMULADOR interrompido pelo operador."
    );

    for (
        int i = 0;
        i < ativos.tamanho;
        i++
    ) {

        free(
            ativos.dados[i]
        );
    }

    free(
        ativos.dados
    );

    destruir_fila(
        &fila
    );

    destruir_aeroporto(
        &aeroporto
    );

    if (arquivo_log != NULL) {

        fclose(
            arquivo_log
        );

        arquivo_log = NULL;
    }

    printf(
        "Simulador encerrado.\n"
    );

    return 0;
}
