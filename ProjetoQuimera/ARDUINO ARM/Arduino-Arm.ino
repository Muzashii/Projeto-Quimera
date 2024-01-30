#include <Servo.h>
#include <AccelStepper.h>

#define numOfValsRec 7
#define digitsPerValRec 1

#define STEP_PIN 22
#define DIR_PIN 23

AccelStepper stepper(1, STEP_PIN, DIR_PIN);

Servo servoThumb;
Servo servoIndex;
Servo servoMiddle;
Servo servoRing;
Servo servoPinky;
Servo servoArm;
Servo servoWrist;

int valsRec[numOfValsRec];
int stringLength = numOfValsRec * digitsPerValRec + 1;
int counter = 0;
bool counterStart = false;
String receivedString;
int estpasso = 90;
int lastEstpasso = 90;

void setup() {
  Serial.begin(9600);
  servoThumb.attach(7);
  servoIndex.attach(5);
  servoMiddle.attach(6);
  servoRing.attach(8);
  servoPinky.attach(10);
  servoArm.attach(4);
  servoWrist.attach(9);

  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(500);
}

void receieveData() {
  while (Serial.available()) {
    char c = Serial.read();
    
    if (c == '$') {
      counterStart = true;
    }
    if (counterStart) {
      if (counter < stringLength) {
        receivedString = String(receivedString + c);
        counter++;
      }
      if (counter >= stringLength) {
        for (int i = 0; i < numOfValsRec; i++) {
          int num = (i * digitsPerValRec) + 1;
          valsRec[i] = receivedString.substring(num, num + digitsPerValRec).toInt();
        }
        receivedString = "";
        counter = 0;
        counterStart = false;
      }
    }
  }
}

void moveElbow() {
  if (valsRec[5] != lastEstpasso) {
    lastEstpasso = valsRec[5];
    if (valsRec[5] == 0) {
      stepper.moveTo(0);
      estpasso = 0;
    } else if (valsRec[5] == 1) {
      stepper.moveTo(90);
      estpasso = 90;
    } else if (valsRec[5] == 2) {
      stepper.moveTo(180);
      estpasso = 180;
    }
  }
}

void loop() {
  receieveData();

  // Controlar os dedos independentemente do movimento do cotovelo
  if (valsRec[0] == 1) { servoThumb.write(180); } else { servoThumb.write(0); }
  if (valsRec[1] == 1) { servoIndex.write(180); } else { servoIndex.write(0); }
  if (valsRec[2] == 1) { servoMiddle.write(180); } else { servoMiddle.write(0); }
  if (valsRec[3] == 1) { servoRing.write(180); } else { servoRing.write(0); }
  if (valsRec[4] == 1) { servoPinky.write(180); } else { servoPinky.write(0); }

  // Controlar o movimento do cotovelo
  moveElbow();
  stepper.run();

  // Novo código para girar o servo do pulso baseado no estado do pulso
  if (valsRec[6] == 0) { servoWrist.write(0); }
  else { servoWrist.write(180); }
}
