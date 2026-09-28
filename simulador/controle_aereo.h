#ifndef CONTROLE_AEREO_H
#define CONTROLE_AEREO_H

#include <time.h>

typedef enum {
    VOO_COMERCIAL,
    VOO_CARGA,
    VOO_PRIVADO
} TipoVoo;

typedef enum {
    HORARIO,
    ANTI_HORARIO
} Sentido;

typedef enum {
    APROXIMANDO,
    AGUARDANDO_POUSO,
    POUSANDO,
    AGUARDANDO_TAXIWAY,
    TAXIANDO,
    NO_PORTAO,
    AGUARDANDO_DECOLAGEM,
    DECOLANDO,
    CONCLUIDO
} EstadoAviao;

typedef struct {
    char identificacao[20];

    int combustivel;
    int emergencia_medica;

    TipoVoo tipo;
    int prioridade;

    unsigned long ordem_chegada;

    Sentido sentido;
    EstadoAviao estado;

    int posicao_taxiway;
    int portao;
    int pista;

    time_t chegada;
    time_t inicio_pouso;
    time_t fim_pouso;

    time_t entrada_taxiway;
    time_t chegada_portao;
    time_t saida_portao;

    time_t inicio_decolagem;
    time_t fim_decolagem;

    time_t proxima_movimentacao;

    int alerta_conflito_registrado;

} Aviao;

typedef struct {
    Aviao **dados;
    int tamanho;
    int capacidade;
} FilaPrioridade;

void inicializar_fila(FilaPrioridade *fila, int capacidade);

int inserir_aviao(
    FilaPrioridade *fila,
    Aviao *aviao
);

Aviao *consultar_proximo_aviao(
    const FilaPrioridade *fila
);

Aviao *remover_proximo_aviao(
    FilaPrioridade *fila
);

int fila_vazia(
    const FilaPrioridade *fila
);

int calcular_prioridade(
    Aviao *aviao
);

Aviao gerar_aviao(
    unsigned long ordem
);

void destruir_fila(
    FilaPrioridade *fila
);

#endif
