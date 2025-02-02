#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include "ws2818b.pio.h"

#define BTN_A_PIN 5
#define BTN_B_PIN 6
// Definição do número de LEDs e pino.
#define LED_COUNT 25
#define LED_PIN 7

const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

int A_state = 0;    //Botao A está pressionado?
int B_state = 0;    //Botao B está pressionado?

char* code[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};  // Array de dígitos a ser usado para exibição no display

// Preparar área de renderização para o display (ssd1306_width pixels por ssd1306_n_pages páginas)
struct render_area frame_area = {
    start_column : 0,
    end_column : ssd1306_width - 1,
    start_page : 0,
    end_page : ssd1306_n_pages - 1
};

uint8_t ssd[ssd1306_buffer_length];


/**
 * INICIALIZAÇÃO 
 * MATRIZ DE LEDS
 */

// Definição de pixel GRB
struct pixel_t {
    uint8_t G, R, B; // Três valores de 8-bits compõem um pixel.
};
typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t; // Mudança de nome de "struct pixel_t" para "npLED_t" por clareza.

// Declaração do buffer de pixels que formam a matriz.
npLED_t leds[LED_COUNT];

// Variáveis para uso da máquina PIO.
PIO np_pio;
uint sm;

/**
* Inicializa a máquina PIO para controle da matriz de LEDs.
*/
void npInit(uint pin) {
    // Cria programa PIO.
    uint offset = pio_add_program(pio0, &ws2818b_program);
    np_pio = pio0;

    // Toma posse de uma máquina PIO.
    sm = pio_claim_unused_sm(np_pio, false);
    if (sm < 0) {
        np_pio = pio1;
        sm = pio_claim_unused_sm(np_pio, true); // Se nenhuma máquina estiver livre, panic!
    }

    // Inicia programa na máquina PIO obtida.
    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);

    // Limpa buffer de pixels.
    for (uint i = 0; i < LED_COUNT; ++i) {
        leds[i].R = 0;
        leds[i].G = 0;
        leds[i].B = 0;
    }
}

/**
* Atribui uma cor RGB a um LED.
*/
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
    leds[index].R = r;
    leds[index].G = g;
    leds[index].B = b;
}

/**
* Limpa o buffer de pixels.
*/
void npClear() {
    for (uint i = 0; i < LED_COUNT; ++i)
        npSetLED(i, 0, 0, 0);
}

/**
* Escreve os dados do buffer nos LEDs.
*/
void npWrite() {
    // Escreve cada dado de 8-bits dos pixels em sequência no buffer da máquina PIO.
    for (uint i = 0; i < LED_COUNT; ++i) {
    pio_sm_put_blocking(np_pio, sm, leds[i].G);
    pio_sm_put_blocking(np_pio, sm, leds[i].R);
    pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
}

/**
 * FIM DA
 * INICIALIZAÇÃO 
 * MATRIZ DE LEDS
 */




// Função de escrita no display OLED
void display_code(int pnt, int* temp) {
    // Resetar display OLED
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);
    int x = 35;

    // Desenha uma barra abaixo do valor atual do apontador
    int bar_position =  x + (pnt * 16);     
    ssd1306_draw_string(ssd, bar_position, 40, "-");
    render_on_display(ssd, &frame_area);

    // Muda valor do display baseado nos valores temporários salvos em cada posição
    for (int i = 0; i < 4; i++) {
        ssd1306_draw_string(ssd, x, 32, code[temp[i]]);
        render_on_display(ssd, &frame_area);
        x += 16;
    }
}

void draw_led(int* target, int R, int G) { // Função que liga a matriz de LED, recebendo os LEDs a serem ativos e qual cor utilizada
    for (int i = 0; i < 11; i++) {
        npSetLED(target[i], R, G, 0);
        npWrite();
        sleep_ms(10);
    }
}

void display_message(int status) {
    char* text[] = {"SENHA", "CORRETA", "ERRADA"};

    // Resetar display OLED
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    // Printa o "Senha" inicial
    ssd1306_draw_string(ssd, 42, 24, text[0]);
    render_on_display(ssd, &frame_area);

    if (status == 1) {
        ssd1306_draw_string(ssd, 40, 32, text[1]);
        render_on_display(ssd, &frame_area);
        int target[] = {1, 2, 3, 6, 7, 8, 13, 16, 22, 18, -1}; // LEDs a serem ativos, na cor verde
        draw_led(target, 0, 255);
    } else {
        ssd1306_draw_string(ssd, 40, 32, text[2]);
        render_on_display(ssd, &frame_area);
        int target[] = {1, 2, 3, 6, 7, 8, 11, 13, 16, 22, 18}; // LEDs a serem ativos, na cor vermelha
        draw_led(target, 255, 0);
    }

    sleep_ms(3000);
}

int increase_value(int value) { // Função para incrementar o valor na posição indicada pelo apontador
    if (value < 9) {
        value++;
    } else {
        value = 0;
    }

    return value;
}

int move_pointer(int pnt) { // Função para mover o apontador (para a direita)
    if (pnt < 3) {
        pnt++;
    } else {
        pnt = 0;
    }

    return pnt;
}

int auth(int* temp) { // Compara o código atual (temporário) com o código de abertura do cofre
    int vault_code[4] = {4, 3, 2, 1}; // Senha de teste, alterar a vontade

    for (int i = 0; i < count_of(vault_code); i++) {
        if (temp[i] != vault_code[i]) {
            return 0; // Código errado
        }
    }

    return 1; // Código certo
}




int main() {
    stdio_init_all();
    // Inicializando BOTÕES
    gpio_init(BTN_A_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);
    gpio_init(BTN_B_PIN);
    gpio_set_dir(BTN_B_PIN, GPIO_IN);
    gpio_pull_up(BTN_B_PIN);

    // Inicialização do i2c
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Processo de inicialização completo do OLED SSD1306
    ssd1306_init();
    calculate_render_area_buffer_length(&frame_area);

    // Inicializando matriz de LEDs NeoPixel
    npInit(LED_PIN);
    npClear();

    
    int temp[4] = {0, 0, 0, 0}; // Array temporário, contendo o código inserido pelo usuário
    int pointer = 0; // Apontador para a posição dos dígitos
    int value = 0; // Valor do dígito
    int status = 0; // Verifica se o código inserido está correto

    // Verificaca se os botões foram pressionados e logo depois soltos
    int release_A = 1;      
    int release_B = 1;

    display_code(pointer, temp);
    printf("teste");

    while (true) {

        A_state = gpio_get(BTN_A_PIN);
        B_state = gpio_get(BTN_B_PIN);

        if (A_state == 1 && release_A == 0) { // Botão A apertado e levantado, incrementa o valor na posição do apontador
            value = temp[pointer];
            temp[pointer] = increase_value(value);
            display_code(pointer, temp);
        } else if (B_state == 1 && release_B == 0) { // Botão B apertado e levantado, incrementa o valor do apontador até o limite de 3
            pointer = move_pointer(pointer);
            display_code(pointer, temp);
        } else if (A_state == 0 && B_state == 0) { // Se A e B forem pressionados simultaneamente, verifica se o código inserido corresponde ao de abertura do cofre
            status = auth(temp);
            display_message(status);
            memset(temp, 0, sizeof(temp)); // Define o array do código temporário para [0, 0, 0, 0] novamente
            npClear(); // Resetar matriz de LED
            npWrite();
        }

        release_A = A_state;
        release_B = B_state;

        sleep_ms(20); // Delay para evitar bouncing
    }

    return 0;

}