/*
 * 
 *  PIC - PX-DOS Interpreter Code - Código Interpretado do PX-DOS
 * 
 * Versão 0.1.1 Beta
 * 
 * Versão de testes 0
 * 
 * 
 * Copyright (C) 2013 Felipe Miguel Nery Lunkes
 * 
 */
 

#include "stdio.h"
#include "setjmp.h"
#include "math.h"
#include "ctype.h"
#include "stdlib.h"
#include <pxdos.h>

#define WINDOWS

#define NUM_LAB 100
#define TAM_ROTULO 10 
#define FOR_NEST 25
#define SUB_NEST 25
#define TAMANHO_PROG 10000

#define DELIMITADOR  1
#define VARIAVEL  2
#define NUMERO    3
#define COMANDO   4
#define STRING	  5
#define QUOTE	  6

#define IMPRIMA 1
#define LEIA 2
#define SE    3
#define ENTAO  4
#define PARA   5
#define PROXIMO  6
#define DEST    7
#define IR  8
#define EOL   9
#define TERMINADO  10
#define IRSUB 11
#define RETORNAR 12
#define SISTEMA 13
#define CLS 14
#define SISVER 15
#define ESPERAR 16
#define REM 17
#define INT 18
#define DECLARAR 19
#define SOBRE 20
#define BEEP 21
#define VERSIS 22
#define ASSINATURA 23
#define INIT32 24
#define SHELL 25
#define VERSAO 26
#define PIC 27
#define ALOC 28

#define AUTOR "Felipe Miguel Nery Lunkes"
#define PT_BR

#define VERSAOAPP "1.0.1"
#define MES "Novembro"
#define DIA "08"
#define ANO "2013"

char *prog;  /* Expressão a ser analisada . */
jmp_buf e_buf; 

int variaveis[26]= {    /* 26 variáveis do usuário, disponíveis,  A-Z */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0
};

struct comandos{
  char comando[20];
  char tok;
} tabela[] = { /* Comandos aceitos */
  "esc", IMPRIMA, 
  "def", REM,
  "pic_2", REM,
  "clrscr", CLS,
  "pic.obterversao", VERSAO,
  "entrada", LEIA,
  "obter", LEIA,
  #ifdef PXDOS
  "sisver", VERSIS,
  #endif
  "se", SE,
  "entao", ENTAO,
  "irpara", IR,
  "para", PARA,
  "proximo", PROXIMO,
  "dest", DEST,
  "irsub", IRSUB,
  "retornar.16;", RETORNAR,
  "sistema", SISTEMA,
  #ifdef PXDOS
  "sisver", SISVER,
  #endif
  "aloc ", ALOC,
  "; ", REM,
  "! ", REM,
  "@ ", REM,
  "assinatura", ASSINATURA,
  "beep", BEEP,
  "declarar", DECLARAR,
  "int", INT,
  "pausa", ESPERAR,
  "shell", SHELL,
  "#", REM,
  "prot", REM,
  "iniciar.32;", INIT32,
  "sobre", SOBRE,
  "", SISTEMA  
};

char conteudo[80];
char tipo_conteudo, tok;
int alocar_quant;

struct rotulo {
  char nome[TAM_ROTULO];
  char *p;  /* Ponteiro para o arquivo */
};
struct rotulo tabela_rotulo[NUM_LAB];

char *encontrar_rotulo(), *gpop();

struct stack_para {
  int var; /* Variável de contagem */
  int alvo;  /* Valor alvo */
  char *loc;
} fstack[FOR_NEST]; /* Ponteiro para o próximo loop */
struct stack_para fpop();

char *gstack[SUB_NEST];	/* Base para irsub */

int ftos; 
int gtos;  

void imprimir(), sombeep(), versaosis(), sobre_mvb(), esperarteclado(), procurar_rotulos(), encontrar_nova_linha(), executar_irpara();
void executar_se(), executar_para(), proximo(), fpush(), entrada();
void irsub(), irsub_obter_retorno(), gpush(), iniciar_rotulo();
void serro(), obter_entrada(), putback();
void nivel2(), nivel3(), nivel4(), nivel5(), nivel6(), primitivo(), ALOCAR();
void unary(), arith();

