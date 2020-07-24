//Navn: Simon Edna
//Dette er koden for vaktpistonen min. Alle kommentarene/forklaringene underveis skal være på norsk, men alle variabler og kode skal være i engelsk.

//Her er de to "libary'ene" jeg brukte
#include <Servo.h> 
#include <SoftwareSerial.h>

//Definerer en haug med I/O porter
const int rxPin = 13;
const int txPin = 12;
const int laser = 10;
const int stepPin = 9; 
const int dirPin = 8;
const int stepPinGun = 7; 
const int dirPinGun = 6;
const int triggerPin = 5;
const int fireMotors = 4;

Servo triggerServo; //Setter opp en servo
SoftwareSerial arduinoCOM =  SoftwareSerial(rxPin, txPin); //Setter opp en Serial-port, for kommunikasjon mellom de to Arduino kortene

//Definerer noen "char's"
const byte numChars = 32;     //Dette er hvor mange bytes de to variablene nedenfor får lov til å bruke.
char receivedChars[numChars]; //Char som datapakken setter sammen i    
char tempChars[numChars];     //Char der man midlertidig lagrer en kopi av den motatte datapakken

//Definerer noen flere variabler;
int xVal = 0;
int yVal = 0;
int btnState2 = 0;
int btnState3 = 0;
boolean newData = false;
char ask = 'A';

void setup() {
    //Definerer I/O-portene som Input eller Output
    pinMode(rxPin, INPUT);
    pinMode(txPin, OUTPUT);
    pinMode(laser, OUTPUT);
    pinMode(stepPin,OUTPUT); 
    pinMode(dirPin,OUTPUT);
    pinMode(stepPinGun,OUTPUT); 
    pinMode(dirPinGun,OUTPUT);
    pinMode(fireMotors,OUTPUT);

    //Velger hvilke I/O-port som skal styre servoen
    triggerServo.attach(triggerPin); 

    //Åpner begge Serial-portene
    arduinoCOM.begin(2400);
    Serial.begin(115200);

    digitalWrite(laser, HIGH); //Skrur på laser
    digitalWrite(fireMotors, HIGH); //Skrur på Relayen i Nerf-geværet, som gjør at geværet er skrudd av
}



void loop() {
    delay(50);  //Denne delayen gjør at vaktpistolen maksimalt ber om 20 datapakker fra kontrolleren i sekundet.
    arduinoCOM.println(ask); //Her spør vaktpistolen om å få data fra kontrolleren. Det åpner for at vaktpistolen kan få langt færre datapakker per sekund, alt etter behov.
    
    //Funksjonen "recvWithStartEndMarkers" er en funksjon som setter sammen data som strømmer gjennom Serial porten mellom de to Arduino kortene.
    //Dette er viktig, fordi at dataen blir sendt byte for byte, f.ex ville det IKKE bli sent som "Hello", men som "H", "e", "l", "l" og "o".
    recvWithStartEndMarkers(); //Starter med å samle og sette sammen innkommende data
    
    if (newData == true) {
        strcpy(tempChars, receivedChars);   //Lager en kopi av datapakken som nettopp ble motatt. Kopien får navnet "tempChars"
        parseData();        //Begynner å splitte opp datapakken
        showParsedData();
        newData = false;
    }
    
    if(btnState2 == 2){
      Serial.println("FIRE");
      fire();
    }

    //Bruker xVal-verdien til å finne ut om våpenet skal begeves opp, ned eller stå i ro. (hvis -20<xVal<20 så vil ingenting skje, ergo vil den stå i ro)
    if(xVal > 20){
      aimUp();
    }else if(xVal < -20){
      aimDown();
    }

    //Tilsvarende som ovenfor for xVal-verdien, så brukes yVal-verdien til å begeve våpenet til høyre, venstre eller stå i ro
    if(yVal > 20){
      aimRight();
    }else if(yVal < -20){
      aimLeft();
    }
}


//Denne funksjonen setter sammen data som blir mottatt fra Arduino Nano'en i kontrolleren.
void recvWithStartEndMarkers() {
    //Diverse variabler
    static boolean recvInProgress = false;
    static byte ndx = 0;      //Denne variabelen teller hvor mange lang en innkommende datapakke er
    char startMarker = '<';   //Markerer starten på den innkommende datapakken
    char endMarker = '>';     //Markerer slutten på den innkommende datapakken
    char rc;


    while (arduinoCOM.available() > 0 && newData == false) {
        rc = arduinoCOM.read(); //Leser byte for byte hva som kommer inn, og lagrer byten i rc (recieved character)

        if (recvInProgress == true) {
            if (rc != endMarker) {                //Hvis programmet ikke har motatt "endMarker" så lagrer den bytene som kommer
                receivedChars[ndx] = rc;          //Legger til byten i rc til "recievedChars"
                ndx++;                            //Legger til 1 på lengden til datapakken
                if (ndx >= numChars) {
                    ndx = numChars - 1;           //Må trekke fra 1 på lengden til datapakken, fordi datamaskiner begynner å telle på tallet 0.
                  }
                }
                else {  //Gjør dette hvis rc er lik ">"
                    receivedChars[ndx] = '\0';    //Avslutter stringen, enkelt forklart kan man si at det tilsvarer å trykke "enter" på tastaturet
                    recvInProgress = false;       //Stopper prosessen med å motta datapakken
                    ndx = 0;                      //Nullstiller variabelen ndx
                    newData = true;               //Sier ifra at det er kommet en ny datapakke
                }
        }
        //Hvis rc er lik "<" så vet programmet at det er starten på en ny datapakke
        else if (rc == startMarker) {
            recvInProgress = true; //I denne funksjonen må programmet i praksis gå ned hit først, før det igjen kan gå opp til begynnelsen på if-løkken på linje 103.
        }
    }
}


