# 🌦️ ClimaConecta – Estação Meteorológica com Interface Web

Projeto desenvolvido como parte da Residência em Sistemas Embarcados na plataforma **BitDogLab**. A estação meteorológica permite monitorar, visualizar temperatura, umidade e pressão em tempo real por meio de uma interface web responsiva.

## 🔧 Funcionalidades

- 📡 Leitura contínua de sensores ambientais:
  - **Temperatura e Umidade:** Sensor AHT20
  - **Pressão atmosférica:** Sensor BMP280
- 📺 Exibição dos dados no display OLED SSD1306
- 🌐 Interface Web via Wi-Fi com:
  - Exibição dos valores em tempo real (AJAX/JSON)
  - Gráficos simples para cada variável (linha ou barras)
- 🚨 Alertas em tempo real:
  - **Buzzer** para sinalização sonora
  - **LED RGB** para indicar o estado do sistema
  - **Matriz de LEDs 8x8** para representar os níveis críticos
- 🔘 Botões físicos:
  - Modo de configuração local com debounce e interrupção

## ⚙️ Componentes Utilizados

| Componente         | Função                                 |
|--------------------|----------------------------------------|
| AHT20              | Medição de temperatura e umidade       |
| BMP280             | Medição de pressão atmosférica         |
| OLED SSD1306       | Exibição local das medições            |
| LED RGB            | Indicação visual de status             |
| Matriz de LEDs     | Representação visual de alertas        |
| Buzzer             | Alerta sonoro                          |
| Push Buttons       | Interação local com o sistema          |
| RP2040 (BitDogLab) | Microcontrolador principal             |

## 🧠 Lógica de Funcionamento

1. Leitura contínua dos sensores AHT20 e BMP280
2. Exibição local no display OLED
3. Envio dos dados via Wi-Fi para a interface web
4. Interface atualizada dinamicamente com JavaScript (AJAX/JSON)
5. Verificação de limites definidos:
   - Se ultrapassado, ativa LED RGB, matriz de LEDs e buzzer


## 🖥️ Interface Web

A interface web foi criada com HTML, CSS e JavaScript e possui:

- Design responsivo (compatível com celular e PC)
- Atualização automática dos dados
- Gráficos simples e intuitivos

