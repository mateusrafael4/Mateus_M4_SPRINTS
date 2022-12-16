// Importação de bibliotecas
#include <WiFi.h>
#include <FS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#define PERIODO_50HZ      20     // 20ms ou 1/50
#define MAXBITSRESOLUCAO  1024   // para resulucao de 10bits, o maximo valor do DUTY cycle
#define INICIODUTY_MS     0.9    // o ponto, em ms onde o angulo é -90
#define FIMDUTY_MS        2.5    // o ponto em ms onde o angulo é 90
#define FACTORDEFAULT     0.05   // menor pedaço de movimento em MS
#define CANALDEFAULT      0
#define FREQDEFAULT       50
#define RESOLUCAO         10
#define DELTADEFAULT      2
#define INICIODEFAULT     40
#define FIMDEFAULT        120
#define TEMPOPASSO        10  //50 //tempo para cada mudança de passo em ms

// Definição das credenciais para acesso a rede Wifi
const char* ssid = "Inteli-COLLEGE";
const char* password =  "QazWsx@123";
// Definição de variáveis auxiliares para as mensagens
const char* PARAM_INPUT = "value";
String estadoBomba = "0";
// Pinos dos LEDs dos jogadores
int jogador1 = 7;
int jogador2 = 6;
// Inicialização do servidor
AsyncWebServer server(80);
const char HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-br">
<meta charset="UTF-8"/>
<body>
    <h1> CONTROLE TV</h1>
    <div id="campo-jogo">
        <!-- Cria os botões que representam os campos do "Campo Minado" -->
        <button id="A" onclick="botaoA()">SOBE</button><br><br><br>
        <button id="B" onclick="botaoB()">DESCE</button>
    </div>
    <script>
        jogador = 1;
        // Função de processamento do comando enviado via XML HTTP Request
        function enviaComando(comando){
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/campo-minado?value="+comando, true);
            xhr.send();
        }
        function botaoA(){
            if (jogador == 1){
                jogador = 2;
                // Envia a informação sobre qual é o jogador da vez para o firmware acender o LED correspondente
                enviaComando(10);
            }
        }
        function botaoB(){
            if (jogador == 2){
                jogador = 1;
                // Envia a informação sobre qual é o jogador da vez para o firmware acender o LED correspondente
                enviaComando(20);
            }
        }
    </script>
</body>
</html>
)rawliteral";
class Servo9G{
  private:
    int canalServo = CANALDEFAULT;
    int freqServo = FREQDEFAULT;
    int resolucaoServo = RESOLUCAO;
    int pinSERVO = 16;
    int delta  = DELTADEFAULT;   // pedaços a cada fração de ms
    int inicio = INICIODEFAULT;  //==> Corresponde aos passos do ponto de 1ms
    int fim    = FIMDEFAULT; //==> Corresponde aos passos do ponto de 2ms
    int posAtual = INICIODEFAULT; // posicao (Em valor de DUTY cyle (ms) atual do motor)
    int tempoPasso = TEMPOPASSO;
    void calculaFatores(float factor){
      int inicioFactor = (float)INICIODUTY_MS/(float)factor;
      int fimFactor = (float)FIMDUTY_MS/(float)factor;
      delta = (int)((float)MAXBITSRESOLUCAO/(float)PERIODO_50HZ) * factor ;// pedaços por  de ms
      inicio = delta *inicioFactor ; //==> Corresponde a 1ms
      fim = delta * fimFactor;// ==> Corresponde a 2ms
      Serial.printf("Fatores calculados para o servo:\n");
      Serial.printf("delta=%i inicio=%i fim=%i canal=%i pino=%i factor=%f\n",delta,inicio,fim,canalServo,pinSERVO,factor);
    }
  public:
    Servo9G(int canal,int pino){
      canalServo = canal;
      pinSERVO = pino;
      pinMode(pinSERVO,OUTPUT);
      ledcAttachPin(pinSERVO, canalServo);
      ledcSetup(canalServo, freqServo, resolucaoServo);
      Serial.printf("Servo9G(%i, %i);\n",canalServo,pino);
    }
    void varre(int nrv,float factor=FACTORDEFAULT){
      calculaFatores(factor);
      posiciona(0);
      for(int j=0; j < nrv ; j++){
        for(int i = inicio ; i<fim; i+=delta){
          ledcWrite(canalServo, i);
          posAtual = i;
          delay(tempoPasso);
        }
        for(int i = fim ; i>inicio; i-=delta){
          ledcWrite(canalServo, i);
          posAtual = i;
          delay(tempoPasso);
        }
      }
    }
    void posiciona(int anguloPercentual){
      int dutyPos = inicio + ((float)(fim - inicio)) * (float)anguloPercentual / 100;
      if(dutyPos > posAtual){ // vai pra frente
        for(int j = posAtual ; j<=dutyPos; j+=delta){
          ledcWrite(canalServo, j);
          delay(tempoPasso);
        }
      }else{ // vai pra trás
        for(int j = posAtual ; j>=dutyPos; j-=delta){
          ledcWrite(canalServo, j);
          delay(tempoPasso);
        }
      }
      posAtual = dutyPos;
    }
};
Servo9G *servo1 = NULL;
void setup(){
    Serial.begin(115200);
    servo1 = new Servo9G(0,16);
    // Declara os pinos dos LEDs que representam os jogadores como output
    pinMode(jogador1, OUTPUT);
    pinMode(jogador2, OUTPUT);
    // Garante que os LEDs estejam desligados no início
    digitalWrite(jogador1, LOW);
    digitalWrite(jogador2, LOW);
    // Inicia a conexão com o Wifi
    WiFi.begin(ssid, password);
    // Exibe esta mensagem enquanto o ESP32 não estiver conectado a rede
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Conectando ao Wifi");
    }
    // Exibe IP do ESP32
    Serial.println(WiFi.localIP());
    // Definição dos tipos de request
    server.on("/html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", HTML);
    });
    server.on("/campo-minado", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String informacaoBomba;
    // Se há um tipo de request...
    if (request->hasParam(PARAM_INPUT)) {
        // Consegue a informação do estado da bomba e/ou do jogador
        informacaoBomba = request->getParam(PARAM_INPUT)->value();
        estadoBomba = informacaoBomba;
        // Controla os LEDs de acordo com o valor recebido
        if (estadoBomba == "10"){
            digitalWrite(jogador1, HIGH);
            digitalWrite(jogador2, LOW);
            servo1->posiciona(0);
        } else if (estadoBomba == "20"){
            digitalWrite(jogador2, HIGH);
            digitalWrite(jogador1, LOW);
            servo1->posiciona(50);
        } else if (estadoBomba == "1"){
            digitalWrite(jogador2, LOW);
            // Pisca LED
            for (int i = 0; i <= 20; i++){
                digitalWrite(jogador1, HIGH);
                delay(500);
                digitalWrite(jogador1, LOW);
                delay(500);
            }
        } else if (estadoBomba == "2"){
            digitalWrite(jogador1, LOW);
        for (int i = 0; i <= 20; i++){
            digitalWrite(jogador2, HIGH);
            delay(500);
            digitalWrite(jogador2, LOW);
            delay(500);
        }
        } else if (estadoBomba == "0"){
            digitalWrite(jogador1, LOW);
            digitalWrite(jogador2, LOW);
        }
    }
    else {
        informacaoBomba = "Nenhuma requisição foi feita.";
    }
    request->send(200, "text/plain", "OK");
    });
    // Inicialização
    server.begin();
}
void loop(){
}