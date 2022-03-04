/*
 * dac/ino based ribbon controller
 * by Rich Holmes Feb 2022
 */

#include <dac_ino.h>
using namespace dcrd;         // dac/ino library

// Hardware setup
const int POT_PIN = A0;       // pin for softpot

// Parameters for conversions
const int PA_DIM = 100;         // number of samples to average
const int PA_MIN_ENTRIES = 10;  // wait for this many samples before setting output
const int POT_MIN =  18;        // max value seen
const int POT_MAX = 365;        // max value seen
const float R_PULLUP = 35.;     // kOhms
const float R_SOFT = 20.;       // kOhms
const int DELAY = 2;            // ms between samples
const int TOUCH_THRESH = POT_MAX+100;   // pot value above this means no touch
const float R_RATIO = R_PULLUP / R_SOFT;
const float POT_K_MIN = R_RATIO * POT_MIN / (1024. - POT_MIN);
const float POT_K_MAX = R_RATIO * POT_MAX / (1024. - POT_MAX);

// SoftPot values
unsigned pot = 0;               // Raw softpot value
float pot_smooth = 0;           // Smoothed softpot value
float pot_k = 0;                // Calculated position in [0, 1]

unsigned pa[PA_DIM];            // Array of stored values
int pa_idx = PA_DIM;            // Pointer to most recent entry
int pa_n_entries = 0;           // Number of fresh entries

// Output values
unsigned cvout = 0;             // CV to output
bool gate = false;              // gate to output

void setup()
{
  // Set up board object
  dac_inoBoard.begin();
  dac_inoBoard.writeGate(dac_ino::GateOutChannel::A, false); // start with gate off

  // Set pullup on pot pin
  pinMode (POT_PIN, INPUT_PULLUP);
}

void loop()
{
  // Read the pot
  pot = analogRead(POT_PIN);
  if (pot > TOUCH_THRESH)
    {
      // Pot value is high so it is not being touched
      // Turn off gate and reset array filling
      gate = false;
      pa_n_entries = 0;
    }
  else
    {
      // Write pot into array
      pa_idx = (pa_idx + 1) % PA_DIM;
      if (pa_n_entries < PA_DIM)
	pa_n_entries += 1;
      pa[pa_idx] = pot;
      
      // Nothing calculated until array we have PA_MIN_ENTRIES entries.
      // Then we take running average of all *except most recent*.
      // (If touch is lifted last entry may be anomalous.
      // It takes ~1-2 ms for value to rise above threshold.)
      
      if (pa_n_entries >= PA_MIN_ENTRIES)
	{
	  gate = true;                       // turn gate on
	  float sum = 0.0;                   // average the array
	  for (int i = pa_idx-pa_n_entries+PA_DIM;
	       i < pa_idx+PA_DIM;
	       ++i)   // all but most recent of last pa_n_entries entries
	    sum += pa[i%PA_DIM];
	  
	  pot_smooth = constrain (sum / (pa_n_entries-1), 0, 1023); // smoothed pot value
	  pot_k = R_RATIO * pot_smooth / (1024. - pot_smooth);
	  pot_k = (pot_k - POT_K_MIN) / (POT_K_MAX - POT_K_MIN); // k (position in range 0 to 1) computed
	  cvout = constrain (4096 * pot_k, 0, 4095);  // CV to output
	}
    }

  // Update DAC and gate
  dac_inoBoard.writeCV(dac_ino::CVOutChannel::A, cvout);      
  dac_inoBoard.writeCV(dac_ino::CVOutChannel::B, cvout);      
  dac_inoBoard.writeGate(dac_ino::GateOutChannel::A, gate);

  delay (DELAY);
}
