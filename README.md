**Embedded System with Distributed Algorithm - RealTime Processing - TTCAN**\
This project is to control an Inverted Rotary Pendulum work at non-zero position input by combining State Space Control,  Distributed Algorithm, Embedded C, FreeRTOS and TTCAN . They all were performed on four Atmel AVR.\
- First microController - MOTO-mu: It controlled the DC motor via H-bidge circuit with frequency of 14kHz.
- Second micrController - LCD-mu: It controlled the Display component - LCD to show the state and condition of the system.
- Third microController - PUSH-mu: It handled the input signal such as changing the reference input, on/off system, check the system's state.
- Fourth microController - Control-mu: implement State-space control algorithm. It processed two input data from two sensors to calculate the system's states (2 angles, 2 angular velocities), which is then applied to control formula. Its output is the amount of energy should be transmitted to the DC motor.
<a/>
All four microController's Timer were synchronized in every 3 millisecond, which ensured all activities will run in real time.

Link to project result: https://www.youtube.com/watch?v=YgHMSljbNwk&t=1s
