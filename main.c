//includes
#include "pic18f4520.h"
#include "config.h"
#include "atraso.h"
#include "bits.h"
#include "rtc.h"
#include "lcd.h"
#include "keypad.h"


//variaveis globais
char condRetornaInicial = 0;//condição para voltar para a tela inicial
char identificaSinal = 0;//variavel para identificar se é uma mensagem ou uma ligação

unsigned char listaLetras[] = {'1',     //ao apertr 1 na mensagem
'a','b','c','A','B','C','2',            //ao apertr 2 na mensagem
'd','e','f','D','E','F','3',            //ao apertr 3 na mensagem
'g','h','i','G','H','I','4',            //ao apertr 4 na mensagem
'j','k','l','J','K','L','5',            //ao apertr 5 na mensagem
'm','n','o','M','N','O','6',            //ao apertr 6 na mensagem
'p','q','r','s','P','Q','R','S','7',    //ao apertr 7 na mensagem
't','u','v','T','U','V','8',            //ao apertr 8 na mensagem
'w','x','y','z','W','X','Y','Z','9',    //ao apertr 9 na mensagem
' ','#','*','0'};                       //ao apertr 0 na mensagem



//prototipos de funcoes
void menu(void);
void ligacao(void);
void mensagem(void);
void simulaMensagem(void);
void simulaChamada(void);
void imprimeSimbolo(unsigned char);

void main(void){
    //variaveis locais
    unsigned char i, tecla;
    
    //inicializacao do sistema
    TRISA = 0xC3;
    TRISB = 0x03;
    TRISC = 0x01;
    TRISD = 0x00;
    TRISE = 0x00;
    ADCON1 = 0x06;
    
    lcd_init();
    lcd_cmd(L_NCR);
    
    
    char teleph[32] =
    {0x00, 0x00, 0x07, 0x0F, 0x0F, 0x0F, 0x0E, 0x0E,
    0x00, 0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x00,
    0x06, 0x07, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x06, 0x0F, 0x1F, 0x1F, 0x1E, 0x00, 0x00, 0x00};
    char mensg[32] =
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x18, 0x14,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x03, 0x05,
    0x12, 0x11, 0x10, 0x10, 0x1F, 0x00, 0x00, 0x00,
    0x09, 0x11, 0x01, 0x01, 0x1F, 0x00, 0x00, 0x00};
    
    //armazena o simbolo do telefone
    lcd_cmd(0x40);
    for(i=0;i<32;i++){
        lcd_dat(teleph[i]);
    }
    
    //armazena o simbolo da mensagem
    lcd_cmd(0x60);
    for(i=0;i<32;i++){
        lcd_dat(mensg[i]);
    }
    
    
    for(;;){//loop da tela principal
        rtc_r();
        lcd_cmd(L_CLR);
        lcd_pos(1, 4);
        lcd_str((const char *) date);
        lcd_pos(2, 5);
        for(i=0;i<5;i++){ lcd_dat((unsigned char) time[i]); }
        tecla = tc_tecla(3000);
        for(i=0;i<=12;i++){
            if(tecla == i){
                menu();
                break;
            }
        }
        if(PORTE == 0x01){//se receber algum "sinal"
            switch (identificaSinal){
                case 1:
                    simulaChamada();
                    break;
                case 2:
                    simulaMensagem();
                    break;
            }
            identificaSinal = 0;
            PORTE = 0x00;
        }
    }
}

//funcoes
void menu(){
    unsigned char i, tecla, tempo;
    lcd_cmd(L_CLR);
    lcd_str("1-Ligar");
    lcd_pos(1, 0);
    lcd_str("2-Mensagem");
    lcd_pos(2, 0);
    lcd_str("3-Teste Ligar");
    lcd_pos(3, 0);
    lcd_str("4-Teste Mensagem");
    tempo = 0;
    for(;;){
        tecla = tc_tecla(1000);
        switch (tecla){
            case 1:
                ligacao();
                condRetornaInicial = 1;
                break;
            case 2:
                mensagem();
                condRetornaInicial = 1;
                break;
            case 3:
                PORTE = 0x01;
                identificaSinal = 1;
                condRetornaInicial = 1;
                break;
            case 4:
                PORTE = 0x01;
                identificaSinal = 2;
                condRetornaInicial = 1;
                break;
        }
        
        //se gastar cerca de 10s ou se pressionar * retorna à tela principal
        if(tecla == 10 || tempo > 10){
            condRetornaInicial = 1;
        }
        
        //para voltar ao relogio inicial
        if(condRetornaInicial){
            condRetornaInicial = 0;
            break;
        }
        tempo++;
    }
}

