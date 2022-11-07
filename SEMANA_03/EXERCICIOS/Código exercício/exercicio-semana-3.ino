// Inserindo os componentes e determinando as portas
int buzzer = 46;
int ldr = 4;
int button1 = 20;
int button2 = 21;
int i = 3;
int n = -1;
int vector[128]; // Vetor de armazenamento das informações
// Definindo as séries: leds e binário que será usado na conversão
int leds[4] = {15, 7, 6, 5};



void setup() {
  pinMode(leds[0], OUTPUT);
  pinMode(leds[1], OUTPUT);
  pinMode(leds[2], OUTPUT);
  pinMode(leds[3], OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(ldr, INPUT);

  Serial.begin(9600);
}

void loop() {
  // loop para apagar todos os leds antes de começar.
  for (int i = 0; i < 4; i++){
    digitalWrite(leds[i], LOW);
  }
  // gerando os botões para futuramente fazer a leitura, gravação e exclusão dos dados no vetor
  int buttonStatus1 = digitalRead(button1);
  int buttonStatus2 = digitalRead(button2);
  int size = sizeof(vector)/sizeof(int);

  // analisa quando o usuário pressiona o primeiro botão
  if (buttonStatus1 == HIGH){
    n += 1;

    Serial.println("Botão APERTADO");
   // Set do array para armazenar os dados em binário;
   // funções subsequentes são usadas para ler o ldr e converter o valor para valores de 0 a 15 e tocar o buzzer 
    int binary[4] = {0, 0, 0, 0};
    int ldrStatus = analogRead(ldr);
    int result = floor(ldrStatus/256);
    tone(buzzer, 1000 + (result*100), 300);
    
    // Armazena os valores dentro do vetor
    Serial.println(result);
    vector[n] = result;
    
    // Cálculo de conversão do valor para binário
    while (result > 0) {
      binary[i] = result % 2;
      result = result / 2;
      i--;
    }
    i = 3;
    
    // acende os leds de acordo com o resultado do calculo acima
    digitalWrite(leds[0], binary[0]); 
    digitalWrite(leds[1], binary[1]);
    digitalWrite(leds[2], binary[2]);
    digitalWrite(leds[3], binary[3]);

    delay(100);

  }

  // Analisa quando o usuário aperta o segundo botão;
  // loop que anda pelos valores armazenados no vetor, diferentes de 0;
    else if (buttonStatus2 == HIGH) {
    Serial.println("Botão NÃO APERTADO");
    for (int e = 0; e< size; e++) {
      if (vector[e] != 0){
        int binary[4] = {0, 0, 0, 0};
        int result = vector[e];
        tone(buzzer, 1000 + (result*100), 300);

        // Refaz a conversão para binário
        while (result > 0) {
          binary[i] = result % 2;
          result = result / 2;
          i--;
        }
        i = 3;
        // Acende os leds de acordo com o resultado do cálculo acima
        digitalWrite(leds[0], binary[0]); 
        digitalWrite(leds[1], binary[1]);
        digitalWrite(leds[2], binary[2]);
        digitalWrite(leds[3], binary[3]);

        delay(300);
        // Usado para confirmar que os leds estão apagados antes de acenderem de novo
        for (int i = 0; i < 4; i++){
          digitalWrite(leds[i], LOW);
        }
        // Printa o valor salvo no vetor
        int val = vector[e];
        Serial.println(val);
      }
    }
    delay(100);
    // Apaga os dados do vetor
    for (int i = 0; i< size; i++) {
      vector[i] = 0;
    }
  }
  delay(300);
}
