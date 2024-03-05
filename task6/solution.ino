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


constexpr unsigned int scrollingInterval = 300;
long unsigned int prev = 0;


/** 
 * Show chararcter on given position. If character is not letter, empty glyph is displayed instead.
 * @param ch character to be displayed
 * @param pos position (0 = leftmost)
 */

class Display
{
  public:
    void displayChar(char ch, byte pos)
    {
      byte glyph = EMPTY_GLYPH;
      if (isAlpha(ch)) {
        glyph = LETTER_GLYPH[ ch - (isUpperCase(ch) ? 'A' : 'a') ];
      }
      
      digitalWrite(latch_pin, LOW);
      shiftOut(data_pin, clock_pin, MSBFIRST, glyph);
      shiftOut(data_pin, clock_pin, MSBFIRST, pos);
      digitalWrite(latch_pin, HIGH);
    }

    void initDisplay()
    {
      int pins[] = {latch_pin, clock_pin, data_pin};
      int number_of_pins = sizeof(pins)/sizeof(pins[0]);
      for (int i = 0; i < number_of_pins; i++)
        pinMode(pins[i], OUTPUT);
    }
    
};
Display d;

SerialInputHandler input;
const char* s;
int length;

class Message
{
  private:
    int phrase_with_gaps;//
    int number_of_gaps = 4;//
    char on_display[4];
    char phrase[4]; //
    int glyph = 0; // 
    int border = 4; //
    int pos = 0;//

  public:
    void AddGaps()
    { phrase_with_gaps = length + number_of_gaps; }

    int MessageLength(const char* s)
    {
      int index = 0;
      int l = 0;
      while (s[index] != '\0')
      { l++; index++; }
      return l;
    }

    void WritePhrase()
    {
      d.displayChar(phrase[3-glyph], digit_muxpos[3-glyph]);
      glyph = (glyph+1) % border;
    }

    void GetPhrase(char display[4])
    {
      for (int i = 0; i < 4; i++)
        phrase[i] = display[i];
    }

    void Update()
    {
      s = input.getMessage();
      len = MessageLength(s);
    }  

    void SetPhrase()
    {
      if (millis() - prev >= scrollingInterval)
      {
        for (int i = 0; i < 4; i++)
        {
          if (pos-i < 0) on_display[3-i] = ' ';
          else
          {
            if (pos-i >= len) on_display[3-i] = ' ';
            else on_display[3-i] = s[pos-i];
          }    
        }

        GetPhrase(on_display);
        pos++;
        phrase_with_gaps--;
        if (phrase_with_gaps == 0)
        {
          Update();        
          pos = 0;
          phrase_with_gaps = len + number_of_gaps;
        }
        prev = millis();
      }
    }
};

Message m;

void setup() 
{
  d.initDisplay();
  input.initialize();

  m.Update();
  m.AddGaps();
  m.SetPhrase();
}

void loop() 
{
  m.SetPhrase();
  m.WritePhrase();
  input.updateInLoop();
}
