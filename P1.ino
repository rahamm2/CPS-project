#include <DHT.h>
#include <DHT_U.h>
#include <WiFiEsp.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <AccelStepper.h>

// Define step constant
#define FULLSTEP 4
#define in1 39
#define in2 38
#define enA 5


int rotDirection = 0;
int pressed = false;
// Creates an instance
// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
AccelStepper myStepper(FULLSTEP, 8, 10, 9, 11);



//Define WiFi Access Point information
char ssid[] = "dreamdog";
char pass[] = "mkz981002";
int status = WL_IDLE_STATUS; //the Wifi radio's status

//Define Adafruit.io setup
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define AIO_USERNAME "Dreamdog"
#define AIO_KEY "aio_WfUs547tciiOp2blfcPYLWqKF3al"


// Path for sensors
#define TEMPERATURE_PUBLISH_PATH "api/feeds/temperature/data/send.json"
#define HUMIDITY_PUBLISH_PATH "api/feeds/humidity/data/send.json"


//WiFi client
WiFiEspClient client;

//Settup the MQTT client class by passing in the wifi client and MQTT server and login details
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT,AIO_USERNAME, AIO_KEY);

#define DHTPIN 2 // Analog Pin sensor is connected to
#define DHTTYPE    DHT11


//Declare the Adafruit IO feeds called Time, Humidity, and Temperature
Adafruit_MQTT_Publish input1 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME"/feeds/time"); 
Adafruit_MQTT_Publish input2 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME"/feeds/humidity"); 
Adafruit_MQTT_Publish input3 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME"/feeds/temperature"); 








DHT dht(DHTPIN, DHTTYPE);


void MQTT_connect();


void setup(){
 
  Serial.begin(115200);
  delay(500);//Delay to let system boot
  
  Serial1.begin (115200);

  WiFi.init(&Serial1);

  if(WiFi.status() == WL_NO_SHIELD){
    Serial.print("WiFi shield not presen");

    while(true);
    }

  if(status != WL_CONNECTED){
    Serial.print("Attempting to connect to WPA SSID:");
    Serial.println(ssid);
    
    status = WiFi.begin(ssid,pass);
    
    }


  Serial.println ("Connected to the network"); 

  Serial.println("DHT11 Humidity & temperature Sensor\n\n");
  delay(1000);//Wait before accessing Sensor
  dht.begin();
  

  // set the maximum speed, acceleration factor,
  // initial speed and the target position
  myStepper.setMaxSpeed(1000.0);
  myStepper.setAcceleration(50.0);
  myStepper.setSpeed(200);
  myStepper.moveTo(2038);



  //DC Motor
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
 
  // Set initial rotation direction
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);



 
}//end "setup()"
 
void loop(){
//Start of Program 
//Connecting MQTT server
MQTT_connect();



double    Hdata = dht.readHumidity();// take in the humidity measurements
double Tdata = dht.readTemperature();// take in the temperature measurements
   delay(200);
 

Serial.print(F("\nSending values "));

delay(2000);
if (! input2.publish(Hdata)) {
Serial.println(F("Failed"));
} else {
Serial.println(F("OK!"));
}


delay(2000);
if (! input3.publish(Tdata)) {
Serial.println(F("Failed"));
} else {
Serial.println(F("OK!"));
}


   



    
    Serial.print(Hdata);
    Serial.print("%  ");
    Serial.print(",");


    Serial.print(Tdata); 
    Serial.println("C  ");

  
if(Hdata<49){

  for(int i = 0; i<2000; i++){
  // Change direction once the motor reaches target position
  if (myStepper.distanceToGo() == 0) 
    myStepper.moveTo(-myStepper.currentPosition());

  // Move the motor one step
    myStepper.run();
   
   Serial.println("Water pump on");
    

    Serial.println(Hdata);
  }
   
   Hdata = dht.readHumidity();

   if (Hdata>49){
      Serial.println("Water pump off"); 
   }
 }

   
  int pwmOutput = 255; // Map the potentiometer value from 0 to 255
  analogWrite(enA, pwmOutput); // Send PWM signal to L298N Enable pin

while (Tdata>26.5){
 
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    rotDirection = 1;
     Serial.println(Tdata);
    Tdata = dht.readTemperature();
   if(Tdata<=26.5){
    digitalWrite(in1, LOW); 
    digitalWrite(in2, LOW);
    break;
   }
  }

    digitalWrite(in1, LOW); 
    digitalWrite(in2, LOW);
    


    
    //delay(1000);//Wait 1 second before accessing sensor again.
 
  //Fastest should be once every two seconds.
 
}// end loop(




void MQTT_connect() {
int8_t ret;
// Stop if already connected.
if (mqtt.connected()) {
return;
}
Serial.print("Connecting to MQTT... ");
uint8_t retries = 3;
while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
Serial.println(mqtt.connectErrorString(ret));
Serial.println("Retrying MQTT connection in 5 seconds...");
mqtt.disconnect();
delay(5); // wait 5 seconds
retries--;
mqtt.connect();
if (retries == 0) {
// basically die and wait for WDT to reset me
break;
//while (1);
}
}
Serial.println("MQTT Connected!");
}
