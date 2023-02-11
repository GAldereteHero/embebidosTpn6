/* Copyright 2022, Laboratorio de Microprocesadores
 * Facultad de Ciencias Exactas y Tecnología
 * Universidad Nacional de Tucuman
 * http://www.microprocesadores.unt.edu.ar/
 * Copyright 2022, Esteban Volentini <evolentini@herrera.unt.edu.ar>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/** \brief Simple sample of use LPC HAL gpio functions
 **
 ** \addtogroup samples Sample projects
 ** \brief Sample projects to use as a starting point
 ** @{ */

/* === Headers files inclusions =============================================================== */

#include <stdbool.h>
#include <bsp.h>
#include "screen.h"
#include <chip.h>
#include "poncho.h"
#include "clock.h"

/* === Macros definitions ====================================================================== */

/* === Private data type declarations ========================================================== */

typedef enum {
    HORA_SIN_AJUSTAR,
    MOSTRANDO_HORA,
    AJUSTANDO_MINUTOS_ACTUAL,
    AJUSTANDO_HORAS_ACTUAL,
    AJUSTANDO_MINUTOS_ALARMA,
    AJUSTANDO_HORAS_ALARMA
} modo_t;

/* === Private variable declarations =========================================================== */

/* === Private function declarations =========================================================== */


/* === Public variable definitions ============================================================= */

static board_t board;

static modo_t modo;

static clock_t reloj;

static const uint8_t LIMITE_MINUTOS[] = {6,0};

static const uint8_t LIMITE_HORAS[] = {2,4};

/* === Private variable definitions ============================================================ */

/* === Private function implementation ========================================================= */

void ChangeMode(modo_t valor) {
    modo = valor;

    switch (modo){
    case HORA_SIN_AJUSTAR:
        DisplayBlinkDigits(board->display, 0, 3, 250);
        break;
    case MOSTRANDO_HORA:
        DisplayBlinkDigits(board->display, 0, 0, 0);
        break;
    case AJUSTANDO_MINUTOS_ACTUAL:
        DisplayBlinkDigits(board->display, 2, 3, 250);
        break;
    case AJUSTANDO_HORAS_ACTUAL:
        DisplayBlinkDigits(board->display, 0, 1, 250);
        break;
    case AJUSTANDO_MINUTOS_ALARMA:
        DisplayBlinkDigits(board->display, 2, 3, 250);
        break;
    case AJUSTANDO_HORAS_ALARMA:
        DisplayBlinkDigits(board->display, 0, 1, 250);
        break;
    
    default:
        break;
    }
}

clock_event_t AlarmaActivada(){

}

void IncrementBCD(uint8_t numero[2], const uint8_t limite[2]){
    numero[1]++;
    if(numero[1] > 9){
        numero[1] = 0;
        numero[0]++;
    }
    if((numero[0] == limite[0]) && (numero[1] == limite[1])){
        numero[0] = 0;
        numero[1] = 0;
    }
}

void DecrementBCD(uint8_t numero[2], const uint8_t limite[2]){
    numero[1]--;
    if(numero[1] > 9){
        numero[1] = 0;
        numero[0]--;
    }
    if((numero[0] == limite[0]) && (numero[1] == limite[1])){
        numero[0] = 0;
        numero[1] = 0;
    }
}

/* === Public function implementation ========================================================= */

