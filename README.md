# OTAValveMaster

OTAValveMaster is the master-side controller for an OTA-enabled valve network. It receives user requests, validates and parses them, initiates order execution, drives the actuator logic, and sends acknowledgment back to the requesting node.

## Project structure

- main.c: Arduino application entry (`setup()` and `loop()`) with a host-only simulation wrapper behind `OTA_HAL_SIMULATION`
- comm.c / comm.h: request parsing, validation, CRC checks, order initiation, and acknowledgment flow
- netintr.c / netintr.h: HAL interface initialization and transport send/receive logic with a simulation/real split via compile-time define
- motor.c / motor.h: valve actuation logic
- options.h: compile-time configuration and limits
- CMakeLists.txt: simulation-friendly build configuration for host testing

## Execution flow

1. The application initializes the interfaces.
2. The master waits for a user request.
3. The request is validated and parsed into the archive.
4. The order is initiated and sent to the valve node.
5. The valve node sends an acknowledgement back to the master.
6. The master posts an aggregated acknowledgment and continues the cycle.

This flow remains aligned with the original design. The work here focused on correcting broken prototypes, type mismatches, incomplete stubs, and the HAL layer so the send/receive and CRC path is explicit and configurable for simulation or real targets.

## Build and run

### Simulation build

From the project root:

cmake -S . -B build
cmake --build build
./build/ota_valve_master

### Arduino build

This project is structured for the Arduino framework. The real application entry is `setup()` and `loop()`. The simulation path is only used when `OTA_HAL_SIMULATION` is enabled to provide a host-side `main()` wrapper for testing and local execution.

## Notes

- The transport layer is intentionally split using the compile-time define `OTA_HAL_SIMULATION`.
- The simulation path is isolated to the HAL/transport layer only; the Arduino framework entry stays as `setup()` and `loop()`.
- There is no ESP32-specific boot code in the application layer.
