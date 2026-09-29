#include "display.h"

void setup() {
  Serial.begin(115200);
  iniciarDisplay();
}

void loop() {
  ToqueBotao toque = lerBotao();
  int escolhida = atualizarMenu(toque);
  if (escolhida >= 0) {
    // estratégia confirmada (0 a 4)
  }
}