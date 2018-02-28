#include "MIDIUSB.h"

// Declare Buttons
const int buttons = 12;
const int input[buttons] = {9, 8, 7, 6, 5, 4, A2, 16, 14, 15, A0, A1};
int buttonCurrentState[buttons] = {0};
int buttonPrevState[buttons] = {0};

// Debounce
unsigned long lastDebounceTime[buttons] = {0};
unsigned long debounceTime = 5;

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
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(10, OUTPUT);
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
void keys()
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
        }
        else
        {
          noteOff(ch, note + i, 100);
          MidiUSB.flush();
        }
        buttonPrevState[i] = buttonCurrentState[i];
      }
    }
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
