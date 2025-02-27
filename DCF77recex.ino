#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

#define INP 5


int PW[] = {75, 135, 250, 750, 850, 950, 1750, 1850, 1950, 0};
int PN[] = { 0,   1,   2,   0,   3,   4,    0,    5,    6, 0};
 

static volatile int pulse = 0;

void pcINT() {
  static unsigned long prevt = 0;
  static unsigned long t;

  pulse = (t =millis()) - prevt;
  prevt = t;
  digitalWrite(LED_BUILTIN, digitalRead(INP));
}


int pulseno(int pw) {
  int p;
  for (p = 0; PW[p] != 0; p++) 
    if (PW[p] > pw) break;
  return PN[p];
}

int recDCF(uint64_t * rbits) {
  int pn;
  int rv = 0;
  static uint64_t recbits;
  static int rbnum = 0;

  if (pulse) {
    pn = pulseno(pulse);
    pulse = 0;
    //Serial.printf("%u - %u - %u\n", prevs, pulse, pn);
    switch (pn) {
      case 1:
        recbits >>= 1;
        rbnum++;
        Serial.print(0);
        break;
      case 2:
        recbits = (recbits >> 1) | (uint64_t(1) << 58);
        rbnum++;
        Serial.print(1);
        break;
      case 3:
      case 4:
        break;
      case 5:
      case 6:
        rv = rbnum;
        rbnum = 0;
        break;
      default:
        Serial.println();
        rbnum = 0;
    }
    *rbits = recbits;
  }
  return rv;
}


void setup() {

  pinMode(INP, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  digitalRead(INP);
  attachInterrupt(INP, pcINT, CHANGE);
}


char * decDCF(char * b, int blen, uint64_t bits) {
  int hour, p1, minute, p2;
  int year, month, day, dow, p3;
  uint32_t cal;
  const char* DWS[] = {"-0-", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

  if (blen != 59) {
    sprintf(b, "Invalid length: %u", blen);
  }
  else {
    hour = (bits >> 29) & 0b111111;
    p2 = (bits >> 35) & 0b1;
    p2 = (p2 ^ hour ^ (hour >> 1) ^ (hour >> 2)^ (hour >> 3) ^ (hour >> 4) ^ (hour >> 5)) & 1;
    minute = (bits >> 21) & 0b1111111;
    p1 = (bits >> 28) & 0b1;
    p1 = (p1 ^ minute ^ (minute >> 1) ^ (minute >> 2) ^ (minute >> 3) ^ (minute >> 4) ^ (minute >> 5) ^ (minute >> 6)) & 1;

    cal = (bits >> 36) & (uint32_t)0b1111111111111111111111;
    p3 = (bits >> 58) & 0b1;
    day = cal & 0b111111;
    dow = (cal >> 6) & 0b111;
    month = (cal >> 9) & 0b11111;
    year = (cal >> 14) & 0b11111111;
    for (int i = 0; i < 22; i++) {
      p3 ^= cal & 1;
      cal >>= 1;
    }

    hour = (hour & 0b1111) + 10 * (hour >> 4);
    minute = (minute & 0b1111) + 10 * (minute >> 4);

    day = (day & 0b1111) + 10 * (day >> 4);
    month = (month & 0b1111) + 10 * (month >> 4);
    year = (year & 0b1111) + 10 * (year >> 4);

    sprintf(b, "Received: %s %02u-%02u-%02u %02u:%02u %u %u %u", DWS[dow], year, month, day, hour, minute, p1, p2, p3);
  }
  return b;
}


static char buf[80];

void loop() {
  static int s;
  static unsigned long t;
  int blen;
  uint64_t bits;

  while (true) {

    if (blen = recDCF(&bits)) {
      sprintf(buf, "\n%u - %08x %08x", blen, (uint32_t)(bits >> 32), (uint32_t)bits);
      Serial.println(buf);
      Serial.println(decDCF(buf, blen, bits));
    }
  }
}