main(argc, argv)
int argc;
char *argv[];
{
  char em[80];
  int resposta;
  char *p_buf;
  char *t;

  if(argc!=2) {
#ifdef WINDOWS
printf("\nPIC do PX-DOS %s para desenvolvimento sobre o Windows\n", VERSAOAPP);
#endif
#ifdef PXDOS
printf("\nPIC %s\n", VERSAOAPP);
#endif
	printf("\n\nUse o interpretador de scripts PIC para executar scripts\n");
	printf("na linguagem PX-DOS Interpreter Code, previamente escritos.\n");
	printf("PIC para PX-DOS.\n");
	printf("Copyright (C) 2013 Felipe Miguel Nery Lunkes\n\n");
	printf("\nUso: pxpic <script.pic>\n\n");
#ifdef WINDOWS
printf("\nAviso! Nesta versao para Windows alguns comandos e recursos foram\n");
printf("removidos por erros de compatibilidade e/ou falta de recursos nativos\n");
printf("compativeis com o PX-DOS, a qual o desenvolvimento nesta linguagem se\n");
printf("destina.\n\n");
#endif	
    sair(1);
  }

  /* Aloca memória para o programa */
  if(!(p_buf=(char *) malloc(TAMANHO_PROG))) {
 #ifdef PXDOS 
    printf("\n\nPIC para PX-DOS\n");
 #endif
#ifdef WINDOWS
printf("\n\nPIC do PX-DOS para desenvolvimento e manutencao sobre o Windows\n");
#endif 
	printf("Erro ao alocar memoria para o programa e seus componentes.\n\n");
	printf("Desculpe pelo transtorno, mas tente reiniciar seu computador ou\n");
	printf("executar o programa novamente.\n\n");
    sair(1);
  }
  
  /* Carrega o programa para a execução */
  if(!carregar_programa_bas(p_buf,argv[1])) sair(1);

  if(setjmp(e_buf)) sair(1); /* Inicializa o jmp do buffer */

  prog = p_buf;
  procurar_rotulos(); /* Procura rótulos no programa */
  ftos = 0; /* inicializa fpos */
  gtos = 0; /* inicializa gpos */
  do {
    tipo_conteudo = obter_conteudo();
    /* Checa para atribuir */
    if(tipo_conteudo==VARIAVEL) {
      putback(); 
      atribuir(); 
    }
    else /* Executa os comandos e rótulos verificados */
      switch(tok) {
        case IMPRIMA:
	  imprimir();
  	  break;
        case IR:
	  executar_irpara();
	  break;
	case SE:
	  executar_se();
	  break;
	case SOBRE:
	  sobre_mvb();
	  break;  
	case PARA:
	  executar_para();
	  break;
	  case BEEP:
	  sombeep();
	  break;
	  
	  case PIC:
	  
	  break;
	  
	  case ALOC:
	  
	 alocar();
	  
	  break;
	  
	  case VERSAO:
	  printf("\n\nPIC - PX-DOS Interpreter Code\n\nCodigo Interpretado do PX-DOS\n\n");
	  printf("Versao: %s\n\n", VERSAOAPP);
	  break;
	  
	case PROXIMO:
	  proximo();
	  break;
#ifdef PXDOS	  
	  case VERSIS:
	  versaosis();
	  break;
#endif	  
	  case DECLARAR:
	  
	  break;
	  
	  case SHELL:
	  
	  break;
	  case REM:
	  printf("");
	  break;
	  
	  case ASSINATURA:
	  

	  
	  break;
	  
	  case INIT32:
	  
        break;
  	case LEIA:
	  entrada();
	  break;
        case IRSUB:
	  irsub();
	  break;
	case RETORNAR:
	  irsub_obter_retorno();
	  break;
	  case CLS:
	  cls();
	  break;
	  case ESPERAR:
	  esperarteclado();
	  break;
        case SISTEMA:
	  sair(0);
      }
  } while (tok != TERMINADO);
}

void esperarteclado(void){
printf("\nPressione ENTER para continuar...\n");
asm {
mov ax, 0
int 16h
}
}

alocar(){
obter_conteudo();
     if (tipo_conteudo == NUMERO){
	 
	 alocar_quant=(int)conteudo;
	 
     malloc(alocar_quant);
     printf("\nAlocados %s bytes extras para o programa.\n", alocar_quant);
	 
	  }
	  if (tipo_conteudo != NUMERO){
	  
	  printf("\nImpossivel alocar memoria seguindo este valor nao numerico.\n");
	  
	  }
	  }

/* Carrega o programa. */
carregar_programa_bas(p, nome_arquivo_bas)
char *p;
char *nome_arquivo_bas;
{
  FILE *fp;
  int i=0;

  if(!(fp=abrir(nome_arquivo_bas, "rb"))) return 0;
  
  i = 0;
  do {
    *p = getc(fp);
    p++; i++;
  } while(!feof(fp) && i<TAMANHO_PROG);
  *(p-2) = '\0'; /*  Termina o programa */
  fechar(fp);
  return 1;
}

/* Atribui um valor a uma variável */
atribuir()
{
  int var, valor_var;

  /* Obtem o nome da variável */
  obter_conteudo();
  if(!ealpha(*conteudo)) {
    serro(4);
    return;
  }

  var = paramaiusculo(*conteudo)-'A';
 if (var == "ALOC"){
 alocar();
 }
  /* Obtem o símbolo de igual */
  obter_conteudo();
  if(*conteudo!='=') {
    serro(3);
    return;
  }

  /* Obtem o valor para atribuir a variável */
  obter_entrada(&valor_var);

  /* Atribui o valor */
  variaveis[var] = valor_var;
}

/* Executa a função imprimir "String" */
void imprimir()
{
  int resposta;
  int len=0, espacos;
  char ultimo_delimitador;

  do {
    obter_conteudo(); /* Obtem o próximo item */
    if(tok==EOL || tok==TERMINADO) break;
    if(tipo_conteudo==QUOTE) { /* É string */
      printf(conteudo);
      len += tamanhostring(conteudo);
      obter_conteudo();
    }
    else { /* É uma expressão */
      putback();
      obter_entrada(&resposta);
      obter_conteudo();
      len += printf("%d", resposta);
    }
    ultimo_delimitador = *conteudo; 

    if(*conteudo==';') {
            espacos = 8 - (len % 8); 
      len += espacos;
      while(espacos) { 
	printf(" ");
        espacos--;
      }
    }
    else if(*conteudo==',') /* Não faz nada */;
    else if(tok!=EOL && tok!=TERMINADO) serro(0); 
  } while (*conteudo==';' || *conteudo==',');

  if(tok==EOL || tok==TERMINADO) {
    if(ultimo_delimitador != ';' && ultimo_delimitador!=',') printf("\n");
  }
  else serro(0); 

}

/* Procura todos os rótulos. */
void procurar_rotulos()
{
  int addr;
  char *temp;

  iniciar_rotulo();  /* Zera os rótulos */
  temp = prog;   /* Salva o ponteiro para o início do programa */

  obter_conteudo();
  if(tipo_conteudo==NUMERO) {
    strcpy(tabela_rotulo[0].nome,conteudo);
    tabela_rotulo[0].p=prog;
  }

  encontrar_nova_linha();
  do {     
    obter_conteudo();
    if(tipo_conteudo==NUMERO) {
      addr = obter_proximo_rotulo(conteudo);
      if(addr==-1 || addr==-2) {
          (addr==-1) ?serro(5):serro(6);
      }
      strcpy(tabela_rotulo[addr].nome, conteudo);
      tabela_rotulo[addr].p = prog;  /* Ponteiro para o programa atual */
    }
    /* Se linha branca, procure próxima linha */
    if(tok!=EOL) encontrar_nova_linha();
  } while(tok!=TERMINADO);
  prog = temp;  /* Restaura para o original */
}

/* Encontra o início da próxima linha */
void encontrar_nova_linha()
{
  while(*prog!='\n'  && *prog!='\0') ++prog;
  if(*prog) prog++;
}

/* Retorna o status de um rótulo. 
   A -1 é retornado se a tabela de rótulos estiver cheia.
   A -2 é retornado em caso de rótulo redundante.
*/
obter_proximo_rotulo(s)
char *s;
{
  register int t;

  for(t=0;t<NUM_LAB;++t) {
    if(tabela_rotulo[t].nome[0]==0) return t;
    if(!strcmp(tabela_rotulo[t].nome,s)) return -2; /* dup */
  }

  return -1;
}

/*
 * Procura localização do próximo rótulo
*/
char *encontrar_rotulo(s)
char *s;
{
  register int t;

  for(t=0; t<NUM_LAB; ++t) 
    if(!strcmp(tabela_rotulo[t].nome,s)) return tabela_rotulo[t].p;
  return '\0'; /* Condição de erro */
}

