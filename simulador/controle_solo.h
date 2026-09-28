#ifndef CONTROLE_SOLO_H
#define CONTROLE_SOLO_H
#include "controle_aereo.h"

// No da lista duplamente encadeada que representa uma posicao de taxiway.
typedef struct No {
    int id;
    // Numero da posicao N1, N2, ...
    Aviao *aviao;
    // Aeronave ocupante ou NULL quando livre.
    struct No *anterior;
    // Posicao anterior na lista.
    struct No *proximo;
    // Posicao seguinte na lista.
} No;
// Portao e pista apontam para a aeronave que os ocupa.
typedef struct {
    int id;
    Aviao *aviao;
} Portao;
typedef struct {
    int id;
    // Identificador da pista.
    Aviao *aviao;
    // Ocupante atual ou NULL.
    time_t liberacao;
    // Instante previsto para liberar a pista.
} Pista;

/*
 * Aeroporto: infraestrutura e controle de ocupacao do solo.
 * A lista liga os nos da taxiway; portoes e pistas sao vetores.
 * reservas_portao impede autorizar pousos acima da capacidade futura.
 */
typedef struct {
    char nome[50];
    // Nome lido do arquivo de configuracao.
    int quantidade_pistas;
    // Pistas disponiveis.
    int quantidade_taxiways;
    // Nos do circuito simplificado.
    int quantidade_portoes;
    // Portoes disponiveis.
    No *inicio, *fim;
    // Extremos da lista duplamente encadeada.
    Portao *portoes;
    // Vetor de portoes.
    Pista *pistas;
    // Vetor de pistas.
    int sentido_ativo;
    // -1 vazio; senao HORARIO ou ANTI_HORARIO.
    int reservas_portao;
    // Avioes aceitos que ainda nao estacionaram.
} Aeroporto;

// Zera os campos e libera o sentido de circulacao inicial.
void inicializar_aeroporto(Aeroporto *a);
// Le nome e quantidades a partir de um arquivo .cfg.
int carregar_configuracao(Aeroporto *a, const char *arquivo);
// Aloca pistas, portoes e lista duplamente encadeada.
int criar_circuito(Aeroporto *a);
// Busca um no pelo numero.
No *obter_no(Aeroporto *a, int id);
// Confere a entrada N1/Nn e o sentido ativo.
int entrada_taxiway_livre(Aeroporto *a, const Aviao *aviao);
// Confere se todos os portoes livres ja estao comprometidos.
int solo_completamente_congestionado(const Aeroporto *a);
// Autoriza somente com pista, entrada, sentido e portao futuro.
int autorizar_pouso(Aeroporto *a, const Aviao *aviao, int *pista);
// Coloca aeronave em um no livre.
int ocupar_no(Aeroporto *a, int id, Aviao *aviao);
// Move apenas entre nos adjacentes, livres e no sentido correto.
int mover_no(Aeroporto *a, int origem, int destino, Aviao *aviao);
// Retira aeronave do no e atualiza o sentido do circuito.
int liberar_no(Aeroporto *a, int id, Aviao *aviao);
// Retorna um portao livre ou -1.
int obter_portao_livre(const Aeroporto *a);
// Reserva fisicamente um portao para a aeronave.
int ocupar_portao(Aeroporto *a, int id, Aviao *aviao);
// Libera o portao ocupado pela aeronave informada.
int liberar_portao(Aeroporto *a, int id, Aviao *aviao);
// Retorna uma pista livre ou -1.
int obter_pista_livre(const Aeroporto *a);
// Ocupa uma pista ate o instante informado.
int ocupar_pista(Aeroporto *a, int id, Aviao *aviao, time_t ate);
// Libera pistas cujo prazo de ocupacao terminou.
void atualizar_pistas(Aeroporto *a, time_t agora);
// Libera lista e vetores alocados para o aeroporto.
void destruir_aeroporto(Aeroporto *a);
#endif
