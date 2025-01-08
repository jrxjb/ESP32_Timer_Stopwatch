#include <Arduino.h>
#include <SevSeg.h>
#include <Ticker.h>

#define MODE_COUNTDOWN 1
#define MODE_STOPWATCH 2

#define DEBOUNCE_DELAY 300  // Tiempo de debounce para pulsadores
#define DISPLAY_MODE_TIME 1000  // Tiempo para mostrar el modo en milisegundos

Ticker myTicker;
int remainingTime = 0; // Temporizador inicia en 0000
int elapsedTime = 0;
bool isRunning = false;  // Variable para controlar el estado del temporizador
bool isPaused = false;   // Variable para controlar el estado de pausa
bool showMode = false;   // Variable para indicar si se está mostrando el modo

int reset = 5;   // Pin para el botón de reset
int inicio = 18; // Pin para el botón de inicio
int pausa = 25;  // Pin para el botón de pausa
int modeCountdown = 34;  // Pin para el botón de modo Temporizador
int modeStopwatch = 35;  // Pin para el botón de modo Cronómetro

// Pines para los nuevos botones de ajuste de tiempo
int buttonXM1 = 19;  // Botón para aumentar la posición de mil minutos
int buttonXM2 = 21;  // Botón para aumentar la posición de cientos de minutos
int buttonXS1 = 22;  // Botón para aumentar la posición de decenas de segundos
int buttonXS2 = 12;  // Botón para aumentar la posición de unidades de segundos

int operationMode = MODE_STOPWATCH;  // Inicialmente seleccionado como cronómetro
unsigned long lastButtonPress = 0;  // Variable para el tiempo de la última pulsación de botón
unsigned long modeDisplayTime = 0;  // Variable para el tiempo de inicio de la pantalla de modo

bool btn35 = false;
bool btn34 = false;

SevSeg sevseg; // Instanciar un objeto controlador de siete segmentos

void printTimeToDisplay(int totalSeconds) {
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    int displayNumber = minutes * 100 + seconds;
    sevseg.setNumber(displayNumber, 0);
}

void adjustTime() {
    if (digitalRead(buttonXM1) == LOW) {
        if (remainingTime / 600 % 10 < 9) {
            remainingTime += 600; // Aumenta 10 minutos
        } else {
            remainingTime = remainingTime - (remainingTime % 6000) + (remainingTime % 600);
        }
        delay(200); // Debounce
    }
    if (digitalRead(buttonXM2) == LOW) {
        if (remainingTime / 60 % 10 < 9) {
            remainingTime += 60;  // Aumenta 1 minuto
        } else {
            remainingTime = remainingTime - (remainingTime % 600) + (remainingTime % 60);
        }
        delay(200); // Debounce
    }
    if (digitalRead(buttonXS1) == LOW) {
        if (remainingTime / 10 % 10 < 6) {
            remainingTime += 10;  // Aumenta 10 segundos
        } else {
            remainingTime -= 60;  // Resetea el dígito XS1 a 0
        }
        delay(200); // Debounce
    }
    if (digitalRead(buttonXS2) == LOW) {
        if (remainingTime % 10 < 9) {
            remainingTime += 1;   // Aumenta 1 segundo
        } else {
            remainingTime = remainingTime - (remainingTime % 10);
        }
        delay(200); // Debounce
    }

    printTimeToDisplay(remainingTime); // Actualiza la pantalla
}

void timerCallback() {
    if (!isPaused) {
        if (operationMode == MODE_COUNTDOWN) {
            if (remainingTime > 0) {
                remainingTime--;
                Serial.print("Tiempo restante: ");
                printTimeToDisplay(remainingTime);
                if (remainingTime == 0) {
                    myTicker.detach();
                    Serial.println("¡El temporizador ha terminado!");
                    isRunning = false;  // Detener el temporizador
                }
            }
        } else if (operationMode == MODE_STOPWATCH) {
            elapsedTime++;
            Serial.print("Tiempo transcurrido: ");
            printTimeToDisplay(elapsedTime);
        }
    }
}

void setup() {
    Serial.begin(9600); 

    byte numDigits = 4;
    byte digitPins[] = {27, 23, 16, 17}; // Pines de los dígitos: 1, 2, 3, 4
    byte segmentPins[] = {15, 4, 14, 33, 13, 2, 26, 32}; // Pines de los segmentos: A, B, C, D, E, F, G, DP

    sevseg.begin(COMMON_ANODE, numDigits, digitPins, segmentPins); // Configurar el display
    sevseg.setBrightness(10); // Configurar el brillo

    // Configurar pines de botones como entrada
    pinMode(reset, INPUT_PULLUP);
    pinMode(inicio, INPUT_PULLUP);
    pinMode(pausa, INPUT_PULLUP); // Configurar el botón de pausa
    pinMode(modeCountdown, INPUT_PULLUP); // Configurar el botón de modo Temporizador
    pinMode(modeStopwatch, INPUT_PULLUP); // Configurar el botón de modo Cronómetro
    pinMode(buttonXM1, INPUT_PULLUP);
    pinMode(buttonXM2, INPUT_PULLUP);
    pinMode(buttonXS1, INPUT_PULLUP);
    pinMode(buttonXS2, INPUT_PULLUP);

    //elapsedTime = 99 * 60; // Cronometro, 99 Representar los minutos a probar
    //remainingTime = 99 * 60; //Temporizador,99 Representar los minutos a probar
    // Mostrar "0000" al inicio
    printTimeToDisplay(0);
}

void loop() {
    unsigned long currentTime = millis();

    // Cambiar modo basado en el estado de los pines
    if (digitalRead(modeCountdown) == LOW && (btn34) == false) {
        btn34 = true;
        btn35 = false;
            operationMode = MODE_COUNTDOWN;
            Serial.println("Modo: Temporizador");
            sevseg.setChars("1111"); // Mostrar 1111 para Temporizador
            showMode = true;
            modeDisplayTime = currentTime;

    }

    if (digitalRead(modeStopwatch) == LOW &&  (btn35) == false) {
        btn35 = true;
        btn34 = false;
            operationMode = MODE_STOPWATCH;
            Serial.println("Modo: Cronómetro");
            sevseg.setChars("C"); // Mostrar C para Cronómetro
            showMode = true;
            modeDisplayTime = currentTime;




    }

    // Mostrar el modo durante 1 segundo y luego refrescar la pantalla
    if (showMode && (currentTime - modeDisplayTime > DISPLAY_MODE_TIME)) {
        showMode = false;
        if (operationMode == MODE_COUNTDOWN) {
            printTimeToDisplay(remainingTime);
        } else {
            printTimeToDisplay(elapsedTime);
        }
    }

    if (!showMode) {
        if (operationMode == MODE_COUNTDOWN) {
            adjustTime(); // Ajustar el tiempo con los botones solo en modo temporizador
        }

        if (digitalRead(inicio) == LOW && !isRunning && ((operationMode == MODE_STOPWATCH) || (operationMode == MODE_COUNTDOWN && remainingTime > 0))) {
            isRunning = true;
            isPaused = false;
            myTicker.attach(1, timerCallback);
        }
        if (digitalRead(reset) == LOW) {
            isRunning = false;
            isPaused = false;
            myTicker.detach();
            remainingTime = 0;
            elapsedTime = 0;
            Serial.println("Reset del temporizador.");
            printTimeToDisplay(0);
        }
        if (digitalRead(pausa) == LOW) {
            isPaused = !isPaused; // Alterna el estado de pausa
            delay(200); // Debounce
        }
    }

    sevseg.refreshDisplay(); // Debe ejecutarse repetidamente para actualizar el display

}
