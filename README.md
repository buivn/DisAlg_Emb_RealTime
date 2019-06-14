**Embedded System with Distributed Algorithm - RealTime Processing - TTCAN**\
This project is to control an Inverted Rotary Pendulum work at non-zero position input by combining State Space Control,  Distributed Algorithm, Embedded C, FreeRTOS and TTCAN . They all were performed on four Atmel AVR.\
- Physical IRP model
<p align="center">
  <img src="https://github.com/buivn/images/blob/master/IRPSieg2014/IRPPhysModel.png" width="250">
</p>

- Simulink Model:
<p align="center">
  <img src="https://github.com/buivn/images/blob/master/IRPSieg2014/SimulinkModel.png" width="250">
</p>

Function's structure of the control system is as figure following:
<p align="center">
  <img src="https://github.com/buivn/images/blob/master/IRPSieg2014/DistributedControl.png" width="250">
</p>
The communication's schedule among microControllers:
<p align="center">
  <img src="https://github.com/buivn/images/blob/master/IRPSieg2014/communicationSchedue.png" width="250">
</p>
- 1st microController - SEN-mu: handles with Sensor signal. After receiving Sensor's data in every 3 milliseconds, the data is then pre-processed to get four system states and transmit to other microController.
<p align="center">
  <img src="https://github.com/buivn/images/blob/master/IRPSieg2014/1ContTaskStru.png" width="250">
</p>
- 2nd microController - HMI-mu: handles human-machine interaction by Display component (LCD) and Button Matrix. On LCD, it shows the state and states of the system. For Button Matrix, it would process the input signal such as changing the reference input, on/off system, check the system's state.
<p align="center">
  <img src="https://github.com/buivn/images/blob/master/IRPSieg2014/2ContTaskStru.png" width="250">
</p>
- 3rd microController - PWM-mu: implements State-space control algorithm and doing Time Master function. It receives four states of IRP (2 angles, 2 angular velocities) and one additional reference input state from HMI-mu, which is then applied to the controlled formula. Its output is the amount of energy should be transmitted to the DC motor.
<p align="center">
  <img src="https://github.com/buivn/images/blob/master/IRPSieg2014/3ContTaskStru.png" width="250">
</p>
- 4th microController - MOTO-mu: It controlled the DC motor via H-bidge circuit with frequency of 14kHz.
<p align="center">
  <img src="https://github.com/buivn/images/blob/master/IRPSieg2014/4ContTaskStru.png" width="250">
</p>

<a/>
All four microController's Timer were synchronized in every 3 millisecond, which ensured all activities will run in real time. The result are shown in the figure following:
The response of Angle Phi - data from encoder 1:
<p align="center">
  <img src="https://github.com/buivn/images/blob/master/IRPSieg2014/alphaResponse_Sensor.png" width="250">
</p>
The response of Angle Phi - data from encoder 2:
<p align="center">
  <img src="https://github.com/buivn/images/blob/master/IRPSieg2014/phiResponse_Sensor.png" width="250">
</p>
The operation of IRP:
<p align="center">
  <img src="https://github.com/buivn/images/blob/master/IRPSieg2014/operState.png" width="250">
</p>
Link to project result: https://www.youtube.com/watch?v=YgHMSljbNwk&t=1s
