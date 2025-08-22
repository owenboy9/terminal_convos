# Pomodoro CLI Timer

A simple command-line Pomodoro timer written in C, complete with sound cues and clean modular design.

## 📦 Features

- Interactive prompt for work time, break time, and number of rounds.
- Console-based countdown display.
- Audio cues for:
  - Start of work session
  - Start of break
  - End of the Pomodoro cycle
- Modular architecture (`main`, `timer`, `ui`, `sound`).
- Build automation using `Makefile`.

## 🛠️ Requirements

- GCC or compatible C compiler
- `mpg123` for playing sound (`sudo apt install mpg123` on Debian/Ubuntu)
- Unix-like environment (Linux/macOS)

## 🧱 Project Structure

pomodoro/
├── src/ # Source files
├── include/ # Header files
├── sounds/ # .mp3 audio cues
├── build/ # Compiled .o files (generated)
├── dist/ # Output binary + sounds (after install)
├── Makefile # Build automation
└── README.md # This file


## ⚙️ Build Instructions

```bash
make

This compiles all source files and links them into a pomodoro binary.

📦 Installation

To copy the compiled binary and sound files into a dist/ directory:

```bash
make install

▶️ Running the Timer

```bash
make run

Or manually:

```bash
./dist/pomodoro

🔁 Example Session

okay, time to start!
enter work session duration in minutes: 25
enter break duration in minutes: 5
enter number of rounds: 4

work session 1
study for 25 minutes...
...
done for the day!

🧹 Clean Up

```bash
make clean

Deletes compiled objects, the final binary, and dist/.