void mensagem(){
    lcd_cmd(L_CLR);
    lcd_cmd(L_CR);
    
    unsigned char tecla, enviar = 0, tempo, linha = 0, coluna = 0, mensagem[64], contMSG=0, letra, numero[12], contN = 0, i, old_TC;
    old_TC = TRISC;
    for(;;){
        tecla = tc_tecla(0);
        tempo = 0;
        //cada tecla pressionada tem seus caracteres que pode imprimir (veja listaLetras)
        switch (tecla){
            case 1:
                letra = 0;
                for(;;){
                    lcd_dat(listaLetras[letra]);
                    lcd_pos(linha, coluna);
                    tecla = tc_tecla(1000);
                    tempo++;
                    if(tecla == 1){
                        letra++;
                        if(letra == 1){ letra = 0; }
                        tempo = 0;
                    }
                    if(tempo>=3){
                        break;
                    }
                }
                mensagem[contMSG] = listaLetras[letra];
                contMSG++;
                coluna++;
                break;
            case 2:
                letra = 1;
                for(;;){
                    lcd_dat(listaLetras[letra]);
                    lcd_pos(linha, coluna);
                    tecla = tc_tecla(1000);
                    tempo++;
                    if(tecla == 2){
                        letra++;
                        if(letra == 8){ letra = 1; }
                        tempo = 0;
                    }
                    if(tempo>=3){
                        break;
                    }
                }
                mensagem[contMSG] = listaLetras[letra];
                contMSG++;
                coluna++;
                break;
            case 3:
                letra = 8;
                for(;;){
                    lcd_dat(listaLetras[letra]);
                    lcd_pos(linha, coluna);
                    tecla = tc_tecla(1000);
                    tempo++;
                    if(tecla == 3){
                        letra++;
                        if(letra == 15){ letra = 8; }
                        tempo = 0;
                    }
                    if(tempo>=3){
                        break;
                    }
                }
                mensagem[contMSG] = listaLetras[letra];
                contMSG++;
                coluna++;
                break;
            case 4:
                letra = 15;
                for(;;){
                    lcd_dat(listaLetras[letra]);
                    lcd_pos(linha, coluna);
                    tecla = tc_tecla(1000);
                    tempo++;
                    if(tecla == 4){
                        letra++;
                        if(letra == 22){ letra = 15; }
                        tempo = 0;
                    }
                    if(tempo>=3){
                        break;
                    }
                }
                mensagem[contMSG] = listaLetras[letra];
                contMSG++;
                coluna++;
                break;
            case 5:
                letra = 22;
                for(;;){
                    lcd_dat(listaLetras[letra]);
                    lcd_pos(linha, coluna);
                    tecla = tc_tecla(1000);
                    tempo++;
                    if(tecla == 5){
                        letra++;
                        if(letra == 29){ letra = 22; }
                        tempo = 0;
                    }
                    if(tempo>=3){
                        break;
                    }
                }
                mensagem[contMSG] = listaLetras[letra];
                contMSG++;
                coluna++;
                break;
            case 6:
                letra = 29;
                for(;;){
                    lcd_dat(listaLetras[letra]);
                    lcd_pos(linha, coluna);
                    tecla = tc_tecla(1000);
                    tempo++;
                    if(tecla == 6){
                        letra++;
                        if(letra == 36){ letra = 29; }
                        tempo = 0;
                    }
                    if(tempo>=3){
                        break;
                    }
                }
                mensagem[contMSG] = listaLetras[letra];
                contMSG++;
                coluna++;
                break;
            case 7:
                letra = 36;
                for(;;){
                    lcd_dat(listaLetras[letra]);
                    lcd_pos(linha, coluna);
                    tecla = tc_tecla(1000);
                    tempo++;
                    if(tecla == 7){
                        letra++;
                        if(letra == 45){ letra = 36; }
                        tempo = 0;
                    }
                    if(tempo>=3){
                        break;
                    }
                }
                mensagem[contMSG] = listaLetras[letra];
                contMSG++;
                coluna++;
                break;
            case 8:
                letra = 45;
                for(;;){
                    lcd_dat(listaLetras[letra]);
                    lcd_pos(linha, coluna);
                    tecla = tc_tecla(1000);
                    tempo++;
                    if(tecla == 8){
                        letra++;
                        if(letra == 52){ letra = 45; }
                        tempo = 0;
                    }
                    if(tempo>=3){
                        break;
                    }
                }
                mensagem[contMSG] = listaLetras[letra];
                contMSG++;
                coluna++;
                break;
            case 9:
                letra = 52;
                for(;;){
                    lcd_dat(listaLetras[letra]);
                    lcd_pos(linha, coluna);
                    tecla = tc_tecla(1000);
                    tempo++;
                    if(tecla == 9){
                        letra++;
                        if(letra == 61){ letra = 52; }
                        tempo = 0;
                    }
                    if(tempo>=3){
                        break;
                    }
                }
                mensagem[contMSG] = listaLetras[letra];
                contMSG++;
                coluna++;
                break;
            case 0:
                letra = 61;
                for(;;){
                    lcd_dat(listaLetras[letra]);
                    lcd_pos(linha, coluna);
                    tecla = tc_tecla(1000);
                    tempo++;
                    if(tecla == 0){
                        letra++;
                        if(letra == 65){ letra = 61; }
                        tempo = 0;
                    }
                    if(tempo>=3){
                        break;
                    }
                }
                mensagem[contMSG] = listaLetras[letra];
                contMSG++;
                coluna++;
                break;
        }
        
        //confirma o envio da mensagem
        if(tecla == 12){
            enviar = 1;
            break;
        }
        
        //apaga um caractere da mensagem
        if(tecla == 10){
            if(linha>0){
                if(coluna == 0){
                    coluna = 16;
                    linha--;
                }
                
            }
            if(coluna > 0){
                lcd_pos(linha,--coluna);
                lcd_dat(' ');
                lcd_pos(linha,coluna);
                contMSG--;
            }
        }
        
        //quebra a linha automaticamente
        if(coluna == 16){
            linha++;
            coluna = 0;
        }
        lcd_pos(linha,coluna);
    }
    if(enviar){
        lcd_cmd(L_CLR);
        lcd_pos(1,0);
        lcd_str("Enviar para:");
        lcd_pos(2,0);
        
        for(;;){
            tecla = tc_tecla(0);
            switch (tecla){
                case 1:
                    numero[contN] = tecla+48;
                    lcd_dat(numero[contN]);
                    contN++;
                    break;
                case 2:
                    numero[contN] = tecla+48;
                    lcd_dat(numero[contN]);
                    contN++;
                    break;
                case 3:
                    numero[contN] = tecla+48;
                    lcd_dat(numero[contN]);
                    contN++;
                    break;
                case 4:
                    numero[contN] = tecla+48;
                    lcd_dat(numero[contN]);
                    contN++;
                    break;
                case 5:
                    numero[contN] = tecla+48;
                    lcd_dat(numero[contN]);
                    contN++;
                    break;
                case 6:
                    numero[contN] = tecla+48;
                    lcd_dat(numero[contN]);
                    contN++;
                    break;
                case 7:
                    numero[contN] = tecla+48;
                    lcd_dat(numero[contN]);
                    contN++;
                    break;
                case 8:
                    numero[contN] = tecla+48;
                    lcd_dat(numero[contN]);
                    contN++;
                    tempo = 0;
                    break;
                case 9:
                    numero[contN] = tecla+48;
                    lcd_dat(numero[contN]);
                    contN++;
                    break;
                case 0:
                    numero[contN] = 48;
                    lcd_dat(numero[contN]);
                    contN++;
                    break;
            }
            if(tecla == 12){
                break;
            }
        }
        TRISC = 0x00;
        PORTC = 0x01;
        lcd_cmd(L_CLR);
        imprimeSimbolo(2);
        lcd_pos(1,5);
        lcd_str("Mensagem");
        lcd_pos(2,5);
        lcd_str("Enviada");
        lcd_pos(3,5);
        for(i=0;i<contN;i++){
            lcd_dat(numero[i]);
        }
        atraso_ms(1000);
        PORTC = 0x00;
    }
    TRISC = old_TC;
}

