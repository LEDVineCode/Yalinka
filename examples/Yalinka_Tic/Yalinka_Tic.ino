#include <Yalinka.h>

int tempo = 125;

void setup() {
  Yalinka::Init(GRAYSCALE);
//  Serial.begin(115200);
  //    Yalinka::Clear(7);
}
void loop() {
  int Tick = (millis() / 3000) % 30;

  switch (Tick) {
    case 0:
    case 1: {
        // Top
        Glow(19, 5, tempo, 0); // Glow
        // Body OFF
        OnOff(0, 19, tempo, 0);
        // Sides OFF
        OnOff(24, 18, tempo, 0);
      } break;

    case 2:
    case 3:
    case 4: {
        // Top
        Glow(19, 5, tempo, 0); // Glow
        // Body

        // Sides Drop set - 1
        Drop(24, 9 , tempo, 0); Drop(33, 9 , tempo, 0);
      } break;
    case 5:
    case 6:
    case 7: {
        // Top
        Glow(19, 5, tempo, 0); // Glow
        // Body - 2 Drops
        Drop(0, 19, tempo >> 1, 0); // Drop(0, 19, tempo>>1, 50);
        // Sides - Drop Both
        Drop(24, 9 , tempo, 0); Drop(33, 9 , tempo, 0);
      } break;
    case 8:
    case 9: {
        // Top
        RunningL(19, 5, tempo << 1, 7); // Beam (Sonar)
        // Body - 2 Drops
        Drop(0, 19, tempo >> 1, 0); Drop(0, 19, tempo >> 1, 50);
        // Sides - Drop Both Doubled
        Drop(24, 9 , tempo, 0); Drop(24, 9 , tempo, 50); Drop(33, 9 , tempo, 0); Drop(33, 9 , tempo, 50);
      } break;
    case 10:
    case 11:
    case 12: {
        // Top
        Drop(19, 5, tempo, 0); // Drop CCW
        // Body Glowing Two-by-two
        Glow(0, 2, tempo >> 1, 0); Glow(2, 7, tempo >> 1, 25); Glow(9, 6, tempo >> 1, 100); Glow(15, 4, tempo >> 1, tempo);
        // Sides - Glowing 6 pcs
        Glow(24, 3 , tempo, 0); Glow(27, 3 , tempo, 66); Glow(30, 3 , tempo, 33); Glow(33, 3 , tempo, 50); Glow(36, 3 , tempo, 16); Glow(39, 3 , tempo, 83);
      } break;
    case 13:
    case 14: {
        // Top
        Drop(19, 5, tempo, 0); // Drop CCW
        // Body Glowing Two-by-two
        Glow(0, 2, tempo >> 1, 0); Glow(2, 7, tempo >> 1, 25); Glow(9, 6, tempo >> 1, 100); Glow(15, 4, tempo >> 1, tempo);
        // Sides Drop set - 1
        Drop(24, 9 , tempo, 0); Drop(33, 9 , tempo, 0);
      } break;
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24: {
        // Top
        RunningL(19, 5, tempo, 4); // Sparks
        //  Bi-Drop opposite ansinchro
        DropSecond(0, 19, tempo >> 1, 0); DropSecondRev(1, 18, (tempo >> 1) + 5, 0);
        // Sides - Glowing Dots - opposite
        GlowSecond(24, 18 , tempo, 0); GlowSecond(25, 18 , tempo, 50); GlowSecond(33, 9 , tempo, 50); GlowSecond(34, 8 , tempo, 0);
      } break;
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    default: {
        // Top
        Drop(19, 5, tempo, 0); // Drop CCW
        // Body Running Set for X-Lights - 2
        OffOn(0, 2, tempo << 2, 20); OffOn(2, 7, tempo << 2, 40); OffOn(9, 6, tempo << 2, 60); OffOn(15, 4, tempo << 2, 80);
        // Sides Glowing set - 5
        GlowSecond(24, 9 , tempo >> 1, 0); GlowSecond(25, 8 , tempo >> 1, 50); GlowSecond(33, 9 , tempo >> 1, 0); GlowSecond(34, 8 , tempo >> 1, 50);
      } break;
  }

}


