#include "log_controller.h"


void initCSV(){
    FILE* csv = fopen(CSV_PATH, "w+");
    fwrite("data e hora,registro\n" , 1 , strlen("data e hora,temperatura,umidade\n") , csv);
    fclose(csv);
}

void saveInFile(void* args){
    char* dados = args;
    time_t timer;
    char dateTime[26];
    struct tm* tm_info;

    timer = time(NULL);
    tm_info = localtime(&timer);

    strftime(dateTime, 26, "%Y-%m-%d %H:%M:%S", tm_info);

    char linha[200];
    sprintf(linha, "%s,%s\n", dateTime, dados);
    FILE* csv = fopen(CSV_PATH, "a+");
    fwrite(linha, 1, strlen(linha), csv);
    fclose(csv);
}