void ligacao(){
    unsigned char old_TC = TRISC;
    lcd_cmd(L_CLR);
    lcd_cmd(L_CR);
    lcd_pos(1,0);
    unsigned char tecla, buzz = 1, ligar = 0, tempo = 0, numero[12], contN = 0, i;
    for(;;){
        tecla = tc_tecla(0);
        
        //cada tecla representa seu proprio numero, que é imprimido e armazenado
        switch (tecla){
            case 1:
                numero[contN] = tecla+48;
                lcd_dat(numero[contN]);
                contN++;
                break;
            case 2:
                numero[contN] = tecla+48;
                lcd_dat(numero[contN]);
                contN++;
                break;
            case 3:
                numero[contN] = tecla+48;
                lcd_dat(numero[contN]);
                contN++;
                break;
            case 4:
                numero[contN] = tecla+48;
                lcd_dat(numero[contN]);
                contN++;
                break;
            case 5:
                numero[contN] = tecla+48;
                lcd_dat(numero[contN]);
                contN++;
                break;
            case 6:
                numero[contN] = tecla+48;
                lcd_dat(numero[contN]);
                contN++;
                break;
            case 7:
                numero[contN] = tecla+48;
                lcd_dat(numero[contN]);
                contN++;
                break;
            case 8:
                numero[contN] = tecla+48;
                lcd_dat(numero[contN]);
                contN++;
                tempo = 0;
                break;
            case 9:
                numero[contN] = tecla+48;
                lcd_dat(numero[contN]);
                contN++;
                break;
            case 0:
                numero[contN] = 48;
                lcd_dat(numero[contN]);
                contN++;
                break;
        }
        
        //confirma a ligação
        if(tecla == 12){
            ligar = 1;
            break;
        }
        
        //cancela a ligação
        if(tecla == 10){
            break;
        }
    }
    lcd_cmd(L_NCR);
    
    PORTC = 0x01;
    if (ligar){
        lcd_cmd(L_CLR);
        imprimeSimbolo(1);
        lcd_pos(1,5);
        lcd_str("Ligando");
        lcd_pos(2,5);
        for(i=0;i<contN;i++){
            lcd_dat(numero[i]);
        }
        
        //a ligação ficará tocando, até você recusar apertando * ou deixar o tempo máximo
        for(;;){
            if(buzz){
                TRISC = 0x02;
                buzz = 0;
            }else{
                TRISC = 0x00;
                buzz = 1;
            }

            tecla = tc_tecla(1000);
            tempo++;
            if(tempo>10 || tecla == 10){
                break;
            }
        }
        lcd_cmd(L_CLR);
        imprimeSimbolo(1);
        lcd_pos(1,5);
        lcd_str("Fim da");
        lcd_pos(2,5);
        lcd_str("Chamada");
        
        for(i=0; i<6; i++){
            if(buzz){
                TRISC = 0x02;
                buzz = 0;
            }else{
                TRISC = 0x00;
                buzz = 1;
            }
            atraso_ms(400);
        }
    }
    PORTC = 0x00;
    TRISC = old_TC;
}

