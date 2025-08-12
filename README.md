# README

## Summary
This repo contains a snapshot of the source code for an ongoing project called DDKnockoff. The general idea of the project is to mimic the original Dungeon Defenders by Trendy Entertainment. The project is not for commercial use, and is presented purely for educational and referential purposes. 

**NOTE** 
The project is not intended to be usable beyond code examination. The Content folder, containing many key blueprints and assets, is not included. Reference for what the game looks like in its current state will be made available at some point on my website, www.samfebvre.com. 

---

## Engine Version
Developed with **Unreal Engine 5.5.4**.

---

## Quick Start / Opening the Project
1. Clone the repository:
   ```bash
   git clone https://github.com/YourUsername/DDKnockoff.git

2. Right-click DDKnockoff.uproject → Generate Visual Studio project files

3. Open DDKnockoff.sln in Visual Studio (or your IDE of choice).

4. Build the project from the IDE, then launch from Unreal Engine.

5. The project will open without assets and is intended for code reference only.

---

## Included Folder Structure
DDKnockoff/
├── .github/           # GitHub Actions / automation workflows
├── Config/            # Default Unreal config files
├── Source/            # All gameplay, editor, and test source code
│   ├── DDKnockoff/        # Runtime module
│   ├── DDKnockoffEditor/  # Editor module
│   └── DDKnockoffTests/   # Automation test module
├── .gitattributes
├── .gitignore
├── DDKnockoff.uproject
└── README.md

---

## What's in the project so far?

### Core Modules
- **DDKnockoff** (Runtime) - Main game module containing all gameplay code
- **DDKnockoffEditor** (Editor) - Editor-specific functionality and tools. At the time of writing, this is mostly empty.
- **DDKnockoffTests** (DeveloperTool) - Automation tests using UE's testing framework

### Key Systems
- Manager Architecture - Core systems are spun up and accessed in a controlled and scalable manner to work smoothly during regular gameplay and test environments.
- Entity System - A basic entity system is established to handle game logic in a robust and scalable way. 
- Combat - damage is handled with a payload based system that all entities can access in a shared manner. Hitboxes and Hurtboxes are explicitly marked up.
- Defensive structures - 5 different defensive structures are fully implemented with attacks and targeting.
- Structure placement system - structure placement is validated before confirmation.
- Basic enemy AI system - enemies path towards the player's base and attack defences they find along the way. 
- Animation - player and enemies possess rudimentary animation control, including basic locomotion, attacks, and hit reactions.
- Currency system - management and spawning of physics-based in-world currency for the player to collect and spend on defences.
- Wave manager system - enemy spawns are controlled in a data-driven way using data assets to control enemy type, count, and spawn location per wave.
- Basic win/loss condition. Enemies kill the crystal causes a game loss and restart. Defeating every wave causes a game win and restart.
- Debug information system - an interface is provided for classes that want to provide debug information. This is displayed on screen and is globally toggleable. Basic use cases include current wave number, remaining enemies, player stats, etc. 
- Basic interaction system - the foundations of an interaction system for the player are present. This is currently used to allow the player to interact with chests and get the currency inside.
- Automated Testing - a suite of spec tests that test various aspects of the game - including basic asset and settings validation, critical gameplay feature testing, and more expansive/specific testing for individual defences.
- Basic CI/CD - a simple GitHub actions automation is included that builds the game and runs all of its tests every time a change is pushed to the main branch.

**A note on the automated tests**. 
The spec tests may seem to be written in a strange way, with odd formatting and boilerplate comments. They were written this way alongside the development of a tool to generate functional tests automatically from spec tests. The core idea was to be able to write tests in a spec format, while also getting the benefit of functional tests (IE - the ability to watch what's actually happening during a test that may be causing it to fail). The solution I opted to explore required some explicit markup and formatting enforcement that resulted in this somewhat wonky style of test writing. This tool, and the respective generated functional, tests, are not included in this repo. 



