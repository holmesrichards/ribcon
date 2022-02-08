Define R_p and R_r as the resistances of the Arduino pullup resistor and the softpot respectively.

Then for a voltage divider with R_p on top and kR_r on the bottom, 

V_out = V_in(kR_r/(kR_r+R_p)

or

(k+r)v = k

where V_out = V_in(ADC/1024), v = V_out / V_in = ADC/1024 and r = R_p/R_r. Then k = rv/(1-v) (for v≠1) = r(ADC/(1024-ADC)) (for ADC≠1024).

R_r can be measured directly and v is known. Then R_p = rR_r = kR_r(1/v-1) (for v≠0) = kR_r(1024/ADC-1). 

For 200 mm Softpot, R_r should be about 100k R_p is about 20k, guaranteed 20k–50k.


