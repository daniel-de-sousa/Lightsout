#include <stdio.h>
#include <stdlib.h>

#define COMANDO "/tmp/dir/software/planners/madagascar/M -S 1 -P 2 -Q -o Plain.txt > /dev/null lightsout.pddl lightsoutp.pddl"

void create_domain(FILE *arq){
    //Definindo o domínio
    fputs("(define\n", arq);
    fputs("\t(domain lightsout)\n", arq);

    //Definindo os requisitos
    fputs("\t(:requirements\n", arq);
    fputs("\t\t:strips\n", arq);
    fputs("\t\t:typing\n", arq);
    fputs("\t\t:negative-preconditions\n", arq);
    fputs("\t\t:conditional-effects\n", arq);
    fputs("\t)\n", arq);

    //Definindos os types
    fputs("\t(:types\n", arq);
    fputs("\t\tbutton\n", arq);
    fputs("\t)\n", arq);

    //Definindo os predicados
    fputs("\t(:predicates\n", arq);
    fputs("\t\t(con ?btn1 ?btn2 - button)\n", arq);
    fputs("\t\t(islit_on ?btn - button)\n", arq);
    fputs("\t\t(isbroken ?btn - button)\n", arq);
    fputs("\t)\n", arq);

    //Definindo a ação
    fputs("\t(:action click\n", arq);
    fputs("\t\t:parameters (?btn - button)\n", arq);
    fputs("\t\t:effect (and\n", arq);
    fputs("\t\t\t(forall (?btna - button) \n", arq);
    fputs("\t\t\t\t(when (con ?btn ?btna) (and\n", arq);
    fputs("\t\t\t\t\t\t(when (islit_on ?btna) (not(islit_on ?btna)))\n", arq);
    fputs("\t\t\t\t\t\t(when (not(islit_on ?btna)) (islit_on ?btna))\n", arq);
    fputs("\t\t\t\t\t)\n", arq);
    fputs("\t\t\t\t)\n", arq);
    fputs("\t\t\t)\n", arq);
    fputs("\t\t\t(when (and (not(isbroken ?btn)) (islit_on ?btn)) (not(islit_on ?btn)))\n", arq);
    fputs("\t\t\t(when (and (not(isbroken ?btn)) (not(islit_on ?btn))) (islit_on ?btn))\n", arq);
    fputs("\t\t)\n", arq);
    fputs("\t)\n", arq);
    fputs(")\n", arq);
}

void problem_init(FILE *arq){
    //Definindo o problema
    fputs("(define\n", arq);
    fputs("\t(problem lightsoutp)\n", arq);
    fputs("\t(:domain lightsout)\n", arq);

    //Definindo os objetos
    fputs("\t(:objects\n", arq);
}

void add_objects(FILE *arq, int quant){
    //Gerando os Botões
    fputs("\t\t", arq);
    for(int i = 0; i < quant; i++) {
        for(int j = 0; j < quant; j++) {
            fprintf(arq, "btn-%d-%d ", i, j);
        }
    }
    fputs("- button\n", arq);
    
    //Fechando os objects
    fputs("\t)\n", arq);
}

void add_init(FILE *arq, FILE *temp, int quant){
    fputs("\t(:init\n", arq);

    //Gerando as conexões
    for(int i = 0; i < quant; i++) {
        fputs("\t\t", arq);
        for(int j = 0; j < quant; j++) {
            if(j + 1 < quant) {
                fprintf(arq, "(con btn-%d-%d btn-%d-%d) ", i, j, i, j + 1);
                fprintf(arq, "(con btn-%d-%d btn-%d-%d) ", i, j + 1, i, j);
            }
            if(i + 1 < quant) {
                fprintf(arq, "(con btn-%d-%d btn-%d-%d) ", i, j, i + 1, j);
                fprintf(arq, "(con btn-%d-%d btn-%d-%d) ", i + 1, j, i, j);
            }
        }
        fputs("\n", arq);
    }

    fputs("\n", arq);

    //Definindo o estado inicial dos botṍes
    temp = fopen("Temporario.txt", "r");
    char estado[256];
    while(!feof(temp)){
        char *result = fgets(estado, sizeof(estado), temp);
        if(result == NULL){break;}
        fprintf(arq, "\t\t%s", estado);
    }
    fclose(temp);
    remove("Temporario.txt");
    fputs("\t)\n", arq);
}

void add_goal(FILE *arq){
    //Definindo o goal
    fputs("\t(:goal\n", arq);
    fputs("\t\t(forall (?btn - button)\n", arq);
    fputs("\t\t\t(not(islit_on ?btn))\n", arq);
    fputs("\t\t)\n", arq);
    fputs("\t)\n", arq);
    fputs(")\n", arq);
}

void read_plan(FILE *arq){
    char passo[256];
    char *click;
    int x, y;
    int flag = 0;

    while (!feof(arq)) {
        if(flag == 0){
            click = fgets(passo, sizeof(passo), arq);
            flag++;
        }
        
        sscanf(passo, "%*d %*c (click btn-%d-%d)", &x, &y);
        printf("(%d, %d)", x, y);

        click = fgets(passo, sizeof(passo), arq);
        if(click == NULL){break;}
        else{printf(";");}
    }

    printf("\n");
}

int main(){
    FILE *Domain = fopen("lightsout.pddl", "w");
    create_domain(Domain);
    fclose(Domain);

    FILE *Problem = fopen("lightsoutp.pddl", "w");
    problem_init(Problem);

    FILE *Temporario = fopen("Temporario.txt", "w");

    char light;
    int x = 0;
    int y = 0;

    while(scanf("%c", &light) != EOF){
        if(light == '\n'){
            x++;
            y = 0;
            continue;
        }
        if(light == 'd'){fprintf(Temporario, "(isbroken btn-%d-%d)\n", x, y);}
        if(light == 'L'){fprintf(Temporario, "(islit_on btn-%d-%d)\n", x, y);}
        if(light == 'l'){
            fprintf(Temporario, "(isbroken btn-%d-%d)\n", x, y);
            fprintf(Temporario, "(islit_on btn-%d-%d)\n", x, y);
        }
        y++;
    }
    fclose(Temporario);

    add_objects(Problem, x);
    add_init(Problem, Temporario, x);
    add_goal(Problem);
    fclose(Problem);

    system(COMANDO);

    FILE *Plain = fopen("Plain.txt", "rt");
    read_plan(Plain);
    fclose(Plain);
    remove("Plain.txt");
}