/* Executa o IR (GOTO) */
void executar_irpara()
{

  char *loc;

  obter_conteudo(); /* Obtem o rótulo a ser chamado */
 
  loc = encontrar_rotulo(conteudo);
  if(loc=='\0')
    serro(7); /* Rótulo não definido */

  else prog=loc;  /* Inicia programa executado nesta localização*/
}


void iniciar_rotulo()
{
  register int t;

  for(t=0; t<NUM_LAB; ++t) tabela_rotulo[t].nome[0]='\0';
}

/* Executa o SE (IF) */
void executar_se()
{
  int x , y, cond;
  char op;

  obter_entrada(&x); 

  obter_conteudo(); /* Obtem o operador */
  if(!strchr("=<>", *conteudo)) {
    serro(0); /* Não é um operador legal */
    return;
  }
  op=*conteudo;

  obter_entrada(&y); 

  
  cond = 0;
  switch(op) {
    case '<':
      if(x<y) cond=1;
      break;
    case '>':
      if(x>y) cond=1;
      break;
    case '=':
      if(x==y) cond=1;
      break;
  }
  if(cond) { 
    obter_conteudo();
    if(tok!=ENTAO) {
      serro(8);
      return;
    }
  }
  else encontrar_nova_linha(); 
}

/* Execute um loop PARA. */
void executar_para()
{
  struct stack_para i;
  int valor_var;

  obter_conteudo(); /* Lê as variáveis de controle */
  if(!ealpha(*conteudo)) {
    serro(4);
    return;
  }

  i.var=paramaiusculo(*conteudo)-'A'; /* Salva na tabela */

  obter_conteudo(); /* Lê o símbolo de igual */
  if(*conteudo!='=') {
    serro(3);
    return;
  }

  obter_entrada(&valor_var); /* Obtem valor inicial */

  variaveis[i.var]=valor_var;

  obter_conteudo();
  if(tok!=DEST) serro(9); /* Lê e descarta DEST */

  obter_entrada(&i.alvo); /* Obtem valor alvo */

  
  if(valor_var>=variaveis[i.var]) { 
    i.loc = prog;
    fpush(i);
  }
  else 
    while(tok!=PROXIMO) obter_conteudo();
}

/* Executa o PROXIMO (NEXT) */
void proximo()
{
  struct stack_para i;

  i = fpop(); /* Obtem informções do loop */

  variaveis[i.var]++; /* Incrementa variável de controle */
  if(variaveis[i.var]>i.alvo) return;  /* Tudo pronto */
  fpush(i);  /* Restaura */
  prog = i.loc;  /* loop */
}

void sobre_mvb(){

printf("\nInterpretador PX-DOS Basic versao %s de %s de %s de %s\n", VERSAO,DIA,MES,ANO);

}

/* Função de guardar para lop PARA */
void fpush(i)
struct stack_para i;
{
   if(ftos>FOR_NEST)
    serro(10);

  fstack[ftos]=i;
  ftos++;
}

struct stack_para fpop()
{
  ftos--;
  if(ftos<0) serro(11);
  return(fstack[ftos]);
}

/* Executa a função LEIA (INPUT) */
void entrada()
{
  char str[80], var;
  int i;

  obter_conteudo(); /* Verifica se uma string está presente. */
  if(tipo_conteudo==QUOTE) {
    printf(conteudo); /* Se sim, imprima a mensagem */
    obter_conteudo();
    if(*conteudo!=',') serro(1);
    obter_conteudo();
  }
  else printf("> "); /* Imprima o prompt de entrada */
  var = paramaiusculo(*conteudo)-'A'; /* Obtem a entrada */

  scanf("%d", &i); /* Ler entrada */

  variaveis[var] = i; /* Guarda a entrada */
}

/* Executa IRSUB (GOSUB). */
void irsub()
{
  char *loc;

  obter_conteudo();
  /* Encontra a rotina para chamar */
  loc = encontrar_rotulo(conteudo);
  if(loc=='\0')
    serro(7); /* Rótulo não definido. */
  else {
    gpush(prog); /* Salva para retornar */
    prog = loc;  
  }
}

/* Retorno para IRSUB. */
void irsub_obter_retorno()
{
   prog = gpop();
}

