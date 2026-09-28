/*
 * Simulador: integra torre de controle, solo e despacho do porao.
 * O tempo pode ser real (menu) ou simulado (argumentos).
 * Cada evento e registrado por setor para facilitar a auditoria.
 */
#define _POSIX_C_SOURCE 200809L
#include "controle_aereo.h"
#include "controle_solo.h"
#include "despacho_porao.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
// Limites de memoria e duracoes em segundos simulados.
#define MAX_ATIVOS 256
#define MAX_SAIDA 256
#define TEMPO_POUSO 4
#define TEMPO_TAXI 2
#define TEMPO_DESCARGA 2
#define TEMPO_SOLO 5
#define TEMPO_DECOLAGEM 4
#define INTERVALO_GERACAO 5
#define MAX_GERADOS 30

/* Cada operacao associa uma aeronave ao SEU manifesto e porao. */
typedef struct {
    Aviao *aviao;
    // Aeronave acompanhada pela simulacao.
    Manifesto manifesto;
    // Cadastro digital exclusivo do voo.
    Porao porao;
    // Pilha fisica exclusiva do voo.
} Operacao;
// Vetor de operacoes ainda nao concluidas.
typedef struct {
    Operacao *itens[MAX_ATIVOS];
    int quantidade;
} Ativos;
// Fila circular FIFO de aeronaves prontas para decolar.
typedef struct {
    Aviao *itens[MAX_SAIDA];
    int inicio, quantidade;
} FilaSaida;
// Arquivos de auditoria: geral, torre/solo e despacho de cargas.
static FILE *arquivo_log;
static FILE *arquivo_torre;
static FILE *arquivo_despacho;
// Sinalizado pelo Ctrl+C para encerrar o laco com seguranca.
static volatile sig_atomic_t executar = 1;
static void interromper(int sinal) {
    (void) sinal;
    executar = 0;
}
/*
 * Registra cada evento com data/hora no console, no log geral e
 * no arquivo do setor correspondente. Os eventos de sistema ficam
 * apenas no log geral. O prefixo permite filtrar eventos facilmente.
 */
static void logar_setor(time_t agora, const char *setor, const char *formato,...) {
    char texto[512], data[40];
    struct tm tm_local;
    va_list ap;
    localtime_r( &agora, &tm_local);
    strftime(data, sizeof(data), "%d/%m/%Y %H:%M:%S", &tm_local);
    va_start(ap, formato);
    vsnprintf(texto, sizeof(texto), formato, ap);
    va_end(ap);
    FILE *destino = NULL;
    if (strcmp(setor, "TORRE") == 0) {
        destino = arquivo_torre;
    }
    else if (strcmp(setor, "DESPACHO") == 0) destino = arquivo_despacho;
    printf("[%s] [%s] %s\n", data, setor, texto);
    if (arquivo_log) {
        fprintf(arquivo_log, "[%s] [%s] %s\n", data, setor, texto);
        fflush(arquivo_log);
    }
    if (destino) {
        fprintf(destino, "[%s] [%s] %s\n", data, setor, texto);
        fflush(destino);
    }
}
// Atalhos para separar as responsabilidades sem duplicar formatacao.
#define log_torre(agora, ...) logar_setor(agora,"TORRE",__VA_ARGS__)
#define log_despacho(agora, ...) logar_setor(agora,"DESPACHO",__VA_ARGS__)
#define log_sistema(agora, ...) logar_setor(agora,"SISTEMA",__VA_ARGS__)

