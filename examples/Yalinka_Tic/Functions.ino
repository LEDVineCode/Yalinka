
void Drop(uint8_t start, uint8_t count, int tempo, uint8_t d_tempo) {
  int tic = (millis() / tempo + int(d_tempo * (count + 14) / 100)) % (count + 14) - count - 6;
  uint8_t b = 0;
  for (uint8_t i = 0; i < count ; i++) {
    b = tic + i;
    if (i >= 0 && i < 19 && b >= 0 && b < 8) {
      Yalinka::Set(start + i, 7 - b);
    }
  }
}

void DropSecond(uint8_t start, uint8_t count, int tempo, uint8_t d_tempo) {
  int tic = (millis() / tempo + int(d_tempo * (count + 14) / 100)) % (count + 14) - count - 6;
  uint8_t b = 0;
  for (uint8_t i = 0; i <= count >> 1 ; i++) {
    b = tic + i;
    if (i >= 0 && i <= (count >> 1) && b >= 0 && b < 8) {
      Yalinka::Set(start + (i << 1), 7 - b);
    }
  }
}

void DropSecondRev(uint8_t start, uint8_t count, int tempo, uint8_t d_tempo) {
  int tic = (millis() / tempo + int(d_tempo * (count + 14) / 100)) % (count + 14) - count - 6;
  uint8_t b = 0;
  for (uint8_t i = 0; i < (count >> 1) ; i++) { 
    b = tic + i;
    if (i >= 0 && i < (count >> 1) && b >= 0 && b < 8) {
      //      Yalinka::Set(start + (i<<1), 7-b);
      Yalinka::Set(-start + count - (i << 1), 7 - b); 
    }
  } 
}

void Glow(uint8_t start, uint8_t count, int tempo, uint8_t d_tempo) {
  int tic = (millis() / tempo + int(d_tempo * 12 / 100)) % 12;
  for (uint8_t i = 0; i < count ; i++) {
    Yalinka::Set(start + i, 1 + abs(6 - tic));
  }
}

void GlowSecond(uint8_t start, uint8_t count, int tempo, uint8_t d_tempo) {
  int tic = (millis() / tempo + int(d_tempo * 12 / 100)) % 12;
  for (uint8_t i = 0; i < count ; i += 2) {
    Yalinka::Set(start + i, 1 + abs(6 - tic));
  }
}

void GlowThird(uint8_t start, uint8_t count, int tempo, uint8_t d_tempo) {
  int tic = (millis() / tempo + int(d_tempo * 12 / 100)) % 12;
  for (uint8_t i = 0; i < count ; i += 3) {
    Yalinka::Set(start + i, 1 + abs(6 - tic));
  }
}

void Spark(uint8_t start, uint8_t count, int tempo, uint8_t d_tempo) {
  int tic = (millis() / tempo + int(d_tempo * 7 / 100)) % 7;
  for (uint8_t i = 0; i < count ; i++) {
    Yalinka::Set(start + i, tic);
  }
}

void SparkSecond(uint8_t start, uint8_t count, int tempo, uint8_t d_tempo) {
  int tic = (millis() / tempo + int(d_tempo * 7 / 100)) % 7;
  for (uint8_t i = 0; i < count ; i += 2) {
    Yalinka::Set(start + i, tic);
  }
}

void SparkThird(uint8_t start, uint8_t count, int tempo, uint8_t d_tempo) {
  int tic = (millis() / tempo + int(d_tempo * 7 / 100)) % 7;
  //  Serial.println(tic);delay(125);
  for (uint8_t i = 0; i < count ; i += 3) {
    Yalinka::Set(start + i, tic);
  }
}

void RunningL(uint8_t start, uint8_t count, int tempo, uint8_t step) {
  int tic = 7 - (millis() / tempo) % 7;
  //  Serial.println(tic);delay(125);
  for (uint8_t i = 0; i < count ; i++) {
    Yalinka::Set(start + i, tic);
    tic = (tic + step >= 7 ? tic + step - 7 : tic + step);
  }
}

void OnOff(uint8_t start, uint8_t count, int tempo, uint8_t on) {
  int tic = (millis() / ((tempo) / 100)) % (100);
  uint8_t bri = (tic >= on ? 0 : 7);
  for (uint8_t i = 0; i < count ; i++) {
    Yalinka::Set(start + i, bri);
  }
}

void OffOn(uint8_t start, uint8_t count, int tempo, uint8_t on) {
  int tic = (millis() / ((tempo) / 100)) % (100);
  uint8_t bri = (tic >= on ? 7 : 0);
  for (uint8_t i = 0; i < count ; i++) {
    Yalinka::Set(start + i, bri);
  }
}
