#include <Arduino.h>
#include <Servo.h>
#include <WiFi.h>
#include <PubSubClient.h>


//************************
//**** P I N E S *********
//************************

//MOTOR Izq
#define pin_ena1  27
#define pin_n1    14
#define pin_n2    4

//MOTOR Der
#define pin_n3    2
#define pin_n4    32
#define pin_ena2  33

//SERVO
#define pin_servo 13

//HC-SR04
#define pin_trig  26
#define pin_echo  25

//IMU MPU92.65
#define pin_scl   22
#define pin_sda   21

//************************
//**** C O N F I G *******
//************************

//WIFI
const char* ssid = "IOTICOS";
const char* password =  "pingpong";


//MQTT
const char *mqtt_server = "ioticos.org";
const int mqtt_port = 1883;
const char *mqtt_user = "0AJDqCpuJnr3VwG";
const char *mqtt_pass = "hIDxo6MJeZeOVAC";
const String root_topic = "wBdfeDSE8C1zFW6";

//SERVO
#define servo_max_ang   180
#define servo_min_ang   0
<<<<<<< HEAD
#define servo_speed     20 
=======
#define servo_speed     25 
>>>>>>> 3b4cc612c4868e20c6d5f8a88387398f32cab5f4
#define servo_step      3


/*
// Set your Static IP address
IPAddress local_IP(192, 168, 100, 33);
// Set your Gateway IP address
IPAddress gateway(192, 168, 100, 254);

IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional
*/


//************************
//** V A R I A B L E S ***
//************************


Servo servo;                // se declara instancia de servo
int pos = 0;                // guardaremos la posición actual del servo
int dist_map[(servo_max_ang-servo_min_ang)/servo_step];  //creamos array mapa para guardar una distancia por ángulo

//MQTT
WiFiClient espClient;
PubSubClient client(espClient);

//MENSAJERIA
String tosend = "";
char buf[20];


//************************
//** F U N C I O N E S ***
//************************

int distance();
void setup_wifi();
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();
void movement(String side); //side: left,right,straight,back

//_________________________________________________________________





//******************
//*** S E T U P  ***
//******************
void setup() {
  Serial.begin(115200);

  //PINMODES
  //hc-sr04
  pinMode(pin_trig,OUTPUT);
  pinMode(pin_echo,INPUT);

  //motor izq
  pinMode(pin_ena1,OUTPUT);
  pinMode(pin_n1,OUTPUT);
  pinMode(pin_n2,OUTPUT);

  //motor der
  pinMode(pin_ena2,OUTPUT);
  pinMode(pin_n3,OUTPUT);
  pinMode(pin_n4,OUTPUT);

  digitalWrite(pin_ena1,HIGH);
  digitalWrite(pin_ena2,HIGH);

  digitalWrite(pin_echo,LOW);

  servo.attach(pin_servo);  // attaches the servo on pin 13 to the servo object

  randomSeed(micros());
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);


}



//******************
//*** L O O P 1  ***
//******************
void loop() {

  if (!client.connected()) {
		reconnect();
	}

    delay(300);
    for(int posDegrees = servo_min_ang; posDegrees <= servo_max_ang; posDegrees+=servo_step) {
      if (posDegrees>24){
        servo.write(180-posDegrees);
      }
 
      delay(servo_speed);
      tosend = String(distance()) + "," + String(posDegrees);
      tosend.toCharArray(buf, 20);
      client.publish("wBdfeDSE8C1zFW6/map", buf);
      client.loop();
    }
     servo.write(155);
    

    

}

  int distance(){
    digitalWrite(pin_trig,HIGH);
    delayMicroseconds(10);
    digitalWrite(pin_trig,LOW);

    long time = pulseIn(pin_echo,HIGH);
    int distance = time / 58;
    return distance;
  }

void movement(String side){

  if (side == "straight"){
      digitalWrite(pin_n1,LOW);
      digitalWrite(pin_n2,HIGH);
      digitalWrite(pin_n3,HIGH);
      digitalWrite(pin_n4,LOW);
  }

  if (side == "back"){
      digitalWrite(pin_n1,HIGH);
      digitalWrite(pin_n2,LOW);
      digitalWrite(pin_n3,LOW);
      digitalWrite(pin_n4,HIGH);
  }

  if (side == "left"){
      digitalWrite(pin_n1,HIGH);
      digitalWrite(pin_n2,LOW);
      digitalWrite(pin_n3,HIGH);
      digitalWrite(pin_n4,LOW);
  }

  if (side == "right"){
      digitalWrite(pin_n1,LOW);
      digitalWrite(pin_n2,HIGH);
      digitalWrite(pin_n3,LOW);
      digitalWrite(pin_n4,HIGH);
  }

  if (side == "stop"){
      digitalWrite(pin_n1,LOW);
      digitalWrite(pin_n2,LOW);
      digitalWrite(pin_n3,LOW);
      digitalWrite(pin_n4,LOW);
  }

}

//*****************************
//***    CONEXION WIFI      ***
//*****************************
void setup_wifi(){
	delay(10);
	// Nos conectamos a nuestra red Wifi
	Serial.println();
	Serial.print("Conectando a ");
	Serial.println(ssid);

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.println("Conectado a red WiFi!");
	Serial.println("Dirección IP: ");
	Serial.println(WiFi.localIP());
}


void callback(char* topic, byte* payload, unsigned int length){
	String incoming = "";
	Serial.print("Mensaje recibido desde -> ");
	Serial.print(topic);
	Serial.println("");
	for (int i = 0; i < length; i++) {
		incoming += (char)payload[i];
	}
	incoming.trim();
	Serial.println("Mensaje -> " + incoming);

  String topic_str(topic);

	if(topic_str == root_topic + "/movement"){
    movement(incoming);
  }
}

void reconnect() {

	while (!client.connected()) {

		Serial.print("Intentando conexión Mqtt...");
		// Creamos un cliente ID
		String clientId = "ALAN_";
		clientId += String(random(0xffff), HEX);
		// Intentamos conectar
		if (client.connect(clientId.c_str(),mqtt_user,mqtt_pass)) {
			Serial.println("Conectado!");
			// Nos suscribimos
			if(client.subscribe("wBdfeDSE8C1zFW6/movement")){
        Serial.println("Suscripcion ok");
      }else{
        Serial.println("fallo Suscripciión");
      }
		} else {
			Serial.print("falló :( con error -> ");
			Serial.print(client.state());
			Serial.println(" Intentamos de nuevo en 5 segundos");

			delay(5000);
		}
	}
}
