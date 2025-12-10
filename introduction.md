# Introduction

The goal of this project is to build a guitar tuner capable of sensing the vibrations of a guitar string and providing clear feedback to the player on whether the string should be tuned higher or lower. Beyond the basic tuning functionality, our team also aims to explore mechanical tuning—automatically twisting the tuning knob—as well as generating a “target tone” that the user can match by ear.

## Motivation

As musicians, we wanted to pursue a project that combined our musical interests with computer science. A guitar tuner offered a practical, compact, and meaningful project that we could see ourselves using in everyday life.  
Gerson plays bass guitar, Charlie plays electric guitar, and Edy sings and plays acoustic guitar, making this project a natural fit for our team’s shared interests.

## Planned Approach

Our tuner will support three distinct modes:

### 1. Standard Pitch Detection Mode  
Similar to a SuperSnark clip-on tuner, the device will detect the pitch of the vibrating string and provide visual feedback indicating whether the note is flat, sharp, or in tune. The user adjusts the tuning knob manually based on the displayed feedback.

### 2. Motor-Assisted Tuning Mode  
In this mode, the user will select the pitch but also activate a motor to automatically twist the guitar’s tuning knob. The motor will turn in the direction needed to bring the string closer to the correct pitch.

### 3. Pitch Fork Tone Mode  
The user will select the string being tuned and play the corresponding target pitch through a speaker. The user can then tune by ear, matching the guitar string to the generated tone.

