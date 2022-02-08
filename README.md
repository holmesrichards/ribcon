# Ribbon controller

This is an interface and associated software for a resistive ribbon controller. The Nano-based interface outputs the position of the contact on the ribbon controller as a voltage between 0 and 5 V (two copies of this voltage are output), and it outputs a gate which is on or off depending on whether there is contact or not with the ribbon.

The controller itself can be built along the lines of the one here:

[http://synthhacker.blogspot.com/2016/04/diy-ribbon-controller-cv.html](http://synthhacker.blogspot.com/2016/04/diy-ribbon-controller-cv.html)

Other resistive ribbons should also be usable.

The interface is based on my [dac/ino](https://github.com/holmesrichards/dac_ino) lite circuit board. Power is provided by a 9 V battery; there is a toggle switch to turn power off and on. The ribbon controller is connected via a cable to a 3.5 mm jack on the enclosure. In mine I used a TRS jack with the two ends of the controller connected to T and S and the "wiper" to R, though since only one end is used a TR cable could have been used.

In the interface box, the "wiper" is connected to an analog input pin on the Nano via the pad labeled A_A and one of the ends is connected to ground. The other end can float. The two CV outputs and the gate outputs are connected to output jacks.

The Nano's pullup resistor for the pot input pin is enabled, so when the ribbon is touched the voltage appearing on this pin will be

>    5kR_r/(kR_r+R_p)

where k is the position of the "wiper" in the range 0 to 1, R_r is the resistance of the ribbon (for my 500 mm SoftPot it is 20kΩ), and R_p is the pullup resistance. Since R_p is fixed, this voltage is always less than 5 V. By measuring the voltage for known values of k and R_r I was able to determine R_p is about 35 kΩ for my Nano. Then knowing R_r and R_p, the software can determine k.

When the ribbon is not touched the resistance from the wiper to the ends is infinite so the voltage on the pot pin is (about) 5 V. This tells the software to turn off the gate, which it turns on again when the input is far enough below 5 V. It takes a millisecond or two for the voltage to rise to above this threshold when contact is released, so the software delays a couple of milliseconds between samples and ignores the most recent sample in calculating the position.

In the software there are some constants to set:

* PA_DIM is the maximum number of samples to average over. 100 works well for me.
* PA_MIN_ENTRIES is the minimum number of samples to average over. I used 10.
* POT_MIN and POT_MAX are the smallest and largest digitized pot values corresponding to touching at the ends of the ribbon. Enable debug prints to determine this.
* R_PULLUP is the Nano's pullup resistor value.
* R_SOFT is the ribbon's resistance end to end.
* DELAY is the number of milliseconds to wait at the end of each time through the loop. I used 2.

The software simply sets up the Nano pins, including enabling the pullup resistor on the pot pin, and then does the following in the loop:

* Read the ADC value for the pot voltage.
* If the voltage is above POT_MAX+100, set gate to false and set number of entries to 0.
* Otherwise:
    * Set gate to true.
    * The ADC value is stored in an array. If number of entries is smaller than the array size (PA_DIM), increment it.
    * If number of entries is larger than PA_MIN_ENTRIES, average all but the most recent entry.
    * From the average ADC value a value for the pot position is computed.
* Write the pot position to both channels of the DAC, and gate to the gate output pin.
* Delay DELAY milliseconds.

If macro DBG is set nonzero, debug information is written to the serial monitor. (This is slow, so DBG should be set to 0 when not debugging.)

## GitHub repository

* [https://github.com/holmesrichards/ribcon](https://github.com/holmesrichards/ribcon)

## Submodules

This repo uses submodule dac_ino, which provides the dac/ino software library. That submodule in turn uses submodule aoKicad, which provides needed libaries for KiCad. To clone:  

```git clone git@github.com:holmesrichards/ribcon.git```  
```git submodule init```  
```git submodule update```  

Alternatively do

```
git clone --recurse-submodules git@github.com:holmesrichards/ribcon.git
```

Or if you download the repository as a zip file, you must also click on the "dac_ino" link on the GitHub page (it'll have "@ something" after it) and download it as a zip file which you can unzip into this repo's dac_ino directory. Then, if you want to use the dac_ino design files in KiCad, do similarly with the aoKicad link in the dac_ino repo. You can skip this part if you just want the dac_ino library to run the ribbon controller software. 

After doing one of the above, from the dac_ino folder, find dac_ino.h and dac_ino.cpp and copy them into a subfolder named "dac_ino" of your Arduino libraries folder.

If desired, copy the files from aoKicad to wherever you prefer (your KiCad user library directory, for instance, if you have one). Then in KiCad, go into Edit Symbols and add symbol library  

```aoKicad/ao_symbols```  

and go into Edit Footprints and add footprint library  

```aoKicad/ao_tht.```  