int main(void) {
    uint8_t entrada[4];
    board = BoardCreate();
    reloj = ClockCreate(10, AlarmaActivada());
    SisTick_Init(1000);
    ChangeMode(HORA_SIN_AJUSTAR);

    while (true) {

        if(DigitalInputHasActivated(board->accept)){
            if(modo == MOSTRANDO_HORA){
                if(!ClockGetAlarm(reloj, entrada, sizeof(entrada))){
                    ClockToggleAlarm(reloj);
                }
            }else if(modo == AJUSTANDO_MINUTOS_ACTUAL){
                ChangeMode(AJUSTANDO_HORAS_ACTUAL);
            }else if(modo == AJUSTANDO_HORAS_ACTUAL){
                ClockSetupTime(reloj, entrada, sizeof(entrada));
                ChangeMode(MOSTRANDO_HORA);
            }else if(modo == AJUSTANDO_MINUTOS_ALARMA){
                ChangeMode(AJUSTANDO_HORAS_ALARMA);
            }else if(modo == AJUSTANDO_HORAS_ALARMA){
                ClockSetupAlarm(reloj, entrada, sizeof(entrada));
                ChangeMode(MOSTRANDO_HORA);
            }
        }
        if(DigitalInputHasActivated(board->cancel)){
            if(modo == MOSTRANDO_HORA){
                if(ClockGetAlarm(reloj, entrada, sizeof(entrada))){
                    ClockToggleAlarm(reloj);
                }
            }else{
                if(ClockGetTime(reloj, entrada, sizeof(entrada))){
                    ChangeMode(MOSTRANDO_HORA);
                }else{
                    ChangeMode(HORA_SIN_AJUSTAR);
                }
            }
        }
        if(DigitalInputHasActivated(board->setTime)){
            ChangeMode(AJUSTANDO_MINUTOS_ACTUAL);
            ClockGetTime(reloj, entrada, sizeof(entrada));
            DisplayWriteBCD(board->display, entrada, sizeof(entrada));
        }
        if(DigitalInputHasActivated(board->setAlarm)){
            ChangeMode(AJUSTANDO_MINUTOS_ALARMA);
            ClockGetAlarm(reloj, entrada, sizeof(entrada));
            DisplayWriteBCD(board->display, entrada, sizeof(entrada));
            DisplayToggleDots(board->display, 0, 3);
        }

        if(DigitalInputHasActivated(board->decrement)){
            if((modo == AJUSTANDO_MINUTOS_ACTUAL) || (modo == AJUSTANDO_MINUTOS_ALARMA)){
                DecrementBCD(&entrada[2], LIMITE_MINUTOS);
            } else if((modo == AJUSTANDO_HORAS_ACTUAL) || (modo == AJUSTANDO_HORAS_ALARMA)){
                DecrementBCD(&entrada[0], LIMITE_HORAS);
            }
            if((modo == AJUSTANDO_MINUTOS_ACTUAL) || (modo == AJUSTANDO_HORAS_ACTUAL)){
                DisplayWriteBCD(board->display, entrada, sizeof(entrada));
            } else if((modo == AJUSTANDO_HORAS_ACTUAL) || (modo == AJUSTANDO_MINUTOS_ALARMA)){
                DisplayWriteBCD(board->display, entrada, sizeof(entrada));
                DisplayToggleDots(board->display, 0, 3);
            }
        }
        if(DigitalInputHasActivated(board->increment)){
            if((modo == AJUSTANDO_MINUTOS_ACTUAL) || (modo == AJUSTANDO_MINUTOS_ALARMA)){
                IncrementBCD(&entrada[2], LIMITE_MINUTOS);
            } else if((modo == AJUSTANDO_HORAS_ACTUAL) || (modo == AJUSTANDO_HORAS_ALARMA)){
                IncrementBCD(&entrada[0], LIMITE_HORAS);
            }
            if((modo == AJUSTANDO_MINUTOS_ACTUAL) || (modo == AJUSTANDO_HORAS_ACTUAL)){
                DisplayWriteBCD(board->display, entrada, sizeof(entrada));
            } else if((modo == AJUSTANDO_HORAS_ACTUAL) || (modo == AJUSTANDO_MINUTOS_ALARMA)){
                DisplayWriteBCD(board->display, entrada, sizeof(entrada));
                DisplayToggleDots(board->display, 0, 3);
            }
        }

        for (int index = 0; index < 20; index++) {
            for (int delay = 0; delay < 25000; delay++) {
                __asm("NOP");
            }
        }
    }
}

void SysTick_Handler(void) {
    static uint16_t contador = 0; 
    uint8_t hora[4];

    /* Refresco de la pantalla*/
    DisplayRefresh(board->display);

    /*Actualizamos el reloj*/
    ClockNewTick(reloj);

    contador = (contador + 1) % 1000;
    
    if(modo <= MOSTRANDO_HORA){
        /*Obtenemos la hora actual*/
        ClockGetTime(reloj, hora, sizeof(hora));
        /*Mostramos la hora en la pantalla*/
        DisplayWriteBCD(board->display, hora, sizeof(hora));
        if (contador > 500){
            DisplayToggleDots(board->display, 1, 1);
        }
        if(ClockGetAlarm(reloj, hora, sizeof(hora))){
            DisplayToggleDots(board->display, 3, 3);
        }
        
    }

}

/* === End of documentation ==================================================================== */

/** @} End of module definition for doxygen */
