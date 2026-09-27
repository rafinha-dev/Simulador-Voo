                FILA DE PRIORIDADE
                       │
        ┌──────────────┼──────────────┐
        ▼              ▼              ▼
    Prioridade 1   Prioridade 2   Prioridade 3
    Emergência      Comercial      Carga/Particular
        │              │              │
       A01            A05            A03
       A07            A02            A08

Se chegar: 

A01 - Comercial
A02 - Particular
A03 - Emergência
A04 - Comercial

a ordem de pouso será: 
A03 → A01 → A04 → A02

esses elementos não precisam estar simplesmente ordenados. Você pode organizá-los como um heap:

             A01
           /     \
         A05     A03
        /   \
      A07   A02

Se fosse utilizar  a lista ordenada por prioridade, inserir um novo avião pode exigir percorrer a lista. 
Enquanto um heap baseado em array consegue inserir em `O(log n)`. 


                AERONAVES
                    │
                    ▼
              struct Aviao
                    │
                    ▼
         array dinâmico de Aviao
                    │
                    ▼
              Binary Heap
                    │
                    ▼
          Fila de Prioridade

Com um heap, é mantido a propriedade da fila incrementalmente:

Inserir avião       → O(log n)
Ver próximo avião   → O(1)
Remover próximo     → O(log n)


----
Importante: calcular a prioridade e organizar a fila são duas responsabilidades diferentes. A função calcular_prioridade() define a classificação; o heap mantém a ordem de atendimento. O sistema pode fazer as duas operações automaticamente, sem intervenção do usuário.

Uma questão que vale definir no projeto: se uma aeronave já está aguardando pouso e o combustível dela cai abaixo de 10%, o sistema deverá recalcular a prioridade e reorganizar o heap.
----


Primeira parte 
-- 

Revisão do problema: 
Um arquivo é ótimo para salvar dados do aeroporto, e para o sistema identificar que ele é um aeroporto ou outro esses dados podem ficar salvos em um arquivos. 
Uma struct pode descrever cada tipo de avião. E se será tratado como fila a primeira parte irá ter uma  E para imitar um funcionamento um gerador de números aleatórios em um tempo aléatório para simuar a chegada de aviões no aeroporto. 
O sistema precisa ser dinâmico e liberar as pistas de taxyways. Pedir autorização de pouso...




Informações complementares podem ser documentadas mas não precisam ser programadas, de forma que não interfira no funcionamento, além disso cada etapa vai suportar apenas um código C. 