/* IRSUB. Função de salvar */
void gpush(s)
char *s;
{
  gtos++;

  if(gtos==SUB_NEST) {
    serro(12);
    return;
  }

  gstack[gtos]=s;

}

/* IRSUB. Função de restaurar */
char *gpop()
{
  if(gtos==0) {
    serro(13);
    return 0;
  }

  return(gstack[gtos--]);
}

void sombeep(){

asm{
    mov ax, 3600
    mov cx, ax	 ;; Som a ser emitido		

	mov al, 182  ;; Dado a ser enviado
	out 43h, al  
	mov ax, cx			
	out 42h, al
	mov al, ah
	out 42h, al

	in al, 61h			
	or al, 03h
	out 61h, al

	in al, 61h
	and al, 0FCh
	out 61h, al

	}
	
}

#include "pic.h"

/* Ponto de entrada */
void obter_entrada(resultado)
int *resultado;
{
  obter_conteudo();
  if(!*conteudo) {
    serro(2);
    return;
  }
  nivel2(resultado);
  putback(); 
}


/* Mostra os erros */
void serro(erro)
int erro;
{
  static char *e[]= {   
    "Erro de Sintaxe. A sintaxe correta nao foi respeitada, e por isso o comando nao foi executado.", 
    "Paranteses requerido para este comando.", 
    "Nenhuma expressao coerente presente.",
    "Variavel detectada. O uso do '=' e obrigatorio para atribuir a ela um valor.",
    "Nao e uma variavel valida ou declarada.",
    "Lista de rotulos muito cheia. Impossivel definir mais rotulos neste script.",
    "Rotulo duplicado encontrado. Mais de um rotulo com o mesmo nome foi dclarado.",
    "Rotulo nao definido chamado. Impossivel chamar bloco de comandos nao declarado.",
    "ENTAO esperado.",
    "DEST esperado.",
    "Muitos loops PARA encontrados.",
    "PROXIMO sem PARA.",
    "Muitas instrucoes IRSUB detectadas.",
    "RETORNAR sem GOSUB."
	"Erro de alocacao de memoria. Algo aconteceu de errado."
	"Erro ao iniciar o servico de som."
	"Erro ao executar o servico beep."
	"Falha interna. Algum erro ocorreu. O mesmo nao pode ser determinado."
	"Erro geral de alocacao. Uma falha ocorreu no processo de alocacao de memoria."
	"Reinicie seu computador. Isso podera solucionar alguns problemas."
	"O Interpretador PIC foi chamado de uma forma invalida."
  }; 
  printf("%s\n", e[erro]); 
  longjmp(e_buf, 1); /* Retorna ao ponto salvado */
}

/* Obtem o conteúdo. */
obter_conteudo()
{

  register char *temp;

  tipo_conteudo=0; tok=0;
  temp=conteudo;

  if(*prog=='\0') { /* Fim do arquivo */
    *conteudo=0;
    tok = TERMINADO;
    return(tipo_conteudo=DELIMITADOR);
  }

  while(embranco(*prog)) ++prog;  /* Pula espaços */

  if(*prog=='\r') { /* crlf */
    ++prog; ++prog;
    tok = EOL; *conteudo='\r';
    conteudo[1]='\n'; conteudo[2]=0;
    return (tipo_conteudo = DELIMITADOR);
  }

  if(strchr("+-*^/%=;(),><", *prog)){ /* delimitador */
    *temp=*prog;
    prog++; /* avançar a próxima posição */
    temp++;
    *temp=0; 
    return (tipo_conteudo=DELIMITADOR);
  }
    
  if(*prog=='"') { /* quoted string */
    prog++;
    while(*prog!='"'&& *prog!='\r') *temp++=*prog++;
    if(*prog=='\r') serro(1);
    prog++;*temp=0;
    return(tipo_conteudo=QUOTE);
  }
  
  if(eumdigito(*prog)) { /* número */
    while(!eumdelimitador(*prog)) *temp++=*prog++;
    *temp = '\0';
    return(tipo_conteudo = NUMERO);
  }

  if(ealpha(*prog)) { /* variável ou comando */
    while(!eumdelimitador(*prog)) *temp++=*prog++;
    tipo_conteudo=STRING;
  }

  *temp = '\0';


  if(tipo_conteudo==STRING) {
    tok=ler_tabela_procurar(conteudo); 
    if(!tok) tipo_conteudo = VARIAVEL;
    else tipo_conteudo = COMANDO; /* É um comando */
  }
  return tipo_conteudo;
}



