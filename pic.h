/*

PIC - PX-DOS Interpreter Code - Código de Interpretação do PX-DOS

Copyright © 2013 Felipe Miguel Nery Lunkes

Porção dependente da arquitetura e plataforma

*/

#define PXBAS_INCLUSO
#define APPVER "1.0"
#define PLATAFORMA "PX-DOS"
#define ARQUITETURA "x86"
#define MaquinaVitualVER "0.1.1"
#ifdef _PXDOS

void versaosis(){

// Este método será usado para chamar o sistema operacional PX-DOS
// para que o mesmo retorne sua versão.

asm {

mov ah, 01h
mov bx, 01h
int 90h

}
}

#endif
