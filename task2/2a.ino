// Motor 
const int IN1 = 17;
const int IN2 = 5;
const int ENA = 16; 
const int ENCODER_A = 12;
const int ENCODER_B = 13;

// Encoder Parameters
#define PPR 235       
#define PULSES_PER_REV (PPR * 4)  
#define DEBOUNCE_TIME 2000

volatile long pulseCount = 0; 
volatile unsigned long lastInterruptTime = 0;


void IRAM_ATTR handleEncoder() {
  unsigned long currentTime = micros();
  if (currentTime - lastInterruptTime < DEBOUNCE_TIME) {
    return;
  }
  lastInterruptTime = currentTime;

  static int lastA = LOW;
  int a = digitalRead(ENCODER_A);
  int b = digitalRead(ENCODER_B);

  if (lastA != a) {  
    if (a == HIGH) {
      pulseCount += (b == LOW) ? 1 : -1;  // A rising: B low -> forward, B high -> reverse
    } else {
      pulseCount += (b == HIGH) ? 1 : -1;  // A falling: B high -> forward, B low -> reverse
    }
  }
  lastA = a;

  // Debug: print encoder states (uncomment for troubleshooting)
  // Serial.print("A: "); Serial.print(a); Serial.print(" B: "); Serial.print(b);
  // Serial.print(" Count: "); Serial.println(pulseCount);
}

void setup() {
  // Initialize Serial
  Serial.begin(115200);

  // Set up L298N pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);
  Serial.println("Motor stopped (IN1=LOW, IN2=LOW, ENA=0). Ready for manual rotation.");

  // Set up encoder pins with internal pull-ups
  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);

  // Attach interrupts for both channels in CHANGE mode
  attachInterrupt(digitalPinToInterrupt(ENCODER_A), handleEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_B), handleEncoder, CHANGE);

  // Print encoder settings
  Serial.print("Encoder PPR (per channel): ");
  Serial.println(PPR);
  Serial.print("Total pulses per revolution (CHANGE mode): ");
  Serial.println(PULSES_PER_REV);
  Serial.println("Rotate wheel manually 360 degrees to count pulses.");
}

void loop() {
  // Static variables to track pulses for one revolution
  static long startPulseCount = 0;
  static bool countingRevolution = false;

  // Start counting for a new revolution
  if (!countingRevolution) {
    startPulseCount = pulseCount;
    countingRevolution = true;
  }

  // Check if one revolution is completed
  if (abs(pulseCount - startPulseCount) >= PULSES_PER_REV) {
    long pulsesInRevolution = abs(pulseCount - startPulseCount);
    Serial.print("Revolution completed! Pulses counted: ");
    Serial.println(pulsesInRevolution);
    countingRevolution = false;  
  }

  // Print current pulse count periodically, ignore small noise
  if (millis() % 1000 == 0) {
    if (abs(pulseCount) > 5) {  
      Serial.print("Current pulse count: ");
      Serial.println(pulseCount);
    } else {
      Serial.println("Pulse count stable (no significant movement)");
    }
    delay(1);  
  }
}
