#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include "lwip/tcp.h"

#include "lib/ssd1306/ssd1306.h"
#include "lib/ssd1306/display.h"
#include "lib/led/led.h"
#include "lib/button/button.h"
#include "lib/matrix_leds/matrix_leds.h"
#include "lib/buzzer/buzzer.h"
#include "lib/aht20/aht20.h"
#include "lib/bmp280/bmp280.h"
#include "config/wifi_config.h"
#include "lib/webserver/webserver.h"
#include "lib/sensors/sensors.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "pico/bootrom.h"

ssd1306_t ssd; // Declaração do display OLED

SemaphoreHandle_t xMutexDisplay;
QueueHandle_t xQueueSensorData; // Fila para armazenar leituras dos sensores

float temp_max_limit = 26; 
float hum_max_limit = 80.0;  
volatile bool alarm_disable=false;

void button_callback(uint gpio, uint32_t events);
bool handle_http_request(const char *request, size_t request_size, char *response, size_t response_size);

void vSensorTask(void *pvParameters){
    init_i2c_sensor();      
    init_bmp280();        
    init_aht20();
    while (1) {
        SensorReadings readings = get_sensor_readings();
        xQueueOverwrite(xQueueSensorData, &readings); // Atualiza a fila com as leituras dos sensores
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay de 1 segundo
    }
} 

