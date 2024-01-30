#include <Adafruit_NeoPixel.h>
#include <AFMotor.h>

#define LED_PIN      2  // Pino de controle do anel de LEDs
#define NUM_LEDS     16 // Número de LEDs no anel
#define BRIGHTNESS   50 // Brilho dos LEDs (ajuste conforme necessário)
#define DELAY_MS     100 // Atraso entre as atualizações da animação em milissegundos

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

int ledPosition = 0;  // Posição do LED atual
uint32_t ledColor = strip.Color(255, 0, 0);  // Cor inicial do LED (vermelho)
int colorCycle = 0;  // Contador de ciclos de cores

AF_DCMotor motor(1);  // Cria um objeto do tipo AF_DCMotor para controlar um motor no canal 1.
int potpin = 5;       // Define a porta analógica usada para conectar o potenciômetro.
int val;              // Variável para armazenar o valor lido da porta analógica.

bool animationEnabled = true; // Variável de controle da animação
bool vibracallActive = false; // Variável para rastrear o status do vibracall

void setup() {
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show();

  Serial.begin(115200);  // Inicia a comunicação serial com uma taxa de 115200 bps.
  motor.setSpeed(200);   // Define a velocidade do motor para 200 (0 a 255).
  motor.run(RELEASE);    // Define o motor para o estado "RELEASE" (desligado) no início.
}

void loop() {
  motor.run(FORWARD);   // Define o motor para girar no sentido "FORWARD" (frente).

  val = analogRead(potpin);  // Lê o valor do potenciômetro (0 a 1023).
  val = map(val, 0, 900, 0, 500);  // Mapeia o valor do potenciômetro para o intervalo 0-500.
  Serial.println(val);  // Imprime o valor do potenciômetro no monitor serial.

  motor.setSpeed(val);  // Define a velocidade do motor com base no valor do potenciômetro.

  // Verifique se o vibracall está ativado (valor do potenciômetro acima de um limite)
  if (val > 20) {
    // Desativar a animação
    animationEnabled = false;
    // Ativar o vibracall
    if (!vibracallActive) {
      strip.fill(strip.Color(0, 255, 0)); // Cor verde claro
      strip.show();
      vibracallActive = true;
    }
  } else {
    // Ativar a animação se não estiver ativa
    if (!animationEnabled) {
      animationEnabled = true;
      // Reinicie a animação
      ledPosition = 0;
      colorCycle = 0;
      ledColor = strip.Color(75, 0, 130); // Defina a cor inicial (roxo)
      vibracallActive = false;
    }
  }

  // Verifique se a animação está ativada
  if (animationEnabled) {
    // Acenda o LED na posição atual com a cor atual
    strip.setPixelColor(ledPosition, ledColor);
    strip.show();

    // Aguarde antes de atualizar a posição do LED
    delay(DELAY_MS);

    // Avance para a próxima posição do LED
    ledPosition = (ledPosition + 1) % NUM_LEDS;

    // Verifique se o ciclo de 16 LEDs foi concluído
    if (ledPosition == 0) {
      // Troque a cor do LED para a próxima cor do ciclo
      colorCycle = (colorCycle + 1) % 3;  // Alterne entre 3 cores diferentes

      switch (colorCycle) {
        case 0:
          ledColor = strip.Color(75, 0, 130);  // roxo
          break;
        case 1:
          ledColor = strip.Color(0, 255, 255);  // azul
          break;
        case 2:
          ledColor = strip.Color(255, 20, 147);  // rosa
          break;
      }
    }
  }
}
