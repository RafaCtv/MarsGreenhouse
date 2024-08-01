#include <Wire.h>
//Definindo id deste arduino
const int slave_id = 0x2;

#include <Servo.h>
//Definindo o servo contínuo
Servo myservocont;
//definindo o servo posicional
Servo myservopos;

//Códigos de cada tipo de leitura
const char code_gaz = 'g';
const char code_irrigation = 'i';

//Variáveis referentes aos dados lidos
int gaz;
int irrigation;

//Controle das execuções
bool stand_by = true;

//Variáveis referentes pinos
const int LED = 2;
const int LED_RGB_vermelho = 5;
const int LED_RGB_verde = 4;
const int PIN_GAZ = A0; //O sensor de gás
const int slavcont = 9; //Servo contínuo
const int slavpos = 10; //Servo posicional
const int MOTOR = 3; // Motor irrigação para umidade

void setup() {
  //iniciando ponte de conexão do slave com seu id
  Wire.begin(slave_id);
  Serial.begin(9600);
  //Passando a função que vai ser executada quando receber algum dado
  Wire.onReceive(receive);
  //Passando a função que vai ser executada quando receber uma req dos
  //seus dados
  Wire.onRequest(requestEvent);
  pinMode(LED, OUTPUT);
  pinMode(MOTOR, OUTPUT);
  pinMode(LED_RGB_verde, OUTPUT);
  pinMode(LED_RGB_vermelho, OUTPUT);
  //anexa a variável do servo contínuo em um PIN
  myservocont.attach(slavcont);
  //anexa a variável do servo posicional em um PIN
  myservopos.attach(slavpos);
  //Iniciando suas variáveis
  gaz = 0;
  irrigation = 0;
  myservocont.write(90);
  myservopos.write(90);
}

void loop(){
  //Recebe do master se é para executar as leituras ou não
  if(stand_by) return;
  read_gaz(); //chama a função que faz a leitura do gás
  read_irrigation();//chama a função que faz a leitura de irrigação
  delay(4000);
}

void read_gaz(){
  //recebe as informações do sensor de gás
  int read_gaz = analogRead(PIN_GAZ);
  delay(100);
  Serial.print(code_gaz);
  Serial.println(read_gaz);
  delay(100);
  //converte essas informações para números de 0 a 100
  gaz = map(read_gaz,300,723,0,100);
  spin_slave(); //chama a função que faz o servo girar
  open_close(); //chama a função que faz o servo abrir
}

void spin_slave(){
  //se o nível de gás for maior ou igual a 15
  if(gaz>=40){
	//quando definido em 0 ou 180, gira em velocidade máxima
      myservocont.write(0);
  } else {
      //quando definido em 90, gira em velocidade reduzida
	myservocont.write(90);
  }
}

void open_close(){
  //Se o nível de gás for menor que 5, libera uma capsula de gás
  if(gaz<5){
    myservopos.write(90);
  } else{
    myservopos.write(0);
  }
}

void read_irrigation()
{
  int percent =  analogRead(A1);
  // equação para redefinir os valores do potenciômetro
  irrigation = map(percent, 0, 1023, 0, 100);
 
  // caso a umidade percentual do solo seja menor que 20%
  // o irrigador é acionado juntamente com o led verde
  if(irrigation < 20){
    digitalWrite(LED_RGB_vermelho, HIGH);
    digitalWrite(LED_RGB_verde, LOW);
    digitalWrite(MOTOR,1);
    Serial.print(code_irrigation);
    Serial.println(irrigation);
    delay(20);
  }
  // do contrário ele permanece desligado com o led vermelho
  else{
    digitalWrite(LED_RGB_verde,HIGH);
    digitalWrite(LED_RGB_vermelho,LOW);
    digitalWrite(MOTOR,0);
    delay(20);
  }
}

//Recebe do master se o arduino vai fazer as leituras ou não
void receive(int bytes){
  if(Wire.available()) {
	bool status = Wire.read();
	stand_by = status;
	digitalWrite(LED, !status);
  }
}


//Quando receber uma req de seus dados essa função irá executar
void requestEvent() {
  //Enviar o código referente ao gás
  Wire.write(code_gaz);
  //Enviar o dado do gás
  Wire.write(gaz);

  //Enviar o código referente a irrigação
  Wire.write(code_irrigation);
  //Enviar o dado da irrigação
  Wire.write(irrigation);
 
}