Etapas de Avaliação
A avaliação do Controlador Aéreo Nacional possui duração mı́nima de 1 semana (7 dias) e é composta por
três etapas complementares de desenvolvimento e documentação.

A.**Etapa 1: Modelagem Arquitetural e Configuração Inicial** 

- Compreensão dos requisitos operacionais e especificação dos tipos de dados.

- **Decisão e justificativa** das estruturas de dados mais adequadas para a Autorização de Pouso, a
Circulação de Solo e o Despacho/Cadastro de Porão.

- **Implementação** da estrutura de *configuração inicial* baseado na infraestrutura do aeroporto.

- Configurações complementares não precisam ser implementadas, mas devem ser citadas.
----------------------------------------------------------------

B. **Etapa 2: Implementação dos Componentes e Regras de Negócio** 

- Desenvolvimentos das rotinas de aproximação, navegação no circuito de taxiways.
Caso tenha interesse, implemente algum tipo de temporizador para melhor entender ou simular o
processo fı́sico que acontece com a aeronave.

- Implementação do cadastro prévio, validação por double-check no descarregamento do porão e rotina
das bagagens.

- Testes de integridade de ponteiros e tratamento de casos especı́ficos, estruturas vazias ou busca de
itens inexistentes.

- Serão aceitos, no máximo, 1 código em C por componente — i) tráfego aéreo, ii) manobra em pista
de táxi e iii) despacho de bagagens, — corretamente identificados e comentados.

C. **Etapa 3: Análise de Memória e Confecção do Relatório As-Built** 

- Redação do Relatório As-Built documentando a arquitetura como ela foi efetivamente construı́da,
incluindo possı́veis diagramas dos nós/ponteiros.

- Submissão final do código-fonte modularizado e do relatório em formato PDF.

**Estrutura Obrigatória do Relatório As-Built** 

O relatório As-Built deve ser entregue em formato PDF e documentar de forma transparente o sistema como
ele foi efetivamente projetado, construı́do e testado. O relatório deve conter estritamente as seguintes seções:

A. **Descrição da Arquitetura:**  Para cada um dos três componentes do sistema, o aluno deve apresentar a
estrutura de dados escolhida e defender tecnicamente a sua decisão, abordando obrigatoriamente:

- **Mapeamento do Problema:** Justificativa de por que o tipo de encadeamento/organização escolhido (ex: encadeamento simples, duplo, circular, por vetor, etc.) atende e representa a melhor
opção baseado nos requisitos operacionais.

- **Dados Básicos de cada Estrutura:** Quais as informações básicas alocadas em cada uma das
estruturas ou vetores de dados.

- **Benefı́cios e Vantagens Práticas:** Benefı́cios obtidos pela implementação, como facilidade de
navegação, economias de memória ou operação, são obtidos baseado na estrutura apresentada.

- **Tratamento Especı́fico do Domı́nio:** Como a arquitetura foi desenhada para tratar peculiaridades
do problema, baseado nas polı́ticas operacionais de cada parte do sistema.
---------------------------------------------

B. **Diagramas de Memória e Representação dos Nós:** Desenho/diagrama esquemático ilustrando o
estado da memória durante a execução do programa, destacando explicitamente os nós, seus campos de
dados e os ponteiros de controle.
Em resumo “Uma imagem vale mais que mil palavras” ou “Se não entender... Eu desenho”.
