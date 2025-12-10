# Issues and Challenges

## Microphone Reliability

One of the first issues we encountered was a faulty external microphone. While we were able to get it working intermittently, the connection was unreliable and required frequent troubleshooting. Because this significantly slowed development, we ultimately decided to switch to using the internal microphone on the Circuit Playground Classic. Although the internal microphone had some limitations, it proved to be much more stable and allowed us to continue development without constant hardware issues.

---

## Pitch Detection Difficulties

Pitch detection required substantial experimentation and iteration. Our initial approach attempted to use a frequency-domain correlation-based method, but this approach was highly sensitive to background noise and produced unstable results in non-ideal environments. Because of this, it was difficult to reliably isolate the fundamental frequency of the guitar string.

We eventually transitioned to an autocorrelation-based pitch detection method, which proved to be more robust for monophonic signals like individual guitar strings. Although this approach still required careful parameter tuning, it significantly improved pitch stability and accuracy compared to our initial method.

---

## OLED Display Configuration

Configuring the OLED display presented another major challenge. Documentation and datasheets for the display and the Circuit Playground pinouts were sometimes inconsistent or unclear, making it difficult to determine correct wiring and configuration. As a result, the display did not work initially.

Through trial, error, and systematic experimentation, we were able to determine a working pin configuration and successfully initialize the display. Once correctly configured, the OLED performed reliably and became a central part of the user interface.

---

## Program Storage Limitations

The Circuit Playground Classic has a very limited amount of program storage, which imposed constraints on software complexity. As the project grew, we encountered compilation failures due to insufficient memory.

To address this, we removed unused fonts, reduced display assets, and eliminated non-essential code paths. These trade-offs allowed us to retain core functionality—pitch detection, display output, and motor control—while staying within the available memory limits.

## Motor Strength

The motor was a component that we wished to use from the start, that being said, one thing that we didn't calculate was how strong our motor eneded to be to tune the pegs of a guitar. As it is currently, the motor doesn't have nearly enough strength to tune a guitar which is quite important for accessibility. 

Even if it does detect the current pitch and moves based on that due to the lacking strength of the components it does not work for it's current purpose.

---

---

Overall, these challenges shaped the final design of the project and required us to make practical engineering trade-offs between functionality, reliability, and hardware constraints.
