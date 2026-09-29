// =====================================================
//  display.h  -  Botão (toque rápido / segurado) +
//                menu de estratégias no OLED 0.91" (SSD1306)
//
//  Bibliotecas: Adafruit SSD1306 e Adafruit GFX
//
//  Uso no .ino:
//      #include "display.h"
//
//      void setup() {
//        Serial.begin(115200);
//        iniciarDisplay();
//      }
//
//      void loop() {
//        ToqueBotao toque = lerBotao();
//        int escolhida = atualizarMenu(toque);
//        if (escolhida >= 0) {
//          // estratégia confirmada (0 a 4)
//        }
//      }
// =====================================================

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------------- CONFIGURAÇÃO ----------------
// Para mudar algum valor, defina-o no .ino ANTES do #include "display.h"
#ifndef BOTAO
#define BOTAO A0
#endif

#ifndef LARGURA_OLED
#define LARGURA_OLED 128
#endif

#ifndef ALTURA_OLED
#define ALTURA_OLED 32
#endif

#ifndef ENDERECO_OLED
#define ENDERECO_OLED 0x3C
#endif

#ifndef TEMPO_DEBOUNCE
#define TEMPO_DEBOUNCE 30       // ms
#endif

#ifndef TEMPO_TOQUE_MAX
#define TEMPO_TOQUE_MAX 800    // toque rápido: soltar antes disso (ms)
#endif

#ifndef TEMPO_SEGURADO
#define TEMPO_SEGURADO 1000     // segurado: manter pressionado até isso (ms)
#endif

// ---------------- BOTÃO ----------------
enum ToqueBotao { NENHUM, TOQUE_RAPIDO, SEGURADO };

// Chame uma vez por volta do loop().
// Retorna NENHUM na maioria das chamadas; TOQUE_RAPIDO ou SEGURADO
// aparecem uma única vez por pressão do botão.
ToqueBotao lerBotao() {
  static bool iniciado = false;
  static bool nivelRepouso;
  static bool ultimaLeitura;
  static bool estavaPressionado = false;
  static bool seguradoEnviado = false;
  static unsigned long ultimaMudanca = 0;
  static unsigned long inicioPressao = 0;

  // primeira chamada: configura o pino e lê o nível com o botão solto
  if (!iniciado) {
    pinMode(BOTAO, INPUT_PULLUP);
    delay(100);
    nivelRepouso = digitalRead(BOTAO);
    ultimaLeitura = nivelRepouso;
    iniciado = true;
    return NENHUM;
  }

  unsigned long agora = millis();
  bool leitura = digitalRead(BOTAO);

  // debounce
  if (leitura != ultimaLeitura) {
    ultimaLeitura = leitura;
    ultimaMudanca = agora;
  }
  if (agora - ultimaMudanca < TEMPO_DEBOUNCE) return NENHUM;

  bool pressionado = (ultimaLeitura != nivelRepouso);

  // acabou de pressionar
  if (pressionado && !estavaPressionado) {
    estavaPressionado = true;
    seguradoEnviado = false;
    inicioPressao = agora;
    return NENHUM;
  }

  // continua pressionado: chegou no tempo de "segurado"?
  if (pressionado && estavaPressionado && !seguradoEnviado &&
      agora - inicioPressao >= TEMPO_SEGURADO) {
    seguradoEnviado = true;
    return SEGURADO;
  }

  // soltou
  if (!pressionado && estavaPressionado) {
    estavaPressionado = false;
    if (!seguradoEnviado && agora - inicioPressao < TEMPO_TOQUE_MAX) {
      return TOQUE_RAPIDO;
    }
    // entre TEMPO_TOQUE_MAX e TEMPO_SEGURADO: não conta como nada
  }

  return NENHUM;
}

// ---------------- DISPLAY ----------------
Adafruit_SSD1306 display(LARGURA_OLED, ALTURA_OLED, &Wire, -1);

// ---------------- MENU ----------------
// Edite os nomes aqui (sem acentos: a fonte padrão não tem)
#define TOTAL_ESTRATEGIAS 5
const char* estrategias[TOTAL_ESTRATEGIAS] = {
  "Estrategia 1",
  "Estrategia 2",
  "Estrategia 3",
  "Estrategia 4",
  "Estrategia 5"
};

enum EstadoTela { TELA_MENU, TELA_CONFIRMAR, TELA_CONFIRMADO };