//Denne funksjonen deler opp datapakken fra kontrolleren til ulike variabler.
void parseData() {

    char * strtokIndx;   //Denne variablenen blir nedenfor brukt av "strtok" som en index
 
    strtokIndx = strtok(tempChars, ",");  //Definerer at programmet skal skille verdiene i datapakken med å se etter komma.
    xVal = atoi(strtokIndx);              //Trekker ut xVal fra datapakken og gjør det til en int.

    strtokIndx = strtok(NULL, ",");   //Programmet leser fra NULL-verdien i indexen til neste komma. (NULL-vedrien er som et usynlig tall som skal befinne seg på slutten av en string)
    yVal = atoi(strtokIndx);          //Trekker ut yVal fra datapakken og gjør det til en int.

    strtokIndx = strtok(NULL, ",");   //Programmet leser fra NULL-verdien i indexen til neste komma.
    btnState2 = atoi(strtokIndx);     //Trekker ut btnState2 fra datapakken og gjør det til en int.
    
    strtokIndx = strtok(NULL, ",");   //Programmet leser fra NULL-verdien i indexen til neste komma.
    btnState3 = atoi(strtokIndx);     //Trekker ut btnState2 fra datapakken og gjør det til en int.
}


//Denne funksjonen printer alle vedirene i Serial Monitor, slik at jeg enkelt kan debugge.
void showParsedData() {
    Serial.print("xVal: ");
    Serial.println(xVal);
    Serial.print("yVal: ");
    Serial.println(yVal);
    Serial.print("Btn State: ");
    Serial.println(btnState2);
    Serial.print("Btn State: ");
    Serial.println(btnState3);
}


//Hver gang programmet kommer inn i "fire-løkken" avfyres ett skudd.
void fire(){
  digitalWrite(fireMotors, LOW);  //Skrur på motorene som skyter skuddet
  
  delay(400);  //Venter på at motorene skal komme opp i maks hastighet.

  for(int i=0; i<2; i++){   //Denne for løkken fungerer som avtrekkeren til vaktpistolen, ved at servoen dytter skuddene inn i motorene som allerede er i maks hastighet.
    triggerServo.write(-70);
    delay(10);
   }

  delay(500);   //Venter på at skuddet er ute av løpet.
  
  for(int i=0; i<2; i++){  //Tilbakestiller servoen
    triggerServo.write(70);
    delay(10);
   }
   
  delay(500); 

  digitalWrite(fireMotors, HIGH); //Skrur av motorene som skøt skuddet
}


//Våpen siktes oppover
void aimUp(){
    digitalWrite(dirPinGun,LOW);  //Setter rettningen til oppover
    for(int x = 0; x < 2; x++) {  //for-løkken beveger steppermotoren to hakk oppover
    digitalWrite(stepPinGun,HIGH); 
    delayMicroseconds(2000); 
    digitalWrite(stepPinGun,LOW); 
    delayMicroseconds(2000); 
    }
}


//Våpen siktes nedover
void aimDown(){
    digitalWrite(dirPinGun,HIGH); //Setter rettningen til nedover
    for(int x = 0; x < 2; x++) {  //for-løkken beveger steppermotoren to hakk nedover
    digitalWrite(stepPinGun,HIGH); 
    delayMicroseconds(2000); 
    digitalWrite(stepPinGun,LOW); 
    delayMicroseconds(2000); 
    }
}


//Våpen siktes til høyre
void aimRight(){
    digitalWrite(dirPin,LOW);     //Setter rettningen til høyre
    for(int x = 0; x < 10; x++) { //for-løkken beveger steppermotoren ti hakk til høyre
    digitalWrite(stepPin,HIGH); 
    delayMicroseconds(2000); 
    digitalWrite(stepPin,LOW); 
    delayMicroseconds(2000); 
  }
}


//Våpen siktes til venstre
void aimLeft(){
    digitalWrite(dirPin,HIGH);    //Setter rettningen til venstre
    for(int x = 0; x < 10; x++) { //for-løkken beveger steppermotoren ti hakk til venstre
    digitalWrite(stepPin,HIGH); 
    delayMicroseconds(2000); 
    digitalWrite(stepPin,LOW); 
    delayMicroseconds(2000); 
  }
}
