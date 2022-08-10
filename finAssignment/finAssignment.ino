#include "funshield.h"
#define KEY_PRESSED true
#define KEY_NOT_PRESSED false

class Display {
  private:    
    int numPins;
    int letter_d = 0xA1;
    int dash = 0xBF;
    int glyph = 0;
    int const static number_of_glyphs = 4;
    int emptyGlyph = 0xff;
    int const static digit = 10;
     
  public:
    Display(int numPins){
      this->numPins=numPins;
    }

    int throws = 1;
    int typeOfDice = 0;
    bool configuration = true;
    int cislo = 0;   
    int const static numberOfDices = 7; 
    int dices[numberOfDices] = {4, 6, 8, 10, 12, 20, 100 };
    byte segmentMap[digit] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};
    int screen[number_of_glyphs] = {segmentMap[1], letter_d, segmentMap[0], segmentMap[dices[typeOfDice]]};     
    
    void inicializace(){
      int displayPins[] = {latch_pin, clock_pin, data_pin};
      for (int i=0; i < numPins; i++){
        pinMode(displayPins[i], OUTPUT);
      }
    }
    
    void write_glyph(byte glyph, byte position_){
      digitalWrite(latch_pin, LOW);
      shiftOut(data_pin, clock_pin, MSBFIRST, glyph);
      shiftOut(data_pin, clock_pin, MSBFIRST, position_);
      digitalWrite(latch_pin, HIGH);
    }
    
    void writeRandomNumber(){
      int c = cislo;
      for (int i = number_of_glyphs-1; i>=0; i--) {
        if (cislo < 10 &&  (i == 2 || i == 1 || i == 0)) screen[i] = emptyGlyph;
        else if (cislo < 100 && (i == 0 || i == 1)) screen[i] = emptyGlyph;
        else if (cislo < 1000 && i == 0) screen[i] = emptyGlyph;
        else screen[i] = segmentMap[c%digit];
        c = c / digit;
      }      
    }
    
    void writeThrowing(){
      for (int i = 0; i < number_of_glyphs; i++) screen[i] = dash;      
    }   
         
    void writeConfiguration(){
      screen[0] = segmentMap[throws];
      screen[1] = letter_d;
      screen[2] = segmentMap[(dices[typeOfDice] / digit)%digit];
      screen[3] = segmentMap[dices[typeOfDice] % digit];    
    }

    void writeScreen(){ 
      write_glyph(screen[number_of_glyphs-1 - glyph], digit_muxpos[number_of_glyphs-1-glyph]);
      glyph = (glyph+1)%number_of_glyphs;
    }
};
Display display4(3);

 
class Random{
  public:
   int minimum;
   int maximum;
   int sum = 0;
  
  int randomizer(){
    minimum = display4.throws;
    maximum = display4.throws * display4.dices[display4.typeOfDice];
    int cislo = random(minimum, maximum + 1);
    return cislo;
  }

  void randomSum(){
    int a = sum % (maximum + 1);
    if (a< minimum) a = minimum;
    display4.cislo = a;
  }
};
Random r;


class Knopka {  
  public:
    int knopkaNumber;
    Knopka(int knopkaNumber ) {
      this->knopkaNumber = knopkaNumber;
    } 
    unsigned long activationDelay = 10;  
    int maxThrows = 9; 
    int minThrows = 1; 
    bool tedStav = KEY_NOT_PRESSED;
    bool minuleStates = KEY_NOT_PRESSED;
    unsigned long Time = 0;
    
    void inicializaceKnopki() {
      pinMode(knopkaNumber, INPUT);
    }

    int cteniStavu() {
     return digitalRead(knopkaNumber);
    }
    
    void SecondButton(){
      if (display4.configuration){
        if (display4.throws == maxThrows){display4.throws = minThrows;}
        else{display4.throws++;}
      }
      else if (!display4.configuration){
        display4.configuration = true;
      }
      display4.writeConfiguration();
    }

    void ThirdButton(){
      if (display4.configuration){display4.typeOfDice = (display4.typeOfDice+1)%display4.numberOfDices;}
      else if (!display4.configuration){display4.configuration = true;}
      display4.writeConfiguration();
    }

    void FirstButton(){
      if (tedStav && display4.configuration){
        Time = millis();
        display4.configuration = false;        
      }
      else if (!display4.configuration && tedStav){
        if (millis() - Time >= activationDelay){display4.writeThrowing();}
      }
      else if (!display4.configuration && !tedStav){
        if (millis()- Time > activationDelay ){
          int c = r.randomizer();
          r.sum += c;     
        }
        r.randomSum();
        display4.writeRandomNumber();
        Time = millis();
      }
      minuleStates = tedStav;
    }
};

class Knopky{
  private:
    int const static buttonPinsCount = 3;
    int buttonPins[buttonPinsCount] = { button1_pin, button2_pin, button3_pin };  
  public:
    Knopka arr[buttonPinsCount] = { Knopka(buttonPins[0]), Knopka(buttonPins[1]), Knopka(buttonPins[2] )};
      
    void inicializace(){
      for (int i = 0; i < buttonPinsCount; i++){arr[i].inicializaceKnopki();}
    }

    void stridani(){
      display4.writeScreen();
      for (int i = 0; i < buttonPinsCount; i++) {         
        arr[i].tedStav = !arr[i].cteniStavu();
        if (arr[i].knopkaNumber == button1_pin){ arr[i].FirstButton();}
        else if (arr[i].knopkaNumber == button2_pin && arr[i].tedStav && arr[i].minuleStates != arr[i].tedStav){ arr[i].SecondButton();}  
        else if (arr[i].knopkaNumber == button3_pin && arr[i].tedStav && arr[i].minuleStates != arr[i].tedStav){ arr[i].ThirdButton();}   
        arr[i].minuleStates = arr[i].tedStav;        
      }
  }  
};

Knopky buttons;

void setup() 
{
  Serial.begin(9600);
  randomSeed(analogRead(A0));
  display4.inicializace();
  buttons.inicializace();
}

void loop() {
  Serial.println(display4.cislo);
  buttons.stridani();
}
