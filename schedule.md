# Planned Team Work Schedule

---

##  Weekly Timeline

### **Week 8**
- Wait for materials to arrive  
- Preemptively read datasheets and pinouts  
- Assemble the circuit board  

### **Weeks 9–10**
- Detect the correct tone  
- Implement a fully functional display  
- Make the motor turn the tuning peg  
- Occasional check-ins to ensure steady progress  

### **Week 11**
- System testing and debugging  

### **Week 12**
- Prepare final demo  
- Write project report  
- Prepare presentation  

---

## Task Breakdown by Responsibility

### **Board Assembly & Hardware Setup**
- Assemble the board  
- Determine how to connect each component by reading datasheets and pinouts  
- Identify correct operating voltages for all components  
- Determine required resistors to achieve target voltages  

---

### **Tone Detection & Signal Processing**
- Detect the correct tone from the guitar string  
- Read microphone input into the Circuit Playground  
- Detect which pitch the user is attempting to tune to  
- Determine whether the current pitch is sharp or flat  
- Communicate pitch information via Serial Port to a laptop as an initial proof of concept  
- Final goal: display pitch information directly on the OLED  

---

### **Display / UI Development (OLED)**
**Edy**

- Program and manage the OLED display  
- Display the detected pitch  
- Use color to indicate how close the tone is to being in tune  
- Show battery life status  

---

### **Motorized Tuning System**
**Gerson & Charlie**

- Automatically turn the motor based on detected pitch (Gerson)  
- Synchronize motor movement to the relative pitch of each string  
- Design a method to activate the motor  
  - Holding a button  
  - Automatic activation via microphone input  

---

### **Testing & Validation**
**Charlie**

- Bring a guitar and a commercial tuner for reference  
- Verify pitch accuracy against a known-good tuner  
- Test accuracy across different pitch ranges  
  - Lower strings  
  - Higher strings
---
 
### **Actuality of the Schedule**
- In the end our schedule ended up changing dramatically throughout the project. We simply met up when we were available and set goals dependant on the day. For example, we would schedule a meeting on simply learning how to turn on the display and work with making the microphone detect pitch. These meetings ended up being around 2-3 hours dependant on the task. There were times in which we were not able to finish the goal by the meeting so we would disperse and end up doing parts by ourselves. We have a fairly efficient workflow overall, however we did struggle a bit with scheduling things near the end due to underestimating certain tasks. Each of us ended up working on most of the components together, even more than expected.
- Edy and Gerson focused a lot of the oled, Gerson with figuring how to turn it on alongside the functionality, while Edy worked a lot on the UI and Gerson helped create a lot of the initial states of our machine.
- The group as a whole didn't figure out whether to use the external microphone or the internal one from the circuit playground till near the end due to the difficulties of making it function with the current setup.
- A lot of time near the end was spent combining the code from the components together to create two different sketches that functioned together without bugs. 

[Introduction](introduction.md) |
[Methods](Methods.md) |
[Results](results.md) |
[Schedule](schedule.md) |
[Issues](issues.md) |
[Ethics](ethics.md) |
[Accessibility](accessibility.md) |
[References](references.md)



