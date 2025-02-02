#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"

#define BTN_A_PIN 5
#define BTN_B_PIN 6

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
    } else {
        ssd1306_draw_string(ssd, 40, 32, text[2]);
        render_on_display(ssd, &frame_area);
    }

    sleep_ms(3000);
}

int increase_value(int value) {     // Função para incrementar o valor na posição indicada pelo apontador
    if (value < 9) {
        value++;
    } else {
        value = 0;
    }

    return value;
}

int move_pointer(int pnt) {     // Função para mover o apontador (para a direita)
    if (pnt < 3) {
        pnt++;
    } else {
        pnt = 0;
    }

    return pnt;
}

int auth(int* temp) {      // Compara o código atual (temporário) com o código de abertura do cofre
    int vault_code[4] = {4, 3, 2, 1};

    for (int i = 0; i < count_of(vault_code); i++) {
        if (temp[i] != vault_code[i]) {
            return 0;       // Código errado
        }
    }

    return 1;       // Código certo
}




int main() {
    stdio_init_all();
    // INICIANDO BOTÕES
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

    
    int temp[4] = {0, 0, 0, 0};     // Array temporário, contendo o código inserido pelo usuário
    int pointer = 0;    // Apontador para a posição dos dígitos
    int value = 0;      // Valor do dígito
    int status = 0;     // Verifica se o código inserido está correto

    // Verificaca se os botões foram pressionados e logo depois soltos
    int release_A = 1;      
    int release_B = 1;

    display_code(pointer, temp);
    printf("teste");

    while (true) {

        A_state = gpio_get(BTN_A_PIN);
        B_state = gpio_get(BTN_B_PIN);

        if (A_state == 1 && release_A == 0) {     // Botão A apertado e levantado, incrementa o valor na posição do apontador
            value = temp[pointer];
            temp[pointer] = increase_value(value);
            display_code(pointer, temp);
        } else if (B_state == 1 && release_B == 0) {     // Botão B apertado e levantado, incrementa o valor do apontador até o limite de 3
            pointer = move_pointer(pointer);
            display_code(pointer, temp);
        } else if (A_state == 0 && B_state == 0) {      // Se A e B forem pressionados simultaneamente, verifica se o código inserido corresponde ao de abertura do cofre
            status = auth(temp);
            display_message(status);
            memset(temp, 0, sizeof(temp));    // Define o array do código temporário para [0, 0, 0, 0] novamente
        }

        release_A = A_state;
        release_B = B_state;

        sleep_ms(20);       // Delay para evitar bouncing
    }

    return 0;

}