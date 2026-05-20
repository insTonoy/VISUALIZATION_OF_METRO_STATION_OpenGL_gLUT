# 🚇 Dhaka Metro MRT-6 Visualization

> **An interactive 2D simulation of Dhaka’s MRT-6 line** – featuring three dynamic scenes: outdoor cityscape, ticket vending interior, and platform with train arrival. Built with OpenGL and GLUT to demonstrate real‑time animation, scene management, and keyboard interaction.

---

## 🎬 Scenes

| Scene | Description |
|-------|-------------|
| **1 – Outdoor** | Elevated metro track, moving train, cars, clouds, station building, stairs, trees, lamp posts, billboard, and a complete day/night cycle. |
| **2 – Ticket Vending** | Interior hall with three ticket machines, animated buyers forming a queue, touchscreen simulation, bill insertion, ticket collection, and speech bubbles. |
| **3 – Platform** | Train arrival/departure, sliding platform doors, waiting passengers boarding, real‑time station clock, and scrolling digital billboards. |

---

## 🎮 Features

- 🕹️ **Manual scene switching** – press `1`, `2`, or `3` to jump between scenes instantly.
- 🔁 **Automatic transition** – each scene runs its own update loop independently.
- ☀️ **Dynamic day/night cycle** – sun, moon, stars, and clouds move realistically.
- 💬 **Animated speech bubbles** – passengers show their destination and fare during purchase.
- 🚆 **Real‑time train movement** – metro travels along overhead tracks; doors open at platform.
- 🔥 **Emergency simulation** – toggle fire alarm with strobe light and red tint (Scene 2).
- 🧍 **Crowd simulation** – multiple NPCs walk, queue, interact with machines, and board the train.

---

## 🎛️ Controls

| Key | Action |
|-----|--------|
| `1` / `2` / `3` | Switch between Outdoor / Ticket Vending / Platform scenes |
| `F` | Toggle fire alarm (Scene 2 only) |
| `R` | Reset all passengers and queue (Scene 2 only) |
| `ESC` | Exit the program |

> **Note:** Scene 1 has no interactive keys – it runs autonomously.

---

## ✅ What I Learned

- **2D orthographic rendering** with OpenGL – drawing primitives, blending, and smooth lines.
- **Real‑time animation** using `glutTimerFunc` and delta‑time (`dt`) for frame‑independent motion.
- **Merging multiple independent programs** into one unified framework (namespace separation + global scene state).
- **Keyboard interaction and scene switching** – handling input and resetting projection matrices.
- **Basic UI elements** – touchscreen simulations, HUD text panels, speech bubbles, and fire alarm visuals.
- **State machine design** – each person, train, and queue follows a phase‑based logic.

---

## 📚 Dependencies & Build

- **OpenGL** (any version supporting `gluOrtho2D`)
- **GLUT** – freeglut or the original GLUT library
- **C++11** or later

# For any queries:
Email : 23-54682-3@student.aiub.edu or tonoyt735@gmail.com
[Linkedin](https://www.linkedin.com/in/t0n0y/)
