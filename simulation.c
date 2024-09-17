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
    u = 1.0 - u;
    return u;
}

double gera_tempo(double l){
    return (-1.0/l) * log(uniforme());
}

double min(double n1, double n2){
    if (n1 < n2) return n1;
    return n2;
}

void inicia_little(little *n){
    n->num_eventos = 0;
    n->soma_areas = 0.0;
    n->tempo_anterior = 0.0;
}

void atualiza_little(little *n, double tempo_atual) {
    n->soma_areas += (tempo_atual - n->tempo_anterior) * n->num_eventos;
    n->tempo_anterior = tempo_atual;
}

void simula_cenario(double taxa_chegada, double taxa_saida, double tempo_simulacao, const char* nome_arquivo){
    double tempo_decorrido = 0.0;
    double tempo_chegada = gera_tempo(taxa_chegada);
    double tempo_saida = DBL_MAX;

    unsigned long int fila = 0;
    unsigned long int fila_max = 0;

    double soma_ocupacao = 0.0;

    little en; // Número médio de clientes no sistema (E[N])
    little ew_chegadas; // Tempo médio no sistema para chegadas (E[W] para chegadas)
    little ew_saidas; // Tempo médio no sistema para saídas (E[W] para saídas)
    inicia_little(&en);
    inicia_little(&ew_chegadas);
    inicia_little(&ew_saidas);

    int tempo_coleta = 100;

    FILE *arquivo_csv = fopen(nome_arquivo, "w");
    if (arquivo_csv == NULL) {
        fprintf(stderr, "Não foi possível abrir o arquivo %s para escrita.\n", nome_arquivo);
        return;
    }

    // Cabeçalho do CSV
    fprintf(arquivo_csv, "Tempo Decorrido,Taxa Chegada,Taxa Saída,E[N],E[W],Erro de Little,Maior Tamanho de Fila,Ocupacao\n");

    while(tempo_decorrido <= tempo_simulacao){
        tempo_decorrido = min(tempo_coleta, min(tempo_chegada, tempo_saida));

        if(tempo_decorrido == tempo_coleta) {
            atualiza_little(&en, tempo_decorrido);
            atualiza_little(&ew_chegadas, tempo_decorrido);
            atualiza_little(&ew_saidas, tempo_decorrido);

            double en_final = en.soma_areas / tempo_decorrido; // Número médio de clientes no sistema (E[N])
            double ew_final = (ew_chegadas.soma_areas - ew_saidas.soma_areas) / ew_chegadas.num_eventos; // Tempo médio no sistema (E[W])
            double lambda = ew_chegadas.num_eventos / tempo_decorrido; // Taxa de chegada (λ)
            double erro_little = en_final - lambda * ew_final; // Erro de Little: E[N] - λE[W]

            // Atualiza a ocupação
            double ocupacao = soma_ocupacao / tempo_decorrido;

            // Corrigido: Adiciona o maior tamanho da fila e a ocupação ao CSV
            fprintf(arquivo_csv, "%lf,%.2f,%.2f,%lf,%lf,%lf,%lu,%lf\n", tempo_decorrido, taxa_chegada, taxa_saida, en_final, ew_final, erro_little, fila_max, ocupacao);

            tempo_coleta += 100;
        } else if(tempo_decorrido == tempo_chegada) {
            // sistema está ocioso?
            if(!fila){
                tempo_saida = tempo_decorrido + gera_tempo(taxa_saida);
                soma_ocupacao += tempo_saida - tempo_decorrido;
            }
            fila++;
            fila_max = fila > fila_max ? fila : fila_max;
            tempo_chegada = tempo_decorrido + gera_tempo(taxa_chegada);

            // Atualiza Little
            atualiza_little(&en, tempo_decorrido);
            en.num_eventos++; // Incrementa o número de clientes no sistema

            atualiza_little(&ew_chegadas, tempo_decorrido);
            ew_chegadas.num_eventos++; // Incrementa o número de chegadas

        } else {
            fila--;
            tempo_saida = DBL_MAX;
            if(fila){
                tempo_saida = tempo_decorrido + gera_tempo(taxa_saida);
                soma_ocupacao += tempo_saida - tempo_decorrido;
            }

            // Atualiza Little
            atualiza_little(&en, tempo_decorrido);
            en.num_eventos--; // Decrementa o número de clientes no sistema

            atualiza_little(&ew_saidas, tempo_decorrido);
            ew_saidas.num_eventos++; // Incrementa o número de saídas
        }
    }

    // Última atualização de Little
    atualiza_little(&ew_chegadas, tempo_decorrido);
    atualiza_little(&ew_saidas, tempo_decorrido);

    double en_final = en.soma_areas / tempo_decorrido;
    double ew_final = (ew_chegadas.soma_areas - ew_saidas.soma_areas) / ew_chegadas.num_eventos;
    double lambda = ew_chegadas.num_eventos / tempo_decorrido;
    double erro_little = en_final - lambda * ew_final;

    // Adiciona a última linha com a ocupação e o maior tamanho da fila
    fprintf(arquivo_csv, "%lf,%.2f,%.2f,%lf,%lf,%lf,%lu,%lf\n", tempo_decorrido, taxa_chegada, taxa_saida, en_final, ew_final, erro_little, fila_max, soma_ocupacao / tempo_decorrido);

    fclose(arquivo_csv);
}

int main(){
    srand(time(NULL));
    double tempos[4] = {0.80, 0.90, 0.95, 0.99}; 
    double tempo_simulacao = 100000.0; 

    for(int i = 0; i < 4; i++) {
        double ocupacao = tempos[i];
        double taxa_chegada = 1.0; // Taxa de chegada arbitrária
        double taxa_saida = taxa_chegada / ocupacao; // Corrigido: A taxa de saída é proporcional à taxa de chegada

        // Nome do arquivo baseado na taxa de chegada e taxa de saída
        char nome_arquivo[50];
        snprintf(nome_arquivo, sizeof(nome_arquivo), "resultado_cenario_%.2f.csv", ocupacao);

        simula_cenario(taxa_chegada, taxa_saida, tempo_simulacao, nome_arquivo);
    }

    return 0;
}
