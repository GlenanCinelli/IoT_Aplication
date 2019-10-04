#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <Ticker.h>
#include "DHT.h"

// Set these to run example.
#define FIREBASE_HOST "SEU_FIREBASE_HOST"
#define FIREBASE_AUTH "SEU_FIREBASE_AUTH"
#define WIFI_SSID "NOME_DA_SUA_REDE_WIFI"
#define WIFI_PASSWORD "SENHA_DA_SUA_REDE_WIFI"

#define DHT_PIN D1
#define PRESENCE_PIN D2
#define BTN_1 D3
int estado_botao1 = 0;
int flag_botao1   = 0;
int estado_led1   = 1;



#define BTN_2 D4
#define LAMP_PIN D5
#define VENT_PIN D6


#define DHTTYPE DHT22

#define PUBLISH_INTERVAL 1000*60*1          // Publique a cada 1 min

DHT dht(DHT_PIN, DHTTYPE);
Ticker ticker;
bool publishNewState = true;

void publish(){
  publishNewState = true;
}

void setupPins(){

  pinMode(LAMP_PIN, OUTPUT);               // Define pino RELE LAMPADA como SAÍDA
  digitalWrite(LAMP_PIN, LOW);             // Inicia pino HIGH para desligar lâmpada

  pinMode(VENT_PIN, OUTPUT);               // Define pino RELE VENTILADOR como SAÍDA
  digitalWrite(VENT_PIN, LOW);             // Inicia pino HIGH para desligar ventilador
  
  pinMode(PRESENCE_PIN, INPUT);             // Define pino SENSOR DE PRESENÇA como ENTRADA
  
  pinMode(BTN_1, INPUT_PULLUP);                     // Define pino BOTAO 1 como ENTRADA
  pinMode(BTN_2, INPUT_PULLUP);                     // Define pino BOTAO 2 como ENTRADA
  
  dht.begin();                              // Inicia sensor de umidade e temperatura
  
}

void setupWifi(){
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando ");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Conectado: ");
  Serial.println(WiFi.localIP());
}

void setupFirebase(){
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.setBool("lamp", false);
  Firebase.setBool("vent", false);
  Firebase.setBool("presence", false);
}

void setup() {
  Serial.begin(9600);

  setupPins();
  setupWifi();    
  setupFirebase();
  ticker.attach_ms(PUBLISH_INTERVAL, publish);        // Registra o ticker para publicar de tempos em tempos
}

void loop() {
          
  float h = dht.readHumidity();                       // Reading temperature or humidity takes about 250 milliseconds!
  float t = dht.readTemperature();                    // Read temperature as Celsius (the default)
  float f = dht.readTemperature(true);                // Read temperature as Fahrenheit (isFahrenheit = true)

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Falha ao ler sensor DHT22!"));
    return;
  }
    
  float hif = dht.computeHeatIndex(f, h);             // Compute heat index in Fahrenheit (the default)
  float hic = dht.computeHeatIndex(t, h, false);      // Compute heat index in Celsius (isFahreheit = false)

  Serial.print(F("Umidade: "));
  Serial.print(h);
  Serial.print(F("%  Temperatura: "));
  Serial.print(t);
  Serial.println(F("°C "));

    if (t >= 30) {
    digitalWrite(VENT_PIN, LOW);
    Serial.println("Ventilador ligado.");
    }else{
      digitalWrite(VENT_PIN, HIGH);
      }
    

  // Apenas publique quando passar o tempo determinado
  if(publishNewState){
    Serial.println("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    Serial.println("+ Publicando NOVAS AFERIÇÕES de Temperatura e Umidade no Firebase +");
    Serial.println("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    
    // Obtem os dados do sensor DHT 
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    if(!isnan(humidity) && !isnan(temperature)){
      
      // Manda para o firebase
      Firebase.pushFloat("temperature", temperature);
      Firebase.pushFloat("humidity", humidity);
      publishNewState = false;
    }else{
      Serial.println("Erro ao publicar no Firebase.");
    }
  }

  // Verifica o valor do sensor de presença
  // LOW sem movimento
  // HIGH com movimento
  int presence = digitalRead(PRESENCE_PIN);  
  Firebase.setBool("presence", presence == HIGH);

  // Verifica o valor da lampada no firebase 
  bool lampValue = Firebase.getBool("lamp");

estado_botao1 = digitalRead(BTN_1);                       // ATRIBUI O PINO DE ENTRADA BOTAO A VARIAVEL estado_botao
  if(( estado_botao1 == 1 )&&( flag_botao1 == 0 ))           // TESTA A VARIALVEL SE É 1                    
    {
      flag_botao1 = 1;
      if( estado_led1 == 1)
        {
         estado_led1 = 0; 
         digitalWrite(LAMP_PIN, true);
         Serial.println("LIGADO");
        }
        
      else if( estado_led1 == 0)
        {
         estado_led1 = 1;
         digitalWrite(LAMP_PIN, false);
         Serial.println("DESLIGADO");   
        }
    delay(50);
    } 

  if( estado_botao1 == 0 )
    {
     flag_botao1 = 0;
     delay(50);  
    }


}

  /*
            if(digitalRead(BTN_1) == LOW){
            Serial.println("BOTÃO 1 PRESSIONADO");
            delay(10);
            digitalWrite(LAMP_PIN, !lampValue);
            Firebase.setBool("lamp", !Firebase.getBool("lamp"));
          }
  */ 
  Firebase.setBool("lamp", lampValue);
  digitalWrite(LAMP_PIN, lampValue ? HIGH : LOW);


  // Verifica o valor do ventilador no firebase 
  bool ventValue = Firebase.getBool("vent");
  digitalWrite(VENT_PIN, ventValue ? HIGH : LOW);

  delay(200);
}
