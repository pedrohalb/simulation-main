#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <float.h>

typedef struct {
    unsigned long int num_eventos;
    double tempo_anterior;
    double soma_areas;
} little;

double uniforme(){
    double u = rand() / ((double) RAND_MAX + 1);
    u = 1.0 - u; // Limitando u entre (0,1]
    return u;
}

double gera_tempo(double l){
    return (-1.0/l) * log(uniforme());
}

double min(double n1, double n2){
    return (n1 < n2) ? n1 : n2;
}

void inicia_little(little *n){
    n->num_eventos = 0;
    n->soma_areas = 0.0;
    n->tempo_anterior = 0.0;
}

int main(){
    srand(5);
    double parametro_chegada;
    printf("Informe o tempo médio entre as chegadas (s): ");
    scanf("%lF", &parametro_chegada);
    parametro_chegada = 1.0 / parametro_chegada;

    double parametro_saida;
    printf("Informe o tempo médio de atendimento (s): ");
    scanf("%lF", &parametro_saida);
    parametro_saida = 1.0 / parametro_saida;

    double tempo_simulacao = 100000.0; // Tempo de simulação de 100.000 segundos
    double intervalo_gravacao = 100.0; // Intervalo de gravação de 100 segundos
    int num_resultados = 1000; // Número de resultados

    FILE *arquivo = fopen("saida_simulacao.txt", "w+"); // Abrindo o arquivo em modo de leitura/escrita

    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return 1;
    }

    // Escreve uma linha temporária para o número de resultados
    fprintf(arquivo, "Número de resultados: %d\n\n", num_resultados);

    double tempo_decorrido = 0.0;
    double tempo_chegada = gera_tempo(parametro_chegada);
    double tempo_saida = DBL_MAX;
    double proxima_gravacao = intervalo_gravacao;

    unsigned long int fila = 0;
    unsigned long int fila_max = 0;
    double soma_ocupacao = 0.0;

    little en;
    little ew_chegadas;
    little ew_saidas;
    inicia_little(&en);
    inicia_little(&ew_chegadas);
    inicia_little(&ew_saidas);

    while (tempo_decorrido <= tempo_simulacao) {
        tempo_decorrido = min(tempo_chegada, tempo_saida);

        if (tempo_decorrido == tempo_chegada) {
            if (!fila) {
                tempo_saida = tempo_decorrido + gera_tempo(parametro_saida);
                soma_ocupacao += tempo_saida - tempo_decorrido;
            }
            fila++;
            fila_max = fila > fila_max ? fila : fila_max;

            tempo_chegada = tempo_decorrido + gera_tempo(parametro_chegada);

            en.soma_areas += (tempo_decorrido - en.tempo_anterior) * en.num_eventos;
            en.num_eventos++;
            en.tempo_anterior = tempo_decorrido;

            ew_chegadas.soma_areas += (tempo_decorrido - ew_chegadas.tempo_anterior) * ew_chegadas.num_eventos;
            ew_chegadas.num_eventos++;
            ew_chegadas.tempo_anterior = tempo_decorrido;
        } else {
            fila--;
            tempo_saida = DBL_MAX;

            if (fila) {
                tempo_saida = tempo_decorrido + gera_tempo(parametro_saida);
                soma_ocupacao += tempo_saida - tempo_decorrido;
            }

            en.soma_areas += (tempo_decorrido - en.tempo_anterior) * en.num_eventos;
            en.num_eventos--;
            en.tempo_anterior = tempo_decorrido;

            ew_saidas.soma_areas += (tempo_decorrido - ew_saidas.tempo_anterior) * ew_saidas.num_eventos;
            ew_saidas.num_eventos++;
            ew_saidas.tempo_anterior = tempo_decorrido;
        }

        // Gravar no arquivo a cada 100 segundos
        if (tempo_decorrido >= proxima_gravacao) {
            fprintf(arquivo, "Tempo: %.2f s\n", proxima_gravacao);
            fprintf(arquivo, "Maior tamanho de fila alcançado: %ld\n", fila_max);
            fprintf(arquivo, "Ocupação: %lF\n", soma_ocupacao / tempo_decorrido);
            double en_final = en.soma_areas / tempo_decorrido;
            double ew_final = (ew_chegadas.soma_areas - ew_saidas.soma_areas) / ew_chegadas.num_eventos;
            double lambda = ew_chegadas.num_eventos / tempo_decorrido;
            fprintf(arquivo, "E[N]: %lF\n", en_final);
            fprintf(arquivo, "E[W]: %lF\n", ew_final);
            fprintf(arquivo, "Erro de Little: %lF\n\n", en_final - lambda * ew_final);

            proxima_gravacao += intervalo_gravacao; // Atualizar para o próximo ponto de gravação
        }
    }

    // Voltar para o início do arquivo e sobrescrever o número de resultados
    rewind(arquivo);
    fprintf(arquivo, "Número de resultados: %d\n\n", num_resultados);

    fclose(arquivo); // Fechando o arquivo de saída

    return 0;
}
