//Navn: Simon Edna
//Dette er koden for kontrolleren til vaktpistolen. Alle kommentarene/forklaringene underveis skal være på norsk, men alle variabler og kode skal være i engelsk.

#include <SoftwareSerial.h>   //Inkluderer biblioteket SoftwareSerial

//Diverse konstante variabler
const int btn2 = 2;
const int btn3 = 3;
const int X_pin = 0;
const int Y_pin = 1;
const int rxPin = 12;
const int txPin = 13;

//Lager en ny Serial-port
SoftwareSerial arduinoCOM =  SoftwareSerial(rxPin, txPin);

//Volatile variabler for knappene
volatile int btnState2 = 0;
volatile int btnState3 = 0;

//Variabler til joystickene
int xMap;
int xVal;
int yMap;
int yVal;

//Char for datapakken
char data[20]; //Betyr at datapakkene skal være 20 bytes stor
char beginMark[2] = "<"; //Symbol for starten på en datapakke
char endMark[2] = ">";  //Symbol for slutten på en datapakke

void setup() {
  //Åpner to Serial-porter
  arduinoCOM.begin(2400);
  Serial.begin(115200);
  Serial.println("SETUP . . .");

  //Definerer diverse I/O-pin'er som input
  pinMode(btn2, INPUT_PULLUP);
  pinMode(btn3, INPUT_PULLUP);
  pinMode(X_pin, INPUT);
  pinMode(Y_pin, INPUT);

  //Lager to stykk "Interrupt Service Routine", heretter forkortet ISR. Det er en ISR for hver knapp.
  //På Arduino Uno er det kun mulig å lage en ISR på I/O-pin 2 og 3, dette har med hvordan mikrokontrolleren er laget.
  attachInterrupt(digitalPinToInterrupt(btn2), ISR_2, RISING);
  attachInterrupt(digitalPinToInterrupt(btn3), ISR_3, RISING);
}

void loop() {
  char rc;  //Definerer variabelen "Recieved Character", som forkortes rc.
  
  xMap = analogRead(X_pin);               //Leser x-verdien fra X_pin
  xVal = map(xMap, 0, 1023, -400, 400);   //Konverterer verdien som kan variere mellom 0 og 1023, til å være mellom -400 og 400. 
  yMap = analogRead(Y_pin);               //Leser y-verdien fra Y_pin
  yVal = map(yMap, 0, 1023, -400, 400);   //Konverterer verdien som kan variere mellom 0 og 1023, til å være mellom -400 og 400. 

  while (arduinoCOM.available() > 0){       //Mens programmet mottar data går den inn i denne while-løkken
    rc = arduinoCOM.read(); //Leser det som kommer inn av data og lagrer det i rc

    if(rc == 'A'){  //Hvis rc = "A", så betyr det at kontrolleren skal sende en datapakke til vaktpistolen
      Serial.println("Recieved A - need to transmit!!"); //For debug
      SerialPrt();  //Funksjon for å sende data
      btnState2 = 0;
      btnState3 = 0;
    }
  }

  Serial.println("Ute av While løkke!");

}

//ISR 2 registrerer trykk på I/O-pin 2
void ISR_2(){
  btnState2 = 2;
  delay(20);
}

//ISR 2 registrerer trykk på I/O-pin 3
void ISR_3(){
  btnState3 = 3;
  delay(20);
}

void SerialPrt(){
  sprintf(data, "%s%d, %d, %d, %d%s", beginMark, xVal, yVal, btnState2, btnState3, endMark); //sprintf er et slags copypaste-verktøy, som setter sammen alle variablene jeg vil sende til en string.
  
  arduinoCOM.println(data); //Sender data til vaktpistolen

  int stringLength = strlen(data);  //Sjekker lengden på string'en
  Serial.println(stringLength); //For debug
}
