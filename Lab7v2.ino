#include "Timer.h"
#include "LiquidCrystal.h"
#include "pitches.h"

int cursorPosition = 0;
int currentSong = 0;
int pause = 0;
int playing = 0;
int currentInput = 0;

int sw = 10;
int button = 0;
bool buttonPressed = false;
bool startSelected = false;

int vrx = A0;
int vry = A1;
const int none = 0;
const int up = 1;
const int left = 2;
const int right = 3;
const int down = 4;

int readStick() { 
    if (analogRead(A1) > 800) {
        return 4; // down
    }
    else if (analogRead(A1) < 200) {
        return 1; // up
    }
    else if (analogRead(A0) > 800) {
        return 2; // left
    }
    else if (analogRead(A0) < 200) {
        return 3; // right
    }
    else {
        return 0;
    }
}

// Sound Variables
int buzzer = 3;
int song1_length = 20;
int song2_length = 20;
int song3_length = 20;


// == Song 1 ==

int song1[] = {
NOTE_E4, NOTE_C5, NOTE_B1, NOTE_F3, NOTE_C4, NOTE_A4, NOTE_A4, NOTE_GS5, NOTE_C5, NOTE_CS4, NOTE_AS4, NOTE_C5, NOTE_DS4, NOTE_CS5, NOTE_GS4, NOTE_C3, NOTE_E3, NOTE_DS5, NOTE_D4, NOTE_D3
};
int song1_time[] = {
2, 1, 2, 1, 1, 4, 8, 16, 8, 4, 4, 1, 8, 4, 2, 4, 4, 16, 4, 2
};

 int song2[] = {
// major scale
 NOTE_A4, NOTE_B4, NOTE_CS4,
 NOTE_D4, NOTE_E4, NOTE_FS4,
 NOTE_GS5, NOTE_A4, NOTE_GS5, 
 NOTE_FS4, NOTE_E4, NOTE_D4,
 NOTE_FS4, NOTE_E4, NOTE_D4,
 NOTE_CS4, NOTE_B4, NOTE_A4, 0, 0
};

int song2_time[] = {
  1, 1, 1, 1, 1, 1, 1, 8, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 8, 1, 1,
};


int song3[] = {
 // harmonic minor scale
 NOTE_A4, NOTE_GS3, NOTE_F3, 
 NOTE_D4, NOTE_E4, NOTE_F4,
 NOTE_GS4, NOTE_A4, 
 NOTE_GS4, NOTE_F4, NOTE_E4,
 NOTE_GS4, NOTE_F4, NOTE_E4, NOTE_D4,
 NOTE_C4, NOTE_B4, NOTE_A4, NOTE_GS3, NOTE_F3
 
};

int song3_time[] = {
1, 1, 1, 1, 1, 1, 1, 4, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

// LCD variables
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


void select()
{
  if(buttonPressed == true)
      {
        lcd.print("*");
        buttonPressed = false;
      }
}

void pos1()
{
  lcd.setCursor(6, 0);
  cursorPosition = 1;
}

void pos2()
{

  lcd.setCursor(7, 0);
  cursorPosition = 2;
}

void pos3()
{

  lcd.setCursor(6, 1);
  cursorPosition = 3;
}

void pos4()
{

  lcd.setCursor(7, 1);
  cursorPosition = 4;
}

void pos5()
{

  lcd.setCursor(6, 1);
  cursorPosition = 5;
}

void pos6()
{

  lcd.setCursor(7, 1);
  cursorPosition = 6;
}

void updatePos()
{
  if(cursorPosition == 1){pos1();}
  else if(cursorPosition == 2){pos2();}
  else if(cursorPosition == 3){pos3();}
  else if(cursorPosition == 4){pos4();}
  else if(cursorPosition == 5){pos5();}
  else if(cursorPosition == 6){pos6();}
}

// Task Structure Definition
typedef struct task {
   int state;                  // Tasks current state
   unsigned long period;       // Task period
   unsigned long elapsedTime;  // Time elapsed since last task tick
   int (*TickFct)(int);        // Task tick function
} task;


const unsigned char tasksNum = 4;
task tasks[tasksNum]; // We have 4 tasks

// Task Periods
const unsigned long periodLCDOutput = 100;
const unsigned long periodJoystickInput = 100;
const unsigned long periodSoundOutput = 100;
const unsigned long periodController = 500;

// GCD 
const unsigned long tasksPeriodGCD = 100;

// Task Function Definitions
int TickFct_LCDOutput(int state);
int TickFct_JoystickInput(int state);
int TickFct_SoundOutput(int state);
int TickFct_Controller(int state);

// Task Enumeration Definitions
enum LO_States {LO_init, first_screen, second_screen1, second_screen2, second_screen3, first_screen_wait, second_screen_wait};
enum JI_States {JI_init, check_press, release};
enum SO_States {SO_init, SO_SoundOn, SO_SoundOn2, SO_SoundOn3, SO_SoundOff};
enum C_States {C_init, C_wait, C_press, C_play, C_pause_press, C_pause};



void TimerISR() { // TimerISR actually defined here
  unsigned char i;
  for (i = 0; i < tasksNum; ++i) { // Heart of the scheduler code
     if ( tasks[i].elapsedTime >= tasks[i].period ) { // Ready
        tasks[i].state = tasks[i].TickFct(tasks[i].state);
        tasks[i].elapsedTime = 0;
     }
     tasks[i].elapsedTime += tasksPeriodGCD;
  }
}

void LCDWriteLines(String line1, String line2) {
  lcd.clear();          
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0,1);
  lcd.print(line2);
}

