#include "webserver.h"

const char HTML_DASHBOARD[] =
"<!DOCTYPE html>\n"
"<html lang=\"pt-BR\">\n"
"<head>\n"
"  <meta charset=\"UTF-8\">\n"
"  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
"  <title>Dashboard Climático</title>\n"
"  <script src=\"https://cdn.jsdelivr.net/npm/chart.js\"></script>\n"
"  <style>\n"
"    * { margin: 0; padding: 0; box-sizing: border-box; }\n"
"    body { font-family: 'Segoe UI', sans-serif; background: #121212; color: #f0f0f0; padding: 20px; }\n"
"    header { border-bottom: 2px solid #00ff88; padding: 10px 0; text-align: center; margin-bottom: 20px; }\n"
"    header h1 { font-size: 1.6rem; color: #00ff88; }\n"
"    .alert { display: none; background: #ff4444; color: white; padding: 12px; text-align: center; border-radius: 5px; margin-bottom: 20px; }\n"
"    .alert.active { display: block; }\n"
"    .alert button { background: #222; color: #ff4444; border: 1px solid #ff8888; padding: 8px 12px; cursor: pointer; margin-top: 10px; border-radius: 3px; }\n"
"    .cards { display: flex; flex-wrap: wrap; gap: 15px; justify-content: center; }\n"
"    .card { background: #1e1e1e; padding: 16px; border-radius: 8px; border: 1px solid #333; width: 280px; text-align: center; box-shadow: 0 0 10px #00ff8855; }\n"
"    .card h3 { color: #ffee00; margin-bottom: 8px; font-size: 1.2rem; }\n"
"    .value { font-size: 1.8rem; font-weight: bold; color: #00ffaa; margin: 10px 0; }\n"
"    .chart { height: 120px; margin-top: 12px; }\n"
"    .status { text-align: center; margin-top: 20px; font-size: 0.9rem; color: #888; }\n"
"    .online { color: #00ff88; }\n"
"    .offline { color: #ff4444; }\n"
"    @media (max-width: 600px) { .card { width: 100%; } }\n"
"  </style>\n"
"</head>\n"
"<body>\n"
"  <header>\n"
"    <h1>📡 Dashboard Climático - RESTIC 37</h1>\n"
"  </header>\n"
"\n"
"  <div class=\"alert\" id=\"alert\">\n"
"    🚨 Alerta! Valores fora do padrão.\n"
"    <br><button onclick=\"stopAlarm()\">Desativar Alarme</button>\n"
"  </div>\n"
"\n"
"  <div class=\"cards\">\n"
"    <div class=\"card\">\n"
"      <h3>Temperatura</h3>\n"
"      <div class=\"value\" id=\"temp\">--°C</div>\n"
"      <div class=\"chart\"><canvas id=\"tempChart\"></canvas></div>\n"
"    </div>\n"
"    <div class=\"card\">\n"
"      <h3>Umidade</h3>\n"
"      <div class=\"value\" id=\"humidity\">--%</div>\n"
"      <div class=\"chart\"><canvas id=\"humidityChart\"></canvas></div>\n"
"    </div>\n"
"    <div class=\"card\">\n"
"      <h3>Altitude</h3>\n"
"      <div class=\"value\" id=\"altitude\">--m</div>\n"
"      <div class=\"chart\"><canvas id=\"altitudeChart\"></canvas></div>\n"
"    </div>\n"
"  </div>\n"
"\n"
"  <div class=\"status\">\n"
"    <div id=\"update\">Última atualização: --:--</div>\n"
"    <div id=\"status\" class=\"offline\">● Status desconhecido</div>\n"
"  </div>\n"
"\n"
"  <script>\n"
"    const chartConfig = {\n"
"      type: 'line',\n"
"      options: {\n"
"        responsive: true,\n"
"        maintainAspectRatio: false,\n"
"        plugins: { legend: { display: false } },\n"
"        scales: { x: { display: false }, y: { beginAtZero: false } },\n"
"        elements: { line: { tension: 0.3 }, point: { radius: 2 } }\n"
"      }\n"
"    };\n"
"\n"
"    const tempChart = new Chart(document.getElementById('tempChart'), {\n"
"      ...chartConfig,\n"
"      data: { labels: [], datasets: [{ data: [], borderColor: '#ffbb33', backgroundColor: 'rgba(255,187,51,0.2)', fill: true }] }\n"
"    });\n"
"\n"
"    const humidityChart = new Chart(document.getElementById('humidityChart'), {\n"
"      ...chartConfig,\n"
"      data: { labels: [], datasets: [{ data: [], borderColor: '#33ccff', backgroundColor: 'rgba(51,204,255,0.2)', fill: true }] }\n"
"    });\n"
"\n"
"    const altitudeChart = new Chart(document.getElementById('altitudeChart'), {\n"
"      ...chartConfig,\n"
"      data: { labels: [], datasets: [{ data: [], borderColor: '#99ff99', backgroundColor: 'rgba(153,255,153,0.2)', fill: true }] }\n"
"    });\n"
"\n"
"    function updateChart(chart, value) {\n"
"      const time = new Date().toLocaleTimeString();\n"
"      if (chart.data.labels.length >= 10) {\n"
"        chart.data.labels.shift();\n"
"        chart.data.datasets[0].data.shift();\n"
"      }\n"
"      chart.data.labels.push(time);\n"
"      chart.data.datasets[0].data.push(value);\n"
"      chart.update('none');\n"
"    }\n"
"\n"
"    function stopAlarm() {\n"
"      fetch('/stop', { method: 'POST' });\n"
"    }\n"
"\n"
"    function updateData() {\n"
"      fetch('/data')\n"
"        .then(res => res.json())\n"
"        .then(data => {\n"
"          document.getElementById('temp').textContent = data.temp.toFixed(1) + '°C';\n"
"          document.getElementById('humidity').textContent = data.hum.toFixed(1) + '%';\n"
"          document.getElementById('altitude').textContent = data.alt.toFixed(1) + 'm';\n"
"\n"
"          updateChart(tempChart, data.temp);\n"
"          updateChart(humidityChart, data.hum);\n"
"          updateChart(altitudeChart, data.alt);\n"
"\n"
"          document.getElementById('update').textContent = 'Última atualização: ' + new Date().toLocaleTimeString();\n"
"          document.getElementById('status').className = 'online';\n"
"          document.getElementById('status').textContent = '● Conectado';\n"
"        })\n"
"        .catch(() => {\n"
"          document.getElementById('status').className = 'offline';\n"
"          document.getElementById('status').textContent = '● Desconectado';\n"
"        });\n"
"    }\n"
"\n"
"    function checkAlarm() {\n"
"      fetch('/alarm')\n"
"        .then(res => res.json())\n"
"        .then(data => {\n"
"          document.getElementById('alert').classList.toggle('active', data.alert === \"1\");\n"
"        })\n"
"        .catch(console.error);\n"
"    }\n"
"\n"
"    updateData();\n"
"    checkAlarm();\n"
"    setInterval(updateData, 2000);\n"
"    setInterval(checkAlarm, 2000);\n"
"  </script>\n"
"</body>\n"
"</html>\n";


// Função de callback para receber dados TCP
static err_t http_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (!p)
    {
        tcp_close(tpcb);
        return ERR_OK;
    }

    char *req = (char *)p->payload;

    const char *header_html =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "\r\n";

    const char *header_json =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: close\r\n"
        "\r\n";

    char json[64];

    if (p->len >= 6 && strncmp(req, "GET / ", 6) == 0){
        tcp_write(tpcb, header_html, strlen(header_html), TCP_WRITE_FLAG_COPY);
        tcp_write(tpcb, HTML_DASHBOARD, strlen(HTML_DASHBOARD), TCP_WRITE_FLAG_COPY);
    }else if(handle_http_request(req, p->len, json, sizeof(json))) {
        tcp_write(tpcb, header_json, strlen(header_json), TCP_WRITE_FLAG_COPY);
        tcp_write(tpcb, json, strlen(json), TCP_WRITE_FLAG_COPY);
    } else {
        const char *not_found =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/plain\r\n"
            "Connection: close\r\n"
            "\r\n"
            "Recurso não encontrado.";
        tcp_write(tpcb, not_found, strlen(not_found), TCP_WRITE_FLAG_COPY);
    }
    tcp_recved(tpcb, p->tot_len);
    pbuf_free(p);
    tcp_close(tpcb); // Fecha a conexão após envio da resposta
    return ERR_OK;
}

// Função de callback ao aceitar conexões TCP
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err){
    tcp_recv(newpcb, http_recv);
    return ERR_OK;
}

// Função para inicializar o servidor TCP
void server_init(void) {
    //Inicializa a arquitetura do cyw43
    if (cyw43_arch_init()) {
        panic("Failed to initialize CYW43");
    }

    // GPIO do CI CYW43 em nível baixo
    cyw43_arch_gpio_put(LED_PIN, 1);

    // Ativa o Wi-Fi no modo Station, de modo a que possam ser feitas ligações a outros pontos de acesso Wi-Fi.
    cyw43_arch_enable_sta_mode();

    // Conectar à rede WiFI 
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        panic("Failed to connect to WiFi");
    }

    // Caso seja a interface de rede padrão - imprimir o IP do dispositivo.
    if (netif_default){
        printf("IP do dispositivo: %s\n", ipaddr_ntoa(&netif_default->ip_addr));
    }

    // Configura o servidor TCP - cria novos PCBs TCP. É o primeiro passo para estabelecer uma conexão TCP.
    struct tcp_pcb *server = tcp_new();
    if (!server){
        panic("Failed to create TCP PCB\n");
    }

    //vincula um PCB (Protocol Control Block) TCP a um endereço IP e porta específicos.
    if (tcp_bind(server, IP_ADDR_ANY, 80) != ERR_OK){
        panic("Failed to bind TCP PCB\n");
    }

    // Coloca um PCB (Protocol Control Block) TCP em modo de escuta, permitindo que ele aceite conexões de entrada.
    server = tcp_listen(server);

    // Define uma função de callback para aceitar conexões TCP de entrada. É um passo importante na configuração de servidores TCP.
    tcp_accept(server, tcp_server_accept);
    printf("Servidor ouvindo na porta 80\n");
}