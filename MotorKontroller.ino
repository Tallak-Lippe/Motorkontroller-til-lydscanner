const int STEP_PIN   = 2;     // Driver motoren rundt
const int DIR_PIN    = 3;     // Bestemmer retningen til motoren, LOW = fra motoren, HIGH = mot motoren
const int EN_PIN     = 4;     // AV/PÅ-funskjon til motoren, LOW = på, HIGH = av
const int LED_PIN    = 5;
const int BUTTON_PIN = 6;     // Start knappen

// Motorparametere
const int stepsPerRev = 200;   // Antall steg for en omdreining. 
const int revolutions = 13;    // Maks antall omdreinginer i en retning
const long totalSteps = stepsPerRev * revolutions;   // Maks steg i en retning

// State machine
enum State { INAKTIV, FREM, PAUSE, TILBAKE };  // Tilstander
State state = INAKTIV;                         // Starter maskinen i inaktiv tilstand

// Tidsvariabler
//unsigned long lastStepTime  = 0;    // Datatype for å bruke micros() funksjonen for tidstaking i stedet for delay
unsigned long lastPauseTime = 0;

long stepCounter = 0;

//const unsigned long stepInterval = 1200;    // Mikrosekund mellom hvert steg. Styrer hastigheten til vognen. 2=> 5,2 sekund på en vei
const unsigned long pauseTime    = 500000;  // Mikrosekund pause mellom fram og tilbake = 0,5 sekund. Pausen før vognen snur

const unsigned long stepPulse = 600;  // Tiden mellom HIGH og LOW, styrer hastigheten, jo høyere tall jo lavere hastighet, MÅ være over 400. 600 virker som mest ideell hastighet og liten støy
unsigned long lastStepPulse = 0;
bool stepState = LOW;


void setup() {
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(EN_PIN, HIGH);  // Motor AV

  //Serial.begin(9600);   // Serial.print får motoren til å gå for sakt
}

// Ett steg
void stepOnce() {
  unsigned long now = micros();

  if (now - lastStepPulse >= stepPulse) {    //Har det gått lengre tid enn 
    lastStepPulse = now;

    stepState = !stepState;              // !stepSte er lik !LOW som er lik HIGH
    digitalWrite(STEP_PIN, stepState);

    // Ett fullstendig steg når pulsen går HIGH → LOW
    if (stepState == LOW) {
      stepCounter++;
      
    }
  }
}

void loop() {
  unsigned long now = micros();

  switch (state) {

    // ---------- INAKTIV ----------
    case INAKTIV:
      Serial.println("State: INAKTIV");
      digitalWrite(EN_PIN, HIGH);  // Når vognen ikke skal kjøres er motoren AV

      //Knappen skal lyse helt til den blir trykket på
      digitalWrite(LED_PIN, HIGH);   // Knapplys på

      if (digitalRead(BUTTON_PIN) == HIGH) {    
        Serial.println("Knapp registrert");
        digitalWrite(LED_PIN, LOW);           // Knapplys av
        stepCounter = 0;
        digitalWrite(DIR_PIN, HIGH); // Setter retningen på motoren

        state = FREM;
      }
      break;

    // ---------- FREM ----------
    case FREM:
      Serial.println("State: FREM");
      if (stepCounter == 0) {
        digitalWrite(EN_PIN, LOW); // Slår motor PÅ
      }
      stepOnce();   // Kjører kontinuertlig
      
      /* Teller heller i stepOnce()
      if (now - lastStepTime >= stepInterval) {  // Sjekker om det har gått nok tid for å ta neste steg i motoren
        lastStepTime = now;                      // Tidspunkt da forrige steg ble tatt   
        stepOnce();                              // Tar et steg
        stepCounter++;   
        }                        // Teller steget
*/
      if (stepCounter >= totalSteps) {   // Vognen har kommet seg til den andre siden
        digitalWrite(EN_PIN, HIGH);      // Slår motor AV
        stepCounter = 0;                 // Nullstiller steg teller klar til tilbakegangen
        lastPauseTime = now;
        state = PAUSE;
      }
      break;

    // ---------- PAUSE ----------
    case PAUSE:
      //Serial.println("State: PAUSE");
      if (now - lastPauseTime >= pauseTime) {   // Står stille helt til pausetiden har gått
        digitalWrite(DIR_PIN, LOW);             // Endrer retningen til fra motoren
        state = TILBAKE;
      }
      break;

    // ---------- TILBAKE ----------
    case TILBAKE:
      //Serial.println("State: TILBAKE");
      if (stepCounter == 0) {
        digitalWrite(EN_PIN, LOW); // motor PÅ
      }

      stepOnce();

      /*if (now - lastStepTime >= stepInterval) {
        lastStepTime = now;
        stepOnce();
        stepCounter++;
      }*/
      if (stepCounter >= totalSteps) {
        digitalWrite(EN_PIN, HIGH); // motor AV
        state = INAKTIV;            // tilbake til ventemodus
      }
      break;
  }
}