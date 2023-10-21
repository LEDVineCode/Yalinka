#include <Yalinka.h> 

void setup() {
  Yalinka::Init();  //Initializes the screen
    Yalinka::Clear();
}
void loop() {
    for (byte i = 0; i < 42; i++)
    {
      Yalinka::Set(i, 1);
      delay(50);
    }
    delay(2000);
    Yalinka::Clear();
}

