#ifndef CONTROLE_AEREO_H
#define CONTROLE_AEREO_H
#include <time.h>

// Categoria do voo, usada para calcular a prioridade de pouso.
typedef enum {
    VOO_COMERCIAL, VOO_CARGA, VOO_PRIVADO
} TipoVoo;
// Sentido permitido na taxiway compartilhada.
typedef enum {
    HORARIO, ANTI_HORARIO
} Sentido;
// Etapas percorridas por cada aeronave durante a simulacao.
typedef enum {
    AGUARDANDO_POUSO, POUSANDO, TAXIANDO, AGUARDANDO_PORTAO, NO_PORTAO, AGUARDANDO_DECOLAGEM, FILA_DECOLAGEM, DECOLANDO, CONCLUIDO
} EstadoAviao;

/*
 * Aviao: dados e estado operacional de uma aeronave.
 * Identificacao e ordem_chegada permitem identifica-la e desempatar prioridades.
 * proximo_evento marca quando sua proxima etapa pode acontecer.
 */
typedef struct {
    char identificacao[20];
    // Codigo unico do voo.
    int combustivel;
    // Percentual de combustivel restante.
    int emergencia_medica;
    // 1 se houver emergencia medica; 0 caso contrario.
    int prioridade;
    // 1 emergencia, 2 comercial, 3 carga/particular.
    TipoVoo tipo;
    // Categoria do voo.
    unsigned long ordem_chegada;
    // Desempate FIFO entre prioridades iguais.
    Sentido sentido;
    // Direcao de circulacao no solo.
    EstadoAviao estado;
    // Etapa atual do ciclo operacional.
    int posicao_taxiway;
    // No atual; -1 quando fora da taxiway.
    int portao;
    // Portao atual; -1 quando nao estacionado.
    int pista;
    // Pista atribuida; -1 quando nao atribuida.
    time_t proximo_evento;
    // Instante simulado da proxima acao.
} Aviao;

/*
 * FilaPrioridade: heap minimo de ponteiros para aeronaves.
 * dados guarda os enderecos, tamanho indica os elementos presentes
 * e capacidade define o limite alocado.
 */
typedef struct {
    Aviao **dados;
    // Vetor de ponteiros, sem transferir posse dos avioes.
    int tamanho;
    // Quantidade atual na fila.
    int capacidade;
    // Limite de aeronaves na fila.
} FilaPrioridade;

// Inicializa o heap e aloca seu vetor.
int inicializar_fila(FilaPrioridade *f, int capacidade);
// Calcula a prioridade e insere a aeronave no heap.
int inserir_aviao(FilaPrioridade *f, Aviao *a);
// Consulta a primeira aeronave sem remove-la.
Aviao *consultar_proximo_aviao(const FilaPrioridade *f);
// Retira a aeronave mais prioritaria e reorganiza o heap.
Aviao *remover_proximo_aviao(FilaPrioridade *f);
// Informa se nao ha aeronaves aguardando pouso.
int fila_vazia(const FilaPrioridade *f);
// Define prioridade a partir de combustivel, emergencia e tipo.
int calcular_prioridade(Aviao *a);
// Cria os dados aleatorios de uma aeronave ficticia.
Aviao gerar_aviao(unsigned long ordem);
// Libera apenas o vetor do heap; o simulador libera os avioes.
void destruir_fila(FilaPrioridade *f);
#endif
