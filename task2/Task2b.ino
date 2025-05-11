// Motor
const int IN1 = 17;
const int IN2 = 5;
const int ENA = 16; 
// Encoder pins
const int ENCODER_A = 12;
const int ENCODER_B = 13;
volatile int pos = 0;
long prevT = 0;
float eprev = 0;
float eintegral = 0;
const int pulses_per_rev = 235;
const int target_degrees = 360; //45, 90 - I dont want to merge all degree in one cause I cant not track the degree
const int target_pos = (pulses_per_rev * target_degrees) / 360;
bool motorStopped = false;
void setup() {
  Serial.begin(9600);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENCODER_A, INPUT);
  pinMode(ENCODER_B, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENCODER_A), readencoder, RISING);
  Serial.print("Rotating motor to ");
  Serial.print(target_degrees);
  Serial.println(" degrees...");
}
void loop() {
  Serial.print("Target: ");
  Serial.print(target_pos);
  Serial.print(" | Pos: ");
  Serial.print(pos);
  Serial.print(" | Error: ");
  Serial.println(target_pos - pos);
  if (!motorStopped && abs(pos) >= target_pos) {
    analogWrite(ENA, 0);
    Serial.println("Rotation complete!");
    motorStopped = true;
  }
  if (motorStopped) {
    delay(10);  
    return;
  }
  float Kp = 60;
  float Ki = 270;
  float Kd = 0;

  long currentT = millis();
  float dt = (currentT - prevT) / 1000.0;
  prevT = currentT;

  float error = target_pos - pos;
  eintegral += error * dt;
  float dedt = (error - eprev) / dt;
  float u = Kp * error + Kd * dedt + Ki * eintegral;
  eprev = error;

  float pwr = fabs(u);
  if (pwr > 255) pwr = 60; //For 45: 80 because the speed too low 

  int dir = (u > 0) ? 1 : -1;

  digitalWrite(IN1, dir == 1 ? HIGH : LOW);
  digitalWrite(IN2, dir == 1 ? LOW : HIGH);
  analogWrite(ENA, (int)pwr);

  delay(10);  
}
void readencoder() {
  int b = digitalRead(ENCODER_B);
  if (b == HIGH) {
    pos++;
  } else {
    pos--;
  }
}
