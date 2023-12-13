// BIG PROFESSOR
// by C. Ponsard 2023
//
// code to emulate behaviour of little professor on a arduinon nano
// requires
// * a LED display of MD_MAX72xx family (8 modules, e.g. 2 x 4)
// * to connect a keyboard matrix (see rowPins and colPins)
//   (can be build using PC keys and some 3D printing, see full project)
//
// history:
// * dec 2022 - PoC without keyboard (only addition)
// * july 2023 - dummy 2x2 keyboard integration
// * nov 2023 - full demo version 2023 kikk festival with partial support for other operations
//
// License: GPL V3

#include <MD_MAX72xx.h>
#include <SPI.h>
#include <Keypad.h>

#define PRINT(s, v) { Serial.print(F(s)); Serial.print(v); }

//========================================
// high level variables and structures
//========================================

// number of questions
#define N 5

enum Operation { ADD, MIN, MUL, DIV };
Operation op;
int term1[N];
int term2[N];
String SOP[4]={"+","-","x","/"};
int question;
boolean printed;
String answer;
int checked;
int score;

//===================
// low level stuff
//===================
// keypad
const byte ROWS = 5; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
/*
char hexaKeys[ROWS][COLS] = {
  {'1','2'},
  {'3','4'}
};
byte rowPins[ROWS] = {2, 3}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {4, 5}; //connect to the column pinouts of the keypad
*/

char hexaKeys[ROWS][COLS] = {
  {' ','0', ' ', '+'},
  {'1','2', '3', '-'},
  {'4','5', '6', '*'},
  {'7','8', '9', '/'},
  {' ',' ', 'R', 'L'}
};

// provide keyboard matrix here
byte rowPins[ROWS] = {2, 3, 4, 5, 6}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {7, 8, 9, 10};   //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

// LED stuff
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 8
#define CLK_PIN   13  // or SCK
#define DATA_PIN  12  // or MOSI was 11
#define CS_PIN    11  // or SS  was 10

// SPI hardware interface
//MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// note we use arbitrary pins due to keyboard matrix
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// Text parameters
#define CHAR_SPACING  1 // pixels between characters
#define OUT_SIZE 15     // TODO check size
char ctext[OUT_SIZE];

// message from the serial line
#define BUF_SIZE 10 // TODO check size
char message[BUF_SIZE];
bool newMessageAvailable = true;

void readSerial(void)
{
  static uint8_t	putIndex = 0;

  while (Serial.available())
  {
    message[putIndex] = (char)Serial.read();
    if ((message[putIndex] == '\n') || (putIndex >= BUF_SIZE-3))  // end of message character or full buffer
    {
      // put in a message separator and end the string
      message[putIndex] = '\0';
      // restart the index for next filling spree and flag we have a message waiting
      putIndex = 0;
      newMessageAvailable = true;
    }
    else
      // Just save the next char in next location
      message[putIndex++];
  }
}