/*
// Top
  Glow(19, 5, tempo, 0); // Glow
  RunningL(19, 5, tempo<<1, 7); // Beam (Sonar)
  RunningL(19, 5, tempo, 4); // Sparks
  RunningL(19, 5, tempo, 6); // Rain CW
  RunningL(19, 5, tempo, 1); // Rain CCW
  Drop(19, 5, tempo, 0); // Drop CCW

// B O D Y = = = = = = = = = = = = = = = = =

// Body - 2 Drops
  Drop(0, 19, tempo>>1, 0); Drop(0, 19, tempo>>1, 50);

//  Bi-Drop opposite ansinchro
  DropSecond(0, 19, tempo>>1, 0); DropSecondRev(1, 18, (tempo>>1)+5, 0);

// Body Glow Wave
  Glow(0, 2, 150, 0); Glow(2, 7, 150, 25); Glow(9, 6, 150, 50); Glow(15, 4, 150, 75);

// Body Glowing Two-by-two
  Glow(0, 2, tempo>>1, 0); Glow(2, 7, tempo>>1, 25); Glow(9, 6, tempo>>1, 100); Glow(15, 4, tempo>>1, tempo);

// Body Glowing  X-Lights Ansinchro
  Glow(0, 2, tempo, 0); Glow(2, 7, tempo+25, 0); Glow(9, 6, tempo+50, 0); Glow(15, 4, tempo+100, 0);

// Body Glowing Second 33%
  GlowSecond(0, 19, 150, 0); GlowSecond(1, 18, 150, 33);

// Body Glowing 3-by-3 Wave
  GlowThird(0, 19, 150, 66); GlowThird(1, 18, 150, 33); GlowThird(2, 17, 150, 0);

// Body Rain Up
  RunningL(0, 19, 150, 1);

// Body Cleaning Lines
  OnOff(0, 2, tempo<<3, 20); OnOff(2, 7, tempo<<3, 40); OnOff(9, 6, tempo<<3, 60); OnOff(15, 4, tempo<<3, 80);

// Body Filling Lines
  OffOn(0, 2, tempo<<2, 20); OffOn(2, 7, tempo<<2, 40); OffOn(9, 6, tempo<<2, 60); OffOn(15, 4, tempo<<2, 80);

// Body Bink-Blink lines
  OnOff(0, 2, tempo<<2, 20); OffOn(2, 7, tempo<<2, 40); OnOff(9, 6, tempo<<2, 60); OffOn(15, 4, tempo<<2, 80);

// Body Spark lines - small asinchro
  RunningL(0, 2, tempo, 7); RunningL(2, 7, tempo+10, 7); RunningL(9, 6, tempo+20, 7); RunningL(15, 4, tempo+30, 7);


// S I D E S = = = = = = = = = = = = = = = = =
// Sides - Simple Blink - opposite
  OnOff(24, 9, tempo<<2, 50); OffOn(33, 9, tempo<<2, 50);
// Sides - Simple On
  OnOff(24, 9, tempo, 100); OffOn(33, 9, tempo, 100);

// Sides - Drop Both
  Drop(24, 9 , tempo, 0); Drop(33, 9 , tempo, 0);
//   Sides - Drop Side-by-side
  Drop(24, 9 , tempo, 0); Drop(33, 9 , tempo, 50);
// Sides - Drop Both Doubled
  Drop(24, 9 , tempo, 0); Drop(24, 9 , tempo, 50); Drop(33, 9 , tempo, 0); Drop(33, 9 , tempo, 50);

// Sides - Glowing Both
  Glow(24, 9 , tempo, 0); Glow(33, 9 , tempo, 0);
// Sides - Glowing Side-by-side
  Glow(24, 9 , tempo, 0); Glow(33, 9 , tempo, 50);
// Sides - Glowing 6 pcs
  Glow(24, 3 , tempo, 0); Glow(27, 3 , tempo, 66); Glow(30, 3 , tempo, 33); Glow(33, 3 , tempo, 50); Glow(36, 3 , tempo, 16); Glow(39, 3 , tempo, 83);

// Sides - Glowing Dots - opposite
  GlowSecond(24, 18 , tempo, 0); GlowSecond(25, 18 , tempo, 50); GlowSecond(33, 9 , tempo, 50); GlowSecond(34, 8 , tempo, 0);
// Sides - Glowing Dots - same
  GlowSecond(24, 9 , tempo>>1, 0); GlowSecond(25, 8 , tempo>>1, 50); GlowSecond(33, 9 , tempo>>1, 0); GlowSecond(34, 8 , tempo>>1, 50);
// Sides - Sparks little usinchro
  RunningL(24, 9, 125, 7); RunningL(33, 9, 126, 7);

// Sides - Rain CW
    RunningL(24, 9, 125, 1); RunningL(33, 9, 125, 6);
//   Sides - Rain CCW
    RunningL(24, 9, 125, 6); RunningL(33, 9, 125, 1);

*/