void putback() 
{

  char *t; 

  t = conteudo; 
  for(; *t; t++) prog--; 
}

/* Procura um valor na tabela
*/
ler_tabela_procurar(s)
char *s;
{
  register int i,j;
  char *p;

  /* Converte para minúsculo */
  p = s;
  while(*p){ *p = paraminusculo(*p); p++; }

  /* Verifica se o conteúdo está na tabela */
  for(i=0; *tabela[i].comando; i++)
      if(!strcmp(tabela[i].comando, s)) return tabela[i].tok;
  return 0; /* Comando desconhecido */
}

/* Retorna verdadeira se for um delimitador. */
eumdelimitador(c)
char c; 
{
  if(strchr(" ;,+-<>/*%^=()!#", c) || c==9 || c=='\r' || c==0) 
    return 1;  
  return 0;
}

/* Retorna 1 se for espaço ou tab. */
embranco(c)
char c;
{
  if(c==' ' || c=='\t') return 1;
  else return 0;
}



/*  Adiciona ou subtrai dois termos. */
void nivel2(resultado)
int *resultado;
{
  register char  op; 
  int bloqueado; 

  nivel3(resultado); 
  while((op = *conteudo) == '+' || op == '-') {
    obter_conteudo(); 
    nivel3(&bloqueado); 
    arith(op, resultado, &bloqueado);
  }
}

/* Multiplica ou divide dois fatores. */
void nivel3(resultado)
int *resultado;
{
  register char  op; 
  int bloqueado;

  nivel4(resultado); 
  while((op = *conteudo) == '*' || op == '/' || op == '%') {
    obter_conteudo(); 
    nivel4(&bloqueado); 
    arith(op, resultado, &bloqueado); 
  }
}

/* Processa expoente inteiro. */
void nivel4(resultado)
int *resultado;
{
  int bloqueado; 

  nivel5(resultado); 
  if(*conteudo== '^') {
    obter_conteudo(); 
    nivel4(&bloqueado); 
    arith('^', resultado, &bloqueado); 
  }
}

/* É + ou -. */
void nivel5(resultado)
int *resultado;
{
  register char  op; 

  op = 0; 
  if((tipo_conteudo==DELIMITADOR) && *conteudo=='+' || *conteudo=='-') {
    op = *conteudo; 
    obter_conteudo(); 
  }
  nivel6(resultado); 
  if(op)
    unary(op, resultado); 
}

/* Processa expressão entre parênteses */
void nivel6(resultado)
int *resultado;
{
  if((*conteudo == '(') && (tipo_conteudo == DELIMITADOR)) {
    obter_conteudo(); 
    nivel2(resultado); 
    if(*conteudo != ')')
      serro(1);
    obter_conteudo(); 
  }
  else
    primitivo(resultado);
}

/* Obtem valor da variável ou número. */
void primitivo(resultado)
int *resultado;
{

  switch(tipo_conteudo) {
  case VARIAVEL:
    *resultado = encontrar_variavel(conteudo);
    obter_conteudo(); 
    return; 
  case NUMERO:
    *resultado = atoi(conteudo);
    obter_conteudo();
    return;
  default:
    serro(0);
  }
}

/* Realiza operações especificadas. */
void arith(o, r, h)
char o;
int *r, *h;
{
  register int t, ex;

  switch(o) {
    case '-':
      *r = *r-*h; 
      break; 
    case '+':
      *r = *r+*h; 
      break; 
    case '*':  
      *r = *r * *h; 
      break; 
    case '/':
      *r = (*r)/(*h);
      break; 
    case '%':
      t = (*r)/(*h); 
      *r = *r-(t*(*h)); 
      break; 
    case '^':
      ex = *r; 
      if(*h==0) {
        *r = 1; 
        break; 
      }
      for(t=*h-1; t>0; --t) *r = (*r) * ex;
      break;       
  }
}


void unary(o, r)
char o;
int *r;
{
  if(o=='-') *r = -(*r);
}

/* Encontra o valor da variável. */
int encontrar_variavel(s)
char *s;
{
  if(!ealpha(*s)){
    serro(4); /* Não é uma variável */
    return 0;
  }
  return variaveis[paramaiusculo(*conteudo)-'A'];
}


