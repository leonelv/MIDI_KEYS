#include "MIDIUSB.h"

// Declare Buttons
const int buttons = 12;
const int input[buttons] = {9, 8, 7, 6, 5, 4, A2, A1, A0, 15, 14, 16};

int buttonCurrentState[buttons] = {0};
int buttonPrevState[buttons] = {0};

// Debounce
unsigned long lastDebounceTime[buttons] = {0};
unsigned long debounceTime = 250;

// MIDI Settings
int noteMin = 0;
int noteMax = 127;
byte ch = 0;
byte note = 60;

/* // // Functions // // */

// setup
void setup()
{
  Serial.begin(115200);
  pinMode(0, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(10, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(0), cut, LOW);
  attachInterrupt(digitalPinToInterrupt(3), octaveDown, LOW);
  attachInterrupt(digitalPinToInterrupt(2), octaveUp, LOW);
  for (size_t i = 0; i < buttons; i++)
  {
    pinMode(input[i], INPUT_PULLUP);
  }
}

// loop
void loop()
{
  interrupts();
  keys();
  volume();
}

// Keyboard
int keys()
{
  for (size_t i = 0; i < buttons; i++)
  {
    buttonCurrentState[i] = digitalRead(input[i]);

    if (millis() - lastDebounceTime[i] > debounceTime)
    {
      if (buttonCurrentState[i] != buttonPrevState[i])
      {
        if (buttonCurrentState[i] == LOW)
        {
          noteOn(ch, note + i, 100);
          MidiUSB.flush();
          return note + i;
        }
        else
        {
          noteOff(ch, note + i, 100);
          MidiUSB.flush();
          return -1;
        }
        buttonPrevState[i] = buttonCurrentState[i];
      }
    }
  }
}

//cut
void cut()
{
  int note_val = keys();
  if (note_val >= 0)
  {
    MidiUSB.flush();
    noteOn(ch, note_val, 100);
    MidiUSB.flush();
  }

  if (note_val == -1)
  {
    MidiUSB.flush();
    noteOff(ch, note_val, 100);
    MidiUSB.flush();
  }
}
// Light control
void volume()
{
  int val = map(MIDIread(), 0, 127, 0, 255);
  Serial.println(val);
  analogWrite(10, val);
}

// Octave control

void octaveDown()
{
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > 200)
  {
    if (note - buttons >= noteMin)
    {
      note -= buttons;
    }
  }
  last_interrupt_time = interrupt_time;
}

void octaveUp()
{
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > 200)
  {
    if (note + buttons < noteMax)
    {
      note += buttons;
    }
  }
  last_interrupt_time = interrupt_time;
}

int MIDIread()
{
  midiEventPacket_t rx;
  int val;
  do
  {
    rx = MidiUSB.read();
    if (rx.header != 0)
    {
      if (rx.header == 0xB && rx.byte1 == 0xB0)
      {
        val = rx.byte3;
        /*Serial.print("Received: ");
        Serial.print(rx.header, HEX);
        Serial.print("-");
        Serial.print(rx.byte1, HEX);
        Serial.print("-");
        Serial.print(rx.byte2, HEX);
        Serial.print("Value :");
        Serial.println(val);*/
        return val;
      }
    }
  } while (rx.header != 0);
}

// MIDI Functions
void controlChange(byte channel, byte control, byte value)
{
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

void noteOn(byte channel, byte pitch, byte velocity)
{
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity)
{
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}
