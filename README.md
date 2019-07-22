# anomalies
a wiqid collection of free modules for [vcv rack](https://vcvrack.com/)

== 2hp chaotic lfo series ==

these are small lfo modules using strange attractors for chaotic outputs. a
strange attractor is a mathematical system of equations with chaotic
(unpredictable) solutions. these solutions seem to orbit certain points, but
they never repeat in exactly the same way. it is easiest to see what this means
by connecting two outputs to a scope in lissajous mode.

all these modules are intentionally set up the same way. there is a speed knob,
which controls the speed (since this concerns chaotic systems, frequency or
pitch seems an incorrect term). then there is a shape knob, which adjusts one
variable in the equations. finally there is an amplitude knob, which regulates
the range of output voltages. by default the amplification is set to a standard
bipolar ±5v modulation cv range. fully clockwise it goes up to ±10v, and
counterclockwise down to ±0.1v.

as we are using chaotic systems, the output is not guaranteed to stay within the
set range and will occasionally go higher or lower. especially the sakarya
module can be wildly chaotic and may crash, needing a restart.

the x, y, and z outputs use the output values from the equations, scaled and
offset to usable lfo voltages. the t output is my invention, as a "mysterious
fourth dimension", using the formula x+y-z, resulting in another chaotic
output.

== languor ==

this is a module which combines several chaotic lfos at different speeds. it is
inspired by the triple sloths eurorack module by nonlinear circuits.

== expanse ==

this is a resizable blank module in a wiqid color.
