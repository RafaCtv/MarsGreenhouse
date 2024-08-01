#include <Wire.h>
#include <LiquidCrystal.h>

//Valores das entrada para o display
const int rs = 12;
const int en = 11;
const int d4 = 5;
const int d5 = 4;
const int d6 = 3;
const int d7 = 2;
//Iniciando o display
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

typedef struct {
      String max;
	String min;
	String standard;
} Messages;

//Classe responsável por definir os atributos de cada sensor
class Sensor{
  //Definido como públicas
  public:
 
  //Atributos dos sensores
  char key;   	 //chave de identificação
  int value;    //valor a ser exibido
  int max;   	 //seu máximo esperado
  int min;   	 // seu valor mínimo esperado
  char unit;     //unidade do valor
  String slug;    //nome do valor
  bool bips;    //irá apitar quando exceder seus extremos
  Messages messages;    //mensagens de exibição
 
  //Construtor quando for usar as mensagens para o usuário padrão
  Sensor(char _key, int _min,int _max, char _unit,
    String _slug, bool _bips){
	key = _key;
	max = _max;
	min = _min;
	unit = _unit;
	slug = _slug;
	value = 0;
	bips = _bips;
	messages.standard = "Normal";
	messages.min = "Abaixo do Normal";
	messages.max = "Acima do normal";
  }
  //Construtor quando for usar as mensagens para o usuário
  //personalizadas
  Sensor(char _key, int _min,int _max, char _unit,
    String _slug, bool _bips, String S_standard,
    String S_min, String S_max){
	key = _key;
  	max = _max;
	min = _min;
	unit = _unit;
	slug = _slug;
	value = 0;
	bips = _bips;
	messages.standard = S_standard;
	messages.min = S_min;
	messages.max = S_max;
  }
 
  //Responsável por alterar o valor e verificar se ele está ok
  public:
  void update(int _value){
	value = _value > 200 ? _value -256 : _value;
	//Limpo  o display
	lcd.clear();
	//Cursor na primeira linha e coluna
	lcd.setCursor(0, 0);
	//Printar o slug do dado
	lcd.print(slug);
	//Printar o : e espaço
	lcd.print(": ");
	//O valor do sensor
	lcd.print(value);
	//Sua unidade de medida
	lcd.print(unit);
    
	//Posicionar o sensor em uma segunda linha
	lcd.setCursor(0, 1);
	//Se o valor for menor igual ao mínimo
	if(value <= min){
  	lcd.print(messages.min);
 	 
	//Se o valor for maior igual ao máximo
	}else if(value >= max) {
  		lcd.print(messages.max);
	} else {
  		//caso esteja no intervalo padrão
  		lcd.print(messages.standard);
	}
    
	delay(2000);
  }
};

//Variáveis referentes aos dados lidos
Sensor temperature('t', 20, 50, 'C', "Tem", true);
Sensor gaz('g', 40, 101, '%', "Gaz", true);
Sensor irrigation('i', 20, 100,'%', "Umi", true,
             	"Umidade Normal", "Umidade Baixa", "");
Sensor luminosity('l', 30, 60, '%', "Luz", false,
              	"Luz Media", "Luz Fraca", "Luz Forte");

//Definindo o id de cada um dos arduinos
const int slave_1 = 0x1;
const int slave_2 = 0x2;
int status;

void setup() {
    //iniciando ponte de conexão do master
    Wire.begin();
    //Dimensões do Display
    lcd.begin(16, 2);
    lcd.print("Estufa Automatica");
    lcd.setCursor(0, 2);
	lcd.print("Iniciando...");
 
      //Tempo do Serial
    Serial.begin(9600);
      
   	//inicializando variáveis
  	status = true;
 
  	sendInfo(status);
  	delay(1000);
  	sendInfo(!status);
  	delay(1000);
  	sendInfo(status);
}

void loop() {
 
  //Executar as rotinas para cada arduino
  routines(slave_1);
  routines(slave_2);
 
}

//Executar as rotinas de leitura e apresentar os dados
void routines(int slave){
 //Fazendo a requisição dos dados do arduino 1
  receiveInfo(slave);
  sendInfo(!status);
  //Mostrando no display esses dados
  status = !status;
}

//Função que faz a requisição dos dados do ide do arduino
//passado por parâmetro
void receiveInfo(int slave){
  //Faz a requisição de 4 bits
  Wire.requestFrom(slave, 4);  
  //Enquanto existem bytes vindo
  while(Wire.available()) {
	//Primeiro byte relacionado ao código da informação passada
	char code = Wire.read();
	//Segundo o dado em si
	int payload = Wire.read();
	//Chama actions para salvar o valor na sua determinada
	//variável
	actions(code, payload);
  }
  //Encerra a conexão
  Wire.endTransmission();
}

//Enviar qual arduino estara fazendo a leitura
void sendInfo(bool status){
  //Passa o status para primeiro arduino
  Wire.beginTransmission(slave_1);
  Wire.write(status);
  Wire.endTransmission();
  //Passa o status negado para o segundo arduino
  Wire.beginTransmission(slave_2);
  Wire.write(!status);
  Wire.endTransmission();
}

//Recebe o code e a dado para salvar na variável correspondente
void actions(char code, int payload){

  switch(code){
  	case 't':
    	temperature.update(payload);
    	break;
    case 'g':
    	gaz.update(payload);
    	break;
    case 'i':
    	irrigation.update(payload);
    	break;
    case 'l':
    	luminosity.update(payload);
    	break;
  }
}