void printText(uint8_t modStart, uint8_t modEnd, char *pMsg)
// Print the text string to the LED matrix modules specified.
// Message area is padded with blank columns after printing.
{
  uint8_t   state = 0;
  uint8_t   curLen;
  uint16_t  showLen;
  uint8_t   cBuf[8];
  int16_t   col = ((modEnd + 1) * COL_SIZE) - 1;

  mx.control(modStart, modEnd, MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  do     // finite state machine to print the characters in the space available
  {
    switch(state)
    {
      case 0: // Load the next character from the font table
        // if we reached end of message, reset the message pointer
        if (*pMsg == '\0')
        {
          showLen = col - (modEnd * COL_SIZE);  // padding characters
          state = 2;
          break;
        }

        // retrieve the next character form the font file
        showLen = mx.getChar(*pMsg++, sizeof(cBuf)/sizeof(cBuf[0]), cBuf);
        curLen = 0;
        state++;
        // !! deliberately fall through to next state to start displaying

      case 1: // display the next part of the character
        mx.setColumn(col--, cBuf[curLen++]);

        // done with font character, now display the space between chars
        if (curLen == showLen)
        {
          showLen = CHAR_SPACING;
          state = 2;
        }
        break;

      case 2: // initialize state for displaying empty columns
        curLen = 0;
        state++;
        // fall through

      case 3:	// display inter-character spacing or end of message padding (blank columns)
        mx.setColumn(col--, 0);
        curLen++;
        if (curLen == showLen)
          state = 0;
        break;

      default:
        col = -1;   // this definitely ends the do loop
    }
  } while (col >= (modStart * COL_SIZE));

  mx.control(modStart, modEnd, MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

//===================================================================
// big professor logic
//===================================================================

void printlnBoth(String s) {
  Serial.println("LED "+s);
  s.toCharArray(ctext,s.length()+1);
  printText(0, MAX_DEVICES-1, ctext);
}

void generate() {
  int a,b,c;
  if (op==DIV) {
    for(int i=0; i<N; i++) {
      a=random(0,20);
      b=random(0,20);
      c=a*b;    
      term1[i]=c;
      term2[i]=a;
    }
    return;
  }

  // ADD-SUB-MUL
  for(int i=0; i<N; i++) {
    a=random(1,9); // TODO change here for difficulty
    b=random(1,9); // TODO change here for difficulty
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  if (op==DIV && (b>a)) swap(a,b);
    term1[i]=a;
    term2[i]=b;
  }
}

String compute(int a, int b) {
  int res;
  switch (op) {
    case ADD: res=a+b;
              break;
    case MIN: res=a-b;
              break;
    case MUL: res=a*b;
              break;
    case DIV: res=a/b;
              break;
  }
  return String(res);
}

void swap(int a, int b) {
  int t=a;
  a=b;
  b=t;
}


//===================================================================
// setup and loop logic
//===================================================================

void setup()
{
  // matrix init
  mx.begin();

  // serial init
  Serial.begin(9600);
  Serial.print("\nWelcome to BigProfessor\n");
  printlnBoth(" BONJOUR !");
  delay(1000);

  randomSeed(analogRead(0));
  op=ADD;
  question=N;
  printed=false;
}

String text_q;
String text_i;

boolean checkAndSwitch(char cin) {
     Serial.println("CHANGE REQUESTED to "+cin);
     if (cin=='+') {
      if (op==ADD) return false;
      op = ADD;
      generate();
      question=0;
      score=0;
      return true;
    }
    if (cin=='-') {
      if (op==MIN) return false;
      op = MIN;
      generate();
      question=0;
      score=0;
      return true;
    }
    if (cin=='x') {
      if (op==MUL) return false;
      op = MUL;
      generate();
      question=0;
      score=0;
      return true;
    }
    if (cin=='/') {
      if (op==DIV) return false;
      op = DIV;
      generate();
      question=0;
      score=0;
      return true;
    }
    return false;
}

void loop()
{  
  if (question==N) {
    printlnBoth("SCORE: "+String(score)+"/5");
    delay(2000);
    //Serial.println("SCORE: "+String(score)); // TODO distinguish start improve FSM
    generate();
    question=0;
    score=0;
    return;
  }

  // ongoing
  if (!printed) {
    text_q=String(term1[question])+" "+SOP[op]+" "+String(term2[question])+" = ";
    text_i=text_q;
    String text_o=text_q+"?";
    answer=compute(term1[question],term2[question]);
    Serial.println("\n"+String(question+1)+" "+text_o+"  "+answer);
    printlnBoth(text_o);
    printText(0, MAX_DEVICES-1, ctext);
    printed=true;
    checked=0;
    readSerial(); // TODO improve this is just to clear...
  }

  char cin = customKeypad.getKey();  
  if (cin){
    Serial.println("cin: "+cin);
//    if (checkAndSwitch(cin)) return;
    
    text_i=text_i+cin;
    text_i.toCharArray(ctext,text_i.length()+1);
    printText(0, MAX_DEVICES-1, ctext);
    delay(200);
    if (answer[checked]==cin) {
      checked++;
    } else {
      String text_e=text_q+"EEE";
      text_e.toCharArray(ctext,text_e.length()+1);
      printText(0, MAX_DEVICES-1, ctext);
      delay(1000);
      Serial.println("EEE");
      question++;
      printed=false;
    }
    if (checked==answer.length()) {
      Serial.println("OK!");
      score++;
      question++;
      printed=false;
    }
  }
/*
  if (Serial.available() > 0) { // read char by char
    // read the incoming byte:
    char cin = (char)Serial.read();
    if (answer[checked]==cin) { 
      checked++;
    } else {
      Serial.println("EEE");
      question++;
      printed=false;
    }
    if (checked==answer.length()) {
      Serial.println("OK!");
      score++;
      question++;
      printed=false;
    }
  }
  */
  
  /*
  readSerial();
  if (newMessageAvailable) // checking answer
  {
    PRINT("\nProcessing new message: ", message);   
    newMessageAvailable = false;
    // TODO check - assume OK
    question++;
    printed=false;
  }
  */
}