// Task Function Definitions

int menuOption = 0;

// Sound Output
int counter = 0;
int note = 0;

// Task 1
int TickFct_LCDOutput(int state) {
  switch (state) {
    case 0:
      state = first_screen;
      break;
    case first_screen:
      LCDWriteLines("Song 1  Song 2", "Song 3  Start");
      pos1();
      lcd.blink();
      state = first_screen_wait;
      break;
    case second_screen1:
      LCDWriteLines("Playing Song 1", "Pause    Play");
      pos5();
      lcd.blink();
      state = second_screen_wait;
      break;
    case second_screen2:
      LCDWriteLines("Playing Song 2", "Pause    Play");
      pos5();
      lcd.blink();
      state = second_screen_wait;
      break;
    case second_screen3:
      LCDWriteLines("Playing Song 3", "Pause    Play");
      pos5();
      lcd.blink();
      state = second_screen_wait;
      break;
      case first_screen_wait: //idle
  
      if (cursorPosition == 1 && readStick() == right)
      {
        pos2();
        
      }
      else if (cursorPosition == 1 && readStick() == down)
      {
        pos3();
      }
      else if (cursorPosition == 2  && readStick() == down)
      {
        pos4();
      }
      else if (cursorPosition == 2  && readStick() == left)
      {
        pos1();
      }
      else if (cursorPosition == 3  && readStick() == right)
      {
        pos4();
      }
      else if (cursorPosition == 3  && readStick() == up)
      {
        pos1();
      }
      else if (cursorPosition == 4  && readStick() == left)
      {
        pos3();
      }
       else if (cursorPosition == 4  && readStick() == up)
      {
        pos2();
      }

      if (buttonPressed && cursorPosition == 4 && currentSong != 0)
      {
        state = second_screen_wait;
        playing = 1;

        if (currentSong== 1){state = second_screen1;}
        else if (currentSong==2){state = second_screen2;}
        else if (currentSong==3){state = second_screen3;}
      }
      else if (buttonPressed && cursorPosition == 1)
      {
        currentSong = 1;
        pos2();
        lcd.print(" ");
        pos3();
        lcd.print(" ");

        pos1();
        select();
        updatePos();
      }
      else if (buttonPressed && cursorPosition == 2)
      {
        currentSong = 2;
        pos1();

        lcd.print(" ");
        pos3();
        lcd.print(" ");

        pos2();
        select();
        updatePos();
      }
      else if (buttonPressed && cursorPosition == 3)
      {
        currentSong = 3;
        pos1();
        lcd.print(" ");
        pos2();
        lcd.print(" ");

        pos3();
        select();
        updatePos();
      }
      buttonPressed = false;
      break;

      case second_screen_wait:

      if (cursorPosition == 5 && readStick() == right)
      {
        pos6();
      }
      else if (cursorPosition == 6 && readStick() == left) 
      {
        pos5();
      }

      if (buttonPressed && cursorPosition == 5)
      {
        pause = 1;
        playing = 0;
        pos6();
        lcd.print(" ");
        pos5();
        select();
        updatePos();
      }
      else if (buttonPressed && cursorPosition == 6)
      {
        pause = 0;
        playing = 1;
        pos5();
        lcd.print(" ");
        pos6();
        select();
        updatePos();
      }

      buttonPressed = false;

      if (note == 19)
      {
        state = first_screen;
      }
      break;
  }

  switch (state) { // no state actions
    case LO_init:
    break;
  }
  return state;
}

