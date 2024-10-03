#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   // para a função sleep()
#include <pthread.h>  // para a criação de threads (verificar a emergência enquanto o portao está operando)
#include <fcntl.h>
#include <string.h>
#include <sys/select.h>

// Definição dos estados da máquina
typedef enum {
    FECHADA,
    ABRINDO,
    ABERTA,
    FECHANDO,
    EMERGENCIA
} Estado;

// Variável global para verificar se a emergência foi acionada
int emergencia = 0;
Estado estado_anterior;  // Variável para armazenar o estado anterior à emergência

// Funções para a mudança de estados
void abrir_portao(Estado *estado);
void fechar_portao(Estado *estado);
void ciclo_emergencia(Estado *estado);
void verificar_emergencia(void *arg);

// Função para simular o portao com delay de 2 segundos entre ações
void esperar(int segundos, Estado *estado);

// Função para verificar se houve input do usuário
int input_disponivel();

// Função para printar o nome do estado
const char* nome_estado(Estado estado);

// Função principal que simula os estados do portao
int main() {
    
    printf("O portao inicia fechado, aperte 'A' para abrir.\n");

    Estado estado = FECHADA;  // Começamos com o portao fechado
    estado_anterior = estado;  // Inicializa o estado anterior
    
    pthread_t thread_emergencia;  // Thread para verificar a emergência
    pthread_create(&thread_emergencia, NULL, (void *) verificar_emergencia, NULL);

    while (1) {
        switch (estado) {
            case FECHADA:
                if (estado != estado_anterior) {
                    printf("portao está %s.\n", nome_estado(estado));
                    printf("Pressione 'A' para abrir.\n");
                    estado_anterior = estado;  // Atualiza o estado anterior
                }
                if (input_disponivel()) {
                    char botao = getchar();  // Usa getchar para ler o botão pressionado
                    if (botao == 'A') {
                        abrir_portao(&estado);
                    }
                }
                break;

            case ABRINDO:
                if (estado != estado_anterior) {
                    printf("portao está %s...\n", nome_estado(estado));
                    estado_anterior = estado;  // Atualiza o estado anterior
                }
                esperar(2, &estado);  // Simulação do tempo para abrir
                if (emergencia == 0) {  // Se não houve emergência, prossegue
                    estado = ABERTA;
                    printf("portao está completamente %s.\n", nome_estado(estado));
                }
                break;

            case ABERTA:
                if (estado != estado_anterior) {
                    printf("portao está %s.\n", nome_estado(estado));
                    printf("Pressione 'A' para fechar.\n");
                    estado_anterior = estado;  // Atualiza o estado anterior
                }
                if (input_disponivel()) {
                    char botao = getchar();  // Usa getchar para ler o botão pressionado
                    if (botao == 'A') {
                        fechar_portao(&estado);
                    }
                }
                break;

            case FECHANDO:
                if (estado != estado_anterior) {
                    printf("portao está %s...\n", nome_estado(estado));
                    estado_anterior = estado;  // Atualiza o estado anterior
                }
                // Loop para continuar verificando até o portao fechar completamente
                while (estado == FECHANDO) {
                    esperar(1, &estado);  // Simulação de fechamento por 1 segundo de cada vez
                    if (emergencia == 0) {  // Se não houve emergência, prossegue
                        printf("Continuando a fechar...\n");
                    } else {
                        break;  // Se houve emergência, interrompe o fechamento
                    }

                    // Simulação de portao completando o fechamento
                    if (rand() % 3 == 0) {  // Condição simulada para fechar completamente
                        estado = FECHADA;
                        printf("portao está completamente %s.\n", nome_estado(estado));
                    }
                }
                break;

            case EMERGENCIA:
                if (estado_anterior != EMERGENCIA) {  // Não salva EMERGENCIA como estado anterior
                    printf("portao em %s! Parando o portao.\n", nome_estado(estado));
                }
                ciclo_emergencia(&estado);  // Inicia o ciclo de emergência
                if (estado == EMERGENCIA) {
                    // A emergência foi resolvida, então restauramos o estado como ABRINDO
                    printf("Tempo de resolucao de emergencia finalizado. Iniciando abertura do portão.\n");
                    estado = ABRINDO;  // Sempre volta para ABRINDO após a emergência
                    emergencia = 0;  // Resetando a variável de emergência
                }
                break;

            default:
                printf("Estado desconhecido!\n");
        }
    }

    return 0;
}

// Funções de transição entre os estados
void abrir_portao(Estado *estado) {
    printf("Iniciando abertura do portao...\n");
    *estado = ABRINDO;
}

void fechar_portao(Estado *estado) {
    printf("Iniciando fechamento do portao...\n");
    *estado = FECHANDO;
}

void ciclo_emergencia(Estado *estado) {
    printf("Por favor, resolva a emergência...\n");
    sleep(5);  // Tempo para resolver emergência
    printf("Tempo de emergência finalizado\n");
    // A emergência foi resolvida, o estado será restaurado no loop principal
}

// Função de delay com verificação de emergência
void esperar(int segundos, Estado *estado) {
    for (int i = 0; i < segundos; i++) {
        sleep(1);  // Espera de 1 segundo por vez

        if (emergencia == 1) {  // Se a emergência foi acionada
            if (*estado != EMERGENCIA) {  // Não salva emergência como estado anterior
                estado_anterior = *estado;  // Salva o estado atual antes de mudar para EMERGENCIA
            }
            *estado = EMERGENCIA;
            break;
        }
    }
}

// Função para verificar a emergência (rodando em paralelo com o portao)
void verificar_emergencia(void *arg) {
    while (1) {
        if (input_disponivel()) {
            char botao = getchar();  // Lê o botão pressionado
            if (botao == 'E') {
                emergencia = 1;  // Se o botão 'E' foi pressionado, ativa a emergência
            }
        }
    }
}

// Função para verificar se há entrada disponível no stdin
int input_disponivel() {
    fd_set readfds;
    struct timeval tv;
    
    // Inicializa o conjunto de descritores
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    // Define o tempo de espera
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    // Verifica se há entrada disponível
    return select(1, &readfds, NULL, NULL, &tv) > 0;
}

// Função para retornar o nome do estado
const char* nome_estado(Estado estado) {
    switch (estado) {
        case FECHADA: return "FECHADA";
        case ABRINDO: return "ABRINDO";
        case ABERTA: return "ABERTA";
        case FECHANDO: return "FECHANDO";
        case EMERGENCIA: return "EMERGENCIA";
        default: return "DESCONHECIDO";
    }
}
