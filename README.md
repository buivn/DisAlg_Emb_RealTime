**Embedded System with Distributed Algorithm - RealTime Processing - TTCAN**\
This project is to control an Inverted Rotary Pendulum work at non-zero position input by combining State Space Control,  Distributed Algorithm, Embedded C, FreeRTOS and TTCAN . They all were performed on four Atmel AVR.\
Physical IRP model
<p align="center">
  <img src="https://github.com/buivn/images/blob/master/IRPSieg2014/IRPPhysModel.png" width="300">
</p>

Simulink Model:
<p align="center">
  <img src="https://github.com/buivn/images/blob/master/IRPSieg2014/SimulinkModel.png" width="370">
</p>

Function's structure of the control system:
<p align="center">
  <img src="https://github.com/buivn/images/blob/master/IRPSieg2014/DistributedControl.png" width="330">
</p>
The communication's schedule (TTCAN) among microControllers:
<p align="center">
  <img src="https://github.com/buivn/images/blob/master/IRPSieg2014/communicationSchedue.png" width="300">
</p>
- 1st microController - SEN-mu: handles with sensor signals. After receiving sensor's data in every 3 milliseconds, the data is then pre-processed to get four system states and transmit to other microController.
<p align="center">
  <img src="https://github.com/buivn/images/blob/master/IRPSieg2014/1ContTaskStru.png" width="300">
</p>
- 2rd microController - PUSB-mu: implements State-space control algorithm and handle the input command from Button Matrix. It receives four states of IRP (2 angles, 2 angular velocities) and one additional reference input state from HMI-mu, which is then applied to the controlled formula. Its output is the amount of energy should be transmitted to the DC motor. For Button Matrix, it would process the input signal such as changing the reference input, on/off system, check the system's state.
<p align="center">
  <img src="https://github.com/buivn/images/blob/master/IRPSieg2014/3ContTaskStru.png" width="300">
</p>
- 3nd microController - LCD-mu: Display system states and working condition on LCD. It also works as Time Master which transmits Time Point to all other microController to synchronize the clock in the whole system.
<p align="center">
  <img src="https://github.com/buivn/images/blob/master/IRPSieg2014/2ContTaskStru.png" width="300">
</p>

- 4th microController - MOTO-mu: It controlled the DC motor via H-bidge circuit with frequency of 14kHz.
<p align="center">
  <img src="https://github.com/buivn/images/blob/master/IRPSieg2014/4ContTaskStru.png" width="300">
</p>

<a/>
All four microController's Timer were synchronized in every 3 millisecond, which ensured all activities will run in real time. The result are shown in the figure following:
The response of Angle Phi - data from encoder 1:
<p align="center">
  <img src="https://github.com/buivn/images/blob/master/IRPSieg2014/alphaResponse_Sensor.png" width="300">
</p>
The response of Angle Alpha - data from encoder 2:
<p align="center">
  <img src="https://github.com/buivn/images/blob/master/IRPSieg2014/phiResponse_Sensor.png" width="300">
</p>
The operation of IRP:
<p align="center">
  <img src="https://github.com/buivn/images/blob/master/IRPSieg2014/operState.png" width="150">
</p>
For more detail, please read my master thesis report attached on this repo.\
Link to project result: https://www.youtube.com/watch?v=YgHMSljbNwk&t=1s