int input2 = 0;


// Task 2
int TickFct_JoystickInput(int state) 
{
  button = digitalRead(sw);
  switch (state) { // State Transitions
    case 0:
    state = check_press;
    break;
    case check_press:
    if (button == LOW)
    {
      state = release;
    }
    break;
    case release:
    if (button == HIGH)
    {
      buttonPressed = true;
      state = JI_init;
    }
    break;
  }

   switch (state) { // no State Actions
    break;
  }
  return state;
}


int TickFct_SoundOutput(int state) {


  switch (state) { // State Transitions
    case SO_init:
    {
     // Serial.println("In init...");
      state = SO_SoundOn;
    }
    break;
    case SO_SoundOn:
    {
     // Serial.println("In sound on\"1\"...");
      if(counter >= song1_time[note]) {
         state = SO_SoundOff;
         note++;
         counter = 0;
         note = note % 20;
      }
    }
    break;
    case SO_SoundOn2:
    {
      // Serial.println("In sound on2...");
      if(counter >= song2_time[note]) {
         state = SO_SoundOff;
         note++;
         counter = 0;
         note = note % 20;
      }
    }
    break;
    case SO_SoundOn3:
    {
      // Serial.println("In sound on3...");
      if(counter >= song3_time[note]) {
         state = SO_SoundOff;
         note++;
         counter = 0;
         note = note % 20;
      }
    }
    break;
    case SO_SoundOff:
    {
      
      //Serial.println("In sound off...");
      if (currentSong == 0 || pause || playing == 0)
      {
        state = SO_SoundOff;
      }
      else if (currentSong == 1)
      {
        state = SO_SoundOn;
      }
       else if (currentSong == 2)
      {
        state = SO_SoundOn2;
      }
      else if (currentSong == 3)
      {
        state = SO_SoundOn3;
      }
      counter++;
      counter = counter % 4;

      Serial.print("note: ");
      Serial.println(note);
      if (note == 19)
        {
          currentSong = 0;
          playing = 0;
        }
    }

    break;
    
  }
   switch (state) { // State Actions
    case SO_SoundOn:
      tone(8, song1[note], periodSoundOutput * song1_time[note]);
      counter++;
    break;
    case SO_SoundOn2:
      tone(8, song2[note], periodSoundOutput * song2_time[note]);
      counter++;
    break;
    case SO_SoundOn3:
      tone(8, song3[note], periodSoundOutput * song3_time[note]);
      counter++;
    break;
    case SO_SoundOff:
      noTone(8);
    break;

  }
  return state;
}

// Task 4 :used this for the checkpoints but ended up not using it for the lcd menu cursor stuff
int TickFct_Controller(int state) { 
  switch (state) {  
    case C_init:
    state = C_wait;
    break;
    case C_wait:
    break;
    case C_press:
    break;
    case C_play:
    break;
    case C_pause_press:
    break;
    case C_pause:
    break;
  }

   switch (state) { // State Actions
    case C_init:
    break;
    case C_wait:
    break;
  }
  return state;
}

void InitializeTasks() {
  unsigned char i=0;
  tasks[i].state = LO_init;
  tasks[i].period = periodLCDOutput;
  tasks[i].elapsedTime = tasks[i].period;
  tasks[i].TickFct = &TickFct_LCDOutput;
  ++i;
  tasks[i].state = JI_init;
  tasks[i].period = periodJoystickInput;
  tasks[i].elapsedTime = tasks[i].period;
  tasks[i].TickFct = &TickFct_JoystickInput;
  ++i;
  tasks[i].state = SO_init;
  tasks[i].period = periodSoundOutput;
  tasks[i].elapsedTime = tasks[i].period;
  tasks[i].TickFct = &TickFct_SoundOutput;
  ++i;
  tasks[i].state = C_init;
  tasks[i].period = periodController;
  tasks[i].elapsedTime = tasks[i].period;
  tasks[i].TickFct = &TickFct_Controller;

}

void setup() {
  // put your setup code here, to run once:
  InitializeTasks();

  TimerSet(tasksPeriodGCD);
  TimerOn();
  Serial.begin(9600);
  // Initialize Outputs
  lcd.begin(16, 2);
  // Initialize Inputs

  pinMode(vrx, INPUT);
  pinMode(vry, INPUT);
  pinMode(sw, INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
  // Task Scheduler with Timer.

  while(!TimerFlag) {
  }
  TimerFlag = 0;
}