EstadoTela estadoTela = TELA_MENU;
int itemAtual = 0;         // estratégia selecionada no menu
int primeiroVisivel = 0;   // primeira linha da janela de 4 itens
bool opcaoSim = false;     // opção marcada na caixinha (começa em "Nao")
int estrategiaAtiva = -1;  // última estratégia confirmada (-1 = nenhuma)

void desenharMenu() {
  // a tela mostra 4 linhas; a janela acompanha o item selecionado
  if (itemAtual < primeiroVisivel) primeiroVisivel = itemAtual;
  if (itemAtual > primeiroVisivel + 3) primeiroVisivel = itemAtual - 3;

  for (int i = 0; i < 4; i++) {
    int indice = primeiroVisivel + i;
    if (indice >= TOTAL_ESTRATEGIAS) break;

    int y = i * 8;
    if (indice == itemAtual) {
      display.fillRect(0, y, LARGURA_OLED, 8, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    display.setCursor(4, y);
    display.print(estrategias[indice]);
  }
}

void desenharCaixaConfirmar() {
  display.fillRect(14, 2, 100, 28, SSD1306_BLACK);
  display.drawRect(14, 2, 100, 28, SSD1306_WHITE);

  display.setTextColor(SSD1306_WHITE);
  display.setCursor(35, 6);
  display.print("Confirmar?");

  // opção Sim
  if (opcaoSim) {
    display.fillRect(24, 17, 32, 10, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
  } else {
    display.setTextColor(SSD1306_WHITE);
  }
  display.setCursor(31, 19);
  display.print("Sim");

  // opção Não
  if (!opcaoSim) {
    display.fillRect(72, 17, 32, 10, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
  } else {
    display.setTextColor(SSD1306_WHITE);
  }
  display.setCursor(79, 19);
  display.print("Nao");
}

void desenharConfirmado() {
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(4, 4);
  display.print(estrategias[estrategiaAtiva]);
  display.setCursor(4, 16);
  display.print("confirmada!");
}

void desenharTela() {
  display.clearDisplay();
  display.setTextSize(1);

  if (estadoTela == TELA_MENU) {
    desenharMenu();
  } else if (estadoTela == TELA_CONFIRMAR) {
    desenharMenu();
    desenharCaixaConfirmar();
  } else {
    desenharConfirmado();
  }

  display.display();
}

// Chame no setup(). Inicializa o botão (com ele solto), o OLED e desenha o menu.
// Retorna false se o OLED não for encontrado.
bool iniciarDisplay() {
  lerBotao();  // inicializa o botão

  if (!display.begin(SSD1306_SWITCHCAPVCC, ENDERECO_OLED)) {
    Serial.println("OLED nao encontrado");
    return false;
  }
  desenharTela();
  return true;
}

// Volta ao menu inicial (pode ser chamada de qualquer lugar do código).
void voltarAoMenu() {
  estadoTela = TELA_MENU;
  desenharTela();
}

// Chame no loop() passando o resultado de lerBotao().
// Retorna -1 na maioria das vezes; retorna o índice da estratégia
// (0 a TOTAL_ESTRATEGIAS-1) uma única vez, no momento em que ela é confirmada.
int atualizarMenu(ToqueBotao toque) {
  if (toque == NENHUM) return -1;

  int confirmada = -1;

  switch (estadoTela) {

    case TELA_MENU:
      if (toque == TOQUE_RAPIDO) {
        itemAtual = (itemAtual + 1) % TOTAL_ESTRATEGIAS;  // desce; volta ao início no fim
      } else if (toque == SEGURADO) {
        opcaoSim = false;
        estadoTela = TELA_CONFIRMAR;
      }
      break;

    case TELA_CONFIRMAR:
      if (toque == TOQUE_RAPIDO) {
        opcaoSim = !opcaoSim;                 // alterna Sim / Nao
      } else if (toque == SEGURADO) {
        if (opcaoSim) {
          estrategiaAtiva = itemAtual;
          confirmada = itemAtual;
          estadoTela = TELA_CONFIRMADO;
        } else {
          estadoTela = TELA_MENU;             // caixinha some
        }
      }
      break;

    case TELA_CONFIRMADO:
      if (toque == SEGURADO) {
        estadoTela = TELA_MENU;               // volta ao menu
      }
      break;
  }

  desenharTela();
  return confirmada;
}

#endif