void vLimitsTask(void *pvParameters){
    SensorReadings readings;
    set_led_green();
    desenha_frame(status,1);
    // Flag para controlar se o alarme está ativo
    static bool alarm_active = false;
    while (1)
    {
        if(xQueuePeek(xQueueSensorData, &readings, portMAX_DELAY) == pdTRUE){
            if ((readings.aht_temp > temp_max_limit || readings.humidity > hum_max_limit) && !alarm_disable) {
                if (!alarm_active) {
                    printf("ALERTA! Limites excedidos.\n");
                    for (uint8_t i = 0; i < 2; i++)
                    {
                        play_tone(BUZZER_A_PIN,900);
                        set_led_red();
                        vTaskDelay(pdMS_TO_TICKS(250));
                        turn_off_leds();
                        stop_tone(BUZZER_A_PIN);
                        vTaskDelay(pdMS_TO_TICKS(250));
                    }
                    set_led_red();
                    desenha_frame(status,0);
                    alarm_active = true;
                }
            } else {
                if (alarm_active) {
                    printf("Alarmes desativados.\n");
                    set_led_green();
                    desenha_frame(status,1);
                    alarm_active = false;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void vDisplayTask(void *pvParameters){
    SensorReadings readings;
    bool cor=false;
    while (cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) <= 0)
    {
        if (xSemaphoreTake(xMutexDisplay,portMAX_DELAY) == pdTRUE){
            ssd1306_fill(&ssd, !cor);                     // Limpa o display
            ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor); // Desenha um retângulo
            ssd1306_draw_string(&ssd, "Aguardando", 10, 30); // Exibe mensagem de Wi-Fi desconectado
            ssd1306_draw_string(&ssd, cor ? "conexao.." : "conexao.", 10, 40); // Exibe mensagem de Wi-Fi desconectado
            printf("Wi-Fi not connected\n");
            ssd1306_send_data(&ssd);   // Envia os dados para o display
            xSemaphoreGive(xMutexDisplay);
        }
        cor=!cor;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    

    while (1)
    {
        if(xQueuePeek(xQueueSensorData, &readings, portMAX_DELAY) == pdTRUE)
        {
            if (xSemaphoreTake(xMutexDisplay,portMAX_DELAY) == pdTRUE){
                ssd1306_fill(&ssd, false);                     // Limpa o display
                ssd1306_rect(&ssd, 3, 3, 122, 60, true, false); // Desenha um retângulo
                
                ssd1306_line(&ssd,3,20,125,20,true); //Linha Horizontal
                ssd1306_line(&ssd,64,3,64,45,true);  //Linha Vertical

                ssd1306_draw_string(&ssd, "BMP280", 10, 10);
                char temp_str[10];
                snprintf(temp_str, sizeof(temp_str), "%.1fC", readings.bmp_temp);
                ssd1306_draw_string(&ssd, temp_str, 10, 25);

                char altitude_str[10];
                snprintf(altitude_str, sizeof(altitude_str), "%.1fm", readings.altitude_m);
                ssd1306_draw_string(&ssd, altitude_str, 10, 35);
                
                //AHT20 DADOS
                ssd1306_draw_string(&ssd, "AHT20", 70, 10);

                char temp_aht_str[10];
                snprintf(temp_aht_str, sizeof(temp_aht_str), "%.1fC", readings.aht_temp);
                ssd1306_draw_string(&ssd, temp_aht_str, 70, 25);

                char humidity_str[10];
                snprintf(humidity_str, sizeof(humidity_str), "%.1f%%", readings.humidity);
                ssd1306_draw_string(&ssd, humidity_str, 70, 35);

                ssd1306_line(&ssd,3,45,125,45,true); //Linha Horizontal
                ssd1306_draw_string(&ssd, cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) <= 0 ? "Wi-Fi: OFF" : "Wi-Fi: ON", 10, 50);

                
                ssd1306_send_data(&ssd);   // Envia os dados para o display
                xSemaphoreGive(xMutexDisplay);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vWebServerTask(void *pvParameters){
    server_init(); // Inicializa o servidor web
    while (1) {
        cyw43_arch_poll(); // Necessário para manter o Wi-Fi ativo  
        vTaskDelay(pdMS_TO_TICKS(100)); // Delay de 100 ms
    }
    cyw43_arch_deinit(); // Desativa o Wi-Fi ao finalizar a tarefa
}

int main()
{
    stdio_init_all();
    init_display(&ssd); // Inicializa o display OLED
    init_led_matrix(); // Inicializa a matriz de LEDs
    button_init_predefined(true, true, true); // Inicializa os botões
    button_setup_irq(true,true,true,button_callback); // Configura interrupções para os botões
    init_buzzer(BUZZER_A_PIN,4.0f); // Inicializa o buzzer com divisor de clock
    init_leds();// Inicializa os LEDs
    sleep_ms(2000);

    xMutexDisplay = xSemaphoreCreateMutex();
    xQueueSensorData = xQueueCreate(1, sizeof(SensorReadings)); // Cria a fila para leituras dos sensores

    printf("Iniciando o sistema...\n");

    xTaskCreate(vDisplayTask, "Display", 512, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vSensorTask, "Sensor", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vWebServerTask, "WebServer", 1024, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vLimitsTask,"Limits",512,NULL, tskIDLE_PRIORITY, NULL);

    vTaskStartScheduler();
    panic_unsupported();
}


void button_callback(uint gpio, uint32_t events){
    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    if(current_time - last_debounce_time > debounce_delay){

        if(gpio == BUTTON_A){
            alarm_disable=!alarm_disable;
            printf(alarm_disable ? "Alarme desabilitado\n" : "Alarme habilitado\n");
        }
        else if(gpio == BUTTON_B){
            reset_usb_boot(0, 0);
        }
        else if(gpio == BUTTON_SW){

        }

        last_debounce_time = current_time;
    }
}


bool handle_http_request(const char *request, size_t request_size, char *response, size_t response_size){
    SensorReadings readings;
    if (request_size >= 9 && strncmp(request, "GET /data", 9) == 0){
        //printf("Entrou IF\n");
        if (xQueuePeek(xQueueSensorData, &readings, 0) == pdTRUE) {
            snprintf(response, response_size, "{\"temp\":%.2f,\"hum\":%.2f,\"alt\":%.2f}", readings.bmp_temp, readings.humidity, readings.altitude_m);
            //printf("Dados atualizados.\n");
            return true;
        }
    }
    return false;
}
