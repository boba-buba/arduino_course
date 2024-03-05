#include "funshield.h"
#include "input.h"
// map of letter glyphs
constexpr byte LETTER_GLYPH[] {
  0b10001000,   // A
  0b10000011,   // b
  0b11000110,   // C
  0b10100001,   // d
  0b10000110,   // E
  0b10001110,   // F
  0b10000010,   // G
  0b10001001,   // H
  0b11111001,   // I
  0b11100001,   // J
  0b10000101,   // K
  0b11000111,   // L
  0b11001000,   // M
  0b10101011,   // n
  0b10100011,   // o
  0b10001100,   // P
  0b10011000,   // q
  0b10101111,   // r
  0b10010010,   // S
  0b10000111,   // t
  0b11000001,   // U
  0b11100011,   // v
  0b10000001,   // W
  0b10110110,   // ksi
  0b10010001,   // Y
  0b10100100,   // Z
};
constexpr byte EMPTY_GLYPH = 0b11111111;

//constexpr int posiceiceitionsCount = 4;
constexpr unsigned int scrollingInterval = 300;

/** 
 * Show chararcter on given posiceiceition. If character is not letter, empty glyph is displayed instead.
 * @param ch character to be displayed
 * @param posiceice posiceiceition (0 = leftmost)
 */
class Display
{
  private:
  int numPins;  
  public:
    Display(int numPins){
      this->numPins=numPins;
    }
    int const static number_of_glyphs = 4;
    void inicializace(){
      int displayPins[] = {latch_pin, clock_pin, data_pin};
      for (int i=0; i < numPins; i++){
        pinMode(displayPins[i], OUTPUT);
      }
    }
    void displayChar(char ch, byte pozice)
    {
      byte glyph = EMPTY_GLYPH;
      if (isAlpha(ch)) {
        glyph = LETTER_GLYPH[ ch - (isUpperCase(ch) ? 'A' : 'a') ];
      }
      
      digitalWrite(latch_pin, LOW);
      shiftOut(data_pin, clock_pin, MSBFIRST, glyph);
      shiftOut(data_pin, clock_pin, MSBFIRST, pozice);
      digitalWrite(latch_pin, HIGH);
    }  
};
Display dis(3);
SerialInputHandler input;

class Message{
  private:
    const char* line;
    int len;
    int phraseSpaces;
    char phrase[dis.number_of_glyphs];
    int numSpaces = dis.number_of_glyphs;
    int glyph = 0;
    int pozice = 0;
    long unsigned prev = 0;
    char toDisplay[dis.number_of_glyphs];
  public:
    int delkaMessage(const char* line){
      int i = 0;
      int delka = 0;
      while (line[i] != '\0'){
        delka++; 
        i++;
      } 
      return delka;
    }
    
    void updating(){
      line = input.getMessage();
      len = delkaMessage(line);
      phraseSpaces = len + numSpaces;
    }
   
    void writing(){
      dis.displayChar(phrase[dis.number_of_glyphs-1-glyph], digit_muxpos[dis.number_of_glyphs-1-glyph]);
      glyph = (glyph + 1)%dis.number_of_glyphs;
    }

    void setPhrase(){
      if (millis() - prev >= scrollingInterval){
        for (int i = 0; i < dis.number_of_glyphs; i++){
          if (pozice-i < 0) toDisplay[dis.number_of_glyphs-1-i] = ' ';
          else
          {
            if (pozice-i >= len) toDisplay[dis.number_of_glyphs-1-i] = ' ';
            else toDisplay[dis.number_of_glyphs-1-i] = line[pozice-i];
          }
        }
        for (int i = 0; i < dis.number_of_glyphs; i++) phrase[i] = toDisplay[i];
        pozice++;
        phraseSpaces--;
        if (phraseSpaces == 0){
          updating();
          pozice = 0;
          phraseSpaces = len + numSpaces;
        }
        prev = millis();
      }
   }
};

Message message;
void setup() {
  dis.inicializace();
  input.initialize();
  message.updating();
  message.setPhrase();
}

void loop() {
  message.setPhrase();
  message.writing();
  input.updateInLoop();
}