/* Acrescenta um voo ao fim da fila circular de saida. */
static int saida_inserir(FilaSaida *f, Aviao *a) {
    if (f->quantidade == MAX_SAIDA) {
        return 0;
    }
    f->itens[(f->inicio + f->quantidade) % MAX_SAIDA] = a;
    f->quantidade++;
    return 1;
}
/* Consulta o primeiro voo aguardando decolagem. */
static Aviao *saida_primeiro(FilaSaida *f) {
    return f->quantidade ? f->itens[f->inicio] : NULL;
}
/* Retira o primeiro voo e avanca o indice circular. */
static void saida_remover(FilaSaida *f) {
    if (f->quantidade) {
        f->itens[f->inicio] = NULL;
        f->inicio = (f->inicio + 1) % MAX_SAIDA;
        f->quantidade--;
    }
}
/* Simula o cadastro previo e embarque de itens de cada voo. */
static void preparar_carga(Operacao *op, time_t agora) {
    char codigo[32], dono[64];
    int i, quantidade = 2 + rand() % 4;
    inicializar_carga(&op->manifesto,&op->porao);
    for (i = 0; i < quantidade; i++) {
        snprintf(codigo, sizeof(codigo), "%s-B%02d", op->aviao->identificacao, i + 1);
        snprintf(dono, sizeof(dono), "Passageiro %02d", i + 1);
        if (!cadastrar_item(&op->manifesto, codigo, dono, 5.0 + i * 2.0) || !embarcar_item(&op->manifesto,&op->porao, codigo)) {
            log_despacho(agora, "ERRO ao preparar carga %s", codigo);
        }
    }
    log_despacho(agora, "MANIFESTO %s: %d itens previamente cadastrados e embarcados", op->aviao->identificacao, op->porao.quantidade);
}
/* Converte a categoria do voo em texto para os logs. */
static const char *nome_tipo_voo(TipoVoo tipo) {
    switch (tipo) {
        case VOO_COMERCIAL: return "Comercial";
        case VOO_CARGA: return "Carga";
        case VOO_PRIVADO: return "Particular";
        default: return "Desconhecido";
    }
}
/* Converte a prioridade calculada em texto. */
static const char *descricao_prioridade(int prioridade) {
    switch (prioridade) {
        case 1: return "Emergencia";
        case 2: return "Comercial";
        case 3: return "Carga/Particular";
        default: return "Indefinida";
    }
}
/* Exibe menu e valida a opcao digitada. */
static int escolher_aeroporto(void) {
    char linha[64], extra;
    int escolha;
    for (;;) {
        printf("\nSELECIONE O AEROPORTO\n");
        printf("1 - Galeao\n2 - Confins\n3 - Guarulhos\n4 - Brasilia\n");
        printf("Opcao (1-4): ");
        fflush(stdout);
        if (!fgets(linha, sizeof(linha), stdin)) {
            return 0;
        }
        if (sscanf(linha, " %d %c", &escolha, &extra) == 1 && escolha >= 1 && escolha <= 4) {
            return escolha;
        }
        printf("Opcao invalida. Informe um numero de 1 a 4.\n");
    }
}
/* Valida um argumento numerico sem aceitar caracteres extras. */
static int argumento_inteiro(const char *texto, int minimo, int maximo, int *resultado) {
    char *fim;
    long numero;
    if (!texto || !resultado || !*texto) {
        return 0;
    }
    numero = strtol(texto, &fim, 10);
    if (* fim != '\0' || numero < minimo || numero > maximo) {
        return 0;
    }
    *resultado = (int) numero;
    return 1;
}
/* Registra infraestrutura e politica operacional no inicio. */
static void apresentar_aeroporto(const Aeroporto *a, time_t agora) {
    char nos[512];
    size_t usado = 0;
    int i;
    if (!a) {
        return;
    }
    log_torre(agora, "AEROPORTO SELECIONADO: %s", a->nome);
    log_torre(agora, "INFRAESTRUTURA: pistas=%d, nos de taxiway=%d, portoes=%d", a->quantidade_pistas, a->quantidade_taxiways, a->quantidade_portoes);
    nos[0] = '\0';
    for (i = 1; i <= a->quantidade_taxiways; i++) {
        int escrito = snprintf(nos + usado, sizeof(nos) - usado, "%sN%d", i == 1 ? "" : " <-> ", i);
        if (escrito < 0 ||(size_t) escrito >= sizeof(nos) - usado) {
            log_torre(agora, "CIRCUITO: N1 ate N%d", a->quantidade_taxiways);
            return;
        }
        usado += (size_t) escrito;
    }
    log_torre(agora, "CIRCUITO TAXIWAY: %s", nos);
    log_torre(agora, "ENTRADAS: horario=N1; anti-horario=N%d", a->quantidade_taxiways);
    log_torre(agora, "POLITICA: pouso exige pista, entrada, sentido e reserva de portao disponiveis");
}
/* Aloca a operacao, cria voo/carga e entra na fila de pouso. */
static void gerar(FilaPrioridade *fila, Ativos *ativos, unsigned long *sequencia, time_t agora) {
    Operacao *op;
    if (* sequencia >= MAX_GERADOS || fila->tamanho == fila->capacidade || ativos->quantidade == MAX_ATIVOS) {
        return;
    }
    op = calloc(1, sizeof(*op));
    if (!op) {
        return;
    }
    op->aviao = malloc(sizeof(* op->aviao));
    if (!op->aviao) {
        free(op);
        return;
    }
    * op->aviao = gerar_aviao(++ *sequencia);
    preparar_carga(op, agora);
    if (!inserir_aviao(fila, op->aviao)) {
        free(op->aviao);
        free(op);
        return;
    }
    ativos->itens[ativos->quantidade++] = op;
    log_torre(agora, "AERONAVE GERADA: %s | tipo=%s | combustivel=%d%% | emergencia medica=%s | prioridade=%d (%s) | sentido=%s | fila=%d", op->aviao->identificacao, nome_tipo_voo(op->aviao->tipo), op->aviao->combustivel, op->aviao->emergencia_medica ? "SIM" : "NAO", op->aviao->prioridade, descricao_prioridade(op->aviao->prioridade), op->aviao->sentido == HORARIO ? "N1->Nn" : "Nn->N1", fila->tamanho);
}
/* Consulta o heap e autoriza pouso apenas se o solo comportar o voo. */
static void iniciar_pousos(Aeroporto *a, FilaPrioridade *fila, time_t agora) {
    Aviao *v;
    int pista;
    while ((v = consultar_proximo_aviao(fila)) != NULL) {
        if (!autorizar_pouso(a, v, &pista)) {
            /* Nao ultrapassa o primeiro da fila de prioridade. */
            break;
        }
        if (!ocupar_pista(a, pista, v, agora + TEMPO_POUSO)) {
            break;
        }
        (void) remover_proximo_aviao(fila);
        a->reservas_portao++;
        v->estado = POUSANDO;
        v->pista = pista;
        v->proximo_evento = agora + TEMPO_POUSO;
        log_torre(agora, "POUSO autorizado %s na pista %d; portao futuro reservado (%d)", v->identificacao, pista, a->reservas_portao);
    }
}
/* Move da pista para a entrada N1 ou Nn, ou aguarda na pista. */
static void concluir_pousos(Aeroporto *a, Ativos *ativos, time_t agora) {
    int i, entrada;
    Aviao *v;
    for (i = 0; i < ativos->quantidade; i++) {
        v = ativos->itens[i]->aviao;
        if (v->estado != POUSANDO || v->proximo_evento > agora) {
            continue;
        }
        entrada = v->sentido == HORARIO ? a->inicio->id : a->fim->id;
        if (!entrada_taxiway_livre(a, v) || !ocupar_no(a, entrada, v)) {
            /* Conserva a pista ocupada ate conseguir sair dela. */
            a->pistas[v->pista - 1].liberacao = agora + 1;
            log_torre(agora, "SOLO: %s aguarda entrada N%d; pista %d permanece ocupada", v->identificacao, entrada, v->pista);
            v->proximo_evento = agora + 1;
            continue;
        }
        v->estado = TAXIANDO;
        v->posicao_taxiway = entrada;
        v->proximo_evento = agora + TEMPO_TAXI;
        log_torre(agora, "%s pousou e entrou na taxiway N%d", v->identificacao, entrada);
    }
}
/* Avanca entre nos adjacentes e estaciona quando alcanca a saida. */
static void mover_taxiways(Aeroporto *a, Ativos *ativos, time_t agora) {
    int i, destino, portao;
    Aviao *v;
    /* Percorrer na ordem de chegada ao circuito, do mais antigo ao mais novo. */
    for (i = 0; i < ativos->quantidade; i++) {
        v = ativos->itens[i]->aviao;
        if ((v->estado != TAXIANDO && v->estado != AGUARDANDO_PORTAO) || v->proximo_evento > agora) {
            continue;
        }
        destino = v->posicao_taxiway +(v->sentido == HORARIO ? 1 : -1);
        if (destino < 1 || destino > a->quantidade_taxiways) {
            portao = obter_portao_livre(a);
            if (portao < 0) {
                if (v->estado != AGUARDANDO_PORTAO) {
                    log_torre(agora, "%s chegou ao ultimo no e aguarda portao", v->identificacao);
                }
                v->estado = AGUARDANDO_PORTAO;
                v->proximo_evento = agora + 1;
                continue;
            }
            if (!ocupar_portao(a, portao, v)) {
                continue;
            }
            if (!liberar_no(a, v->posicao_taxiway, v)) {
                (void) liberar_portao(a, portao, v);
                continue;
            }
            a->reservas_portao--;
            v->posicao_taxiway = -1;
            v->portao = portao;
            v->estado = NO_PORTAO;
            v->proximo_evento = agora + TEMPO_DESCARGA;
            log_torre(agora, "ESTACIONAMENTO: %s no portao %d; descarregamento iniciado", v->identificacao, portao);
            continue;
        }
        if (!mover_no(a, v->posicao_taxiway, destino, v)) {
            v->proximo_evento = agora + 1;
            continue;
        }
        log_torre(agora, "TAXIWAY: %s N%d -> N%d", v->identificacao, v->posicao_taxiway, destino);
        v->posicao_taxiway = destino;
        v->proximo_evento = agora + TEMPO_TAXI;
    }
}
/* Descarrega um item por evento e aguarda o tempo de solo. */
static void processar_portoes(Aeroporto *a, Ativos *ativos, FilaSaida *saida, time_t agora) {
    int i, resultado;
    Item item;
    Operacao *op;
    Aviao *v;
    for (i = 0; i < ativos->quantidade; i++) {
        op = ativos->itens[i];
        v = op->aviao;
        if (v->estado != NO_PORTAO || v->proximo_evento > agora) {
            continue;
        }
        if (op->porao.quantidade) {
            resultado = descarregar_item(&op->manifesto,&op->porao, &item);
            if (resultado == 1) {
                log_despacho(agora, "DOUBLE-CHECK OK %s: %s, dono %s, %.2f kg", v->identificacao, item.codigo, item.dono, item.peso);
            }
            else if (resultado == -1) log_despacho(agora, "ALERTA INCONFORMIDADE %s: item %s isolado para verificacao", v->identificacao, item.codigo);
            v->proximo_evento = agora + TEMPO_DESCARGA;
            continue;
        }
        if (v->proximo_evento == 0) {
            continue;
        }
        /* O tempo de solo inicia quando termina o descarregamento. */
        v->estado = AGUARDANDO_DECOLAGEM;
        v->proximo_evento = agora + TEMPO_SOLO;
        log_despacho(agora, "%s descarregamento concluido; tempo de solo ate %ld", v->identificacao,(long) v->proximo_evento);
    }
    /* O portao permanece ocupado ate o inicio efetivo da decolagem. */
    (void) a;
    (void) saida;
}
/* Insere uma unica vez o voo pronto na fila FIFO de saida. */
static void enfileirar_saidas(Ativos *ativos, FilaSaida *saida, time_t agora) {
    int i;
    Aviao *v;
    for (i = 0; i < ativos->quantidade; i++) {
        v = ativos->itens[i]->aviao;
        if (v->estado == AGUARDANDO_DECOLAGEM && v->proximo_evento <= agora) {
            if (!saida_inserir(saida, v)) {
                continue;
            }
            v->estado = FILA_DECOLAGEM;
            v->proximo_evento = 0;
            log_torre(agora, "SAIDA: %s entrou na fila de decolagem", v->identificacao);
        }
    }
}
/* Ocupa pista, libera portao e inicia decolagem. */
static void iniciar_decolagens(Aeroporto *a, FilaSaida *saida, time_t agora) {
    Aviao *v;
    int pista;
    while ((v = saida_primeiro(saida)) != NULL) {
        pista = obter_pista_livre(a);
        if (pista < 0) {
            break;
        }
        if (!ocupar_pista(a, pista, v, agora + TEMPO_DECOLAGEM)) {
            break;
        }
        saida_remover(saida);
        if (!liberar_portao(a, v->portao, v)) {
            log_torre(agora, "ERRO: portao inconsistente para %s", v->identificacao);
            /* A aeronave nao e descartada; pista permanece ocupada ate liberacao. */
            break;
        }
        v->portao = -1;
        v->pista = pista;
        v->estado = DECOLANDO;
        v->proximo_evento = agora + TEMPO_DECOLAGEM;
        log_torre(agora, "DECOLAGEM: %s iniciou na pista %d; portao liberado", v->identificacao, pista);
    }
}
/* Marca voos com decolagem finalizada. */
static void concluir_decolagens(Ativos *ativos, time_t agora) {
    int i;
    Aviao *v;
    for (i = 0; i < ativos->quantidade; i++) {
        v = ativos->itens[i]->aviao;
        if (v->estado == DECOLANDO && v->proximo_evento <= agora) {
            v->estado = CONCLUIDO;
            log_torre(agora, "CONCLUIDO: %s decolou", v->identificacao);
        }
    }
}
/* Libera memoria das operacoes concluidas e compacta o vetor. */
static void limpar(Ativos *ativos) {
    int i = 0;
    while (i < ativos->quantidade) {
        Operacao *op = ativos->itens[i];
        if (op->aviao->estado != CONCLUIDO) {
            i++;
            continue;
        }
        free(op->aviao);
        free(op);
        memmove( &ativos->itens[i], &ativos->itens[i + 1],(size_t)(ativos->quantidade - i -1) * sizeof(* ativos->itens));
        ativos->quantidade--;
    }
}
/* Seleciona aeroporto, executa eventos em ordem e libera recursos. */
int main(int argc, char **argv) {
    const char *configs[] = {
        "aeroportos/galeao.cfg", "aeroportos/confins.cfg", "aeroportos/guarulhos.cfg", "aeroportos/brasilia.cfg"
    };
    Aeroporto aeroporto;
    FilaPrioridade fila;
    Ativos ativos = {
        0
    };
    FilaSaida saida = {
        0
    };
    unsigned long sequencia = 0;
    int escolha, ticks = 0, passo = 0, rapido = 0, i;
    time_t inicio, agora;
    if (argc > 3) {
        fprintf(stderr, "Uso: %s [aeroporto 1-4] [segundos simulados]\n", argv[0]);
        return 1;
    }
    if (argc > 1) {
        if (!argumento_inteiro(argv[1], 1, 4, &escolha)) {
            fprintf(stderr, "Aeroporto invalido: 1 Galeao, 2 Confins, 3 Guarulhos, 4 Brasilia\n");
            return 1;
        }
        printf("Aeroporto selecionado por argumento: %d\n", escolha);
    } else {
        escolha = escolher_aeroporto();
        if (!escolha) {
            fprintf(stderr, "Selecao cancelada.\n");
            return 1;
        }
    }
    if (argc > 2) {
        if (!argumento_inteiro(argv[2], 1, 1000000, &ticks)) {
            fprintf(stderr, "Segundos simulados devem ser um inteiro entre 1 e 1000000\n");
            return 1;
        }
        rapido = 1;
    }
    srand((unsigned) time(NULL));
    signal(SIGINT, interromper);
    inicializar_aeroporto(&aeroporto);
    if (!carregar_configuracao(&aeroporto, configs[escolha - 1]) || !criar_circuito(&aeroporto) || !inicializar_fila(&fila, MAX_ATIVOS)) {
        fprintf(stderr, "Erro ao inicializar aeroporto/fila. Execute na pasta do projeto.\n");
        destruir_aeroporto(&aeroporto);
        return 1;
    }
    arquivo_log = fopen("simulador.log", "w");
    arquivo_torre = fopen("torre_controle.log", "w");
    arquivo_despacho = fopen("despacho_porao.log", "w");
    if (!arquivo_log || !arquivo_torre || !arquivo_despacho) {
        fprintf(stderr, "Erro ao abrir os arquivos de log.\n");
        if (arquivo_log) {
            fclose(arquivo_log);
        }
        if (arquivo_torre) {
            fclose(arquivo_torre);
        }
        if (arquivo_despacho) {
            fclose(arquivo_despacho);
        }
        destruir_fila(&fila);
        destruir_aeroporto(&aeroporto);
        return 1;
    }
    inicio = time(NULL);
    apresentar_aeroporto(&aeroporto, inicio);
    while (executar &&(!rapido || passo < ticks)) {
        agora = rapido ? inicio + passo : time(NULL);
        if (passo % INTERVALO_GERACAO == 0) {
            gerar(&fila, &ativos, &sequencia, agora);
        }
        /* Liberar pistas primeiro; depois concluir pousos e movimentar solo. */
        concluir_pousos(&aeroporto, &ativos, agora);
        mover_taxiways(&aeroporto, &ativos, agora);
        processar_portoes(&aeroporto, &ativos, &saida, agora);
        enfileirar_saidas( &ativos, &saida, agora);
        iniciar_decolagens(&aeroporto, &saida, agora);
        concluir_decolagens( &ativos, agora);
        atualizar_pistas(&aeroporto, agora);
        limpar( &ativos);
        iniciar_pousos(&aeroporto,&fila, agora);
        if (sequencia == MAX_GERADOS && fila_vazia(&fila) && ativos.quantidade == 0) {
            log_sistema(agora, "SIMULACAO FINALIZADA");
            break;
        }
        passo++;
        if (!rapido) {
            sleep(1);
        }
    }
    agora = rapido ? inicio + passo : time(NULL);
    log_sistema(agora, "RESUMO: gerados=%lu, fila_pouso=%d, fila_saida=%d, ativos=%d, reservas_portao=%d", sequencia, fila.tamanho, saida.quantidade, ativos.quantidade, aeroporto.reservas_portao);
    for (i = 0; i < ativos.quantidade; i++) {
        free(ativos.itens[i]->aviao);
        free(ativos.itens[i]);
    }
    destruir_fila(&fila);
    destruir_aeroporto(&aeroporto);
    if (arquivo_log) {
        fclose(arquivo_log);
    }
    if (arquivo_torre) {
        fclose(arquivo_torre);
    }
    if (arquivo_despacho) {
        fclose(arquivo_despacho);
    }
    return 0;
}