void simulaMensagem(){//apresenta uma simulação de uma mensagem recebida
    lcd_cmd(L_CLR);
    unsigned char tecla;
    imprimeSimbolo(2);
    lcd_pos(1,5);
    lcd_str("Mensagem");
    lcd_pos(2,5);
    lcd_str("Recebida");
    for(;;){
        tecla = tc_tecla(0);
        
        //apenas quando a tecla de confirmação OK (#) a mensagem sera exibida
        if(tecla == 12){
            break;
        }
    }
    
    lcd_cmd(L_CLR);
    
    lcd_str("Opa eae vei blz, bora ir num churras?");
    lcd_pos(3,0);
    lcd_str("Rmt: Fei_Ja1");
    for(;;){
        tecla = tc_tecla(0);
        if(tecla == 10){
            break;
        }
    }
}

void simulaChamada(){//apresenta uma simulação de uma chamada recebida
    unsigned char old_TC = TRISC;
    unsigned char tecla, atender, tempo, buzz, i;
    tempo = 0;
    buzz = 1;
    
    lcd_cmd(L_CLR);
    imprimeSimbolo(1);
    lcd_pos(1,5);
    lcd_str("Chamada");
    lcd_pos(2,5);
    lcd_str("Recebida");
    
    //aqui o usuario poderia não atender (ou esperar o tempo até a ligação encerrar) ou atender
    for(;;){
        if(buzz){
            TRISC = 0x02;
            buzz = 0;
        }else{
            TRISC = 0x00;
            buzz = 1;
        }
        
        tecla = tc_tecla(1000);
        tempo++;
        
        if(tecla == 12){
            atender = 1;
            break;
        }
        
        if(tempo>10 || tecla == 10){
            atender = 0;
            break;
        }
    }
    TRISC = 0;
    if(atender){
        lcd_cmd(L_CLR);
        imprimeSimbolo(1);
        lcd_pos(1,5);
        lcd_str("Chamada");
        lcd_pos(2,5);
        lcd_str("Atendida");
        for(;;){
            tecla = tc_tecla(0);
            if(tecla == 10){
                break;
            }
        }
        lcd_cmd(L_CLR);
        imprimeSimbolo(1);
        lcd_pos(1,5);
        lcd_str("Fim da");
        lcd_pos(2,5);
        lcd_str("Chamada");
    }else{
        lcd_cmd(L_CLR);
        imprimeSimbolo(1);
        lcd_pos(1,5);
        lcd_str("Chamada");
        lcd_pos(2,5);
        lcd_str("Recusada");
    }
    for(i=0; i<6; i++){
        if(buzz){
            TRISC = 0x02;
            buzz = 0;
        }else{
            TRISC = 0x00;
            buzz = 1;
        }
        atraso_ms(400);
    }
    TRISC = old_TC;
}

void imprimeSimbolo(unsigned char val){
    if(val == 1){ //imprime telefone
        lcd_pos(1,0);
        lcd_dat(0);
        lcd_dat(1);
        lcd_pos(2,0);
        lcd_dat(2);
        lcd_dat(3);
    }else if(val == 2){ //imprime mensagem
        lcd_pos(1,0);
        lcd_dat(4);
        lcd_dat(5);
        lcd_pos(2,0);
        lcd_dat(6);
        lcd_dat(7);
    }
}