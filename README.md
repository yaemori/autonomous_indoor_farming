# Autonomous Indoor Farming Platform

## Introduction

This project aims to address the growing global food demand through sustainable and automated agricultural practices. The system is a hydroponic platform designed to monitor and regulate critical growth factors such as:

- **pH Levels** for plant health
- **EC/TDS** for precise nutrient delivery
- **Temperature and Humidity** for environmental control
- **Plant Growth Metrics** using distance and color sensors

## Components

The system integrates multiple sensors and hardware, including:

- **Sensors**: pH, TDS, DS18B20 (Temperature), DHT11 (Humidity & Temperature), TCS3200 (Color), HC-SR04 (Distance)
- **Actuators**: 12V Peristaltic Pumps, 5V & 12V Relays
- **Microcontrollers**: ESP8266-01, NodeMCU ESP8266 ESP-12E
- **Custom PCB**: Designed using EasyEDA for component integration

## Hardware Integration

- Custom PCB for compact wiring and reliability
- Manual assembly and soldering for precise connections
- Wi-Fi-enabled remote access
- Key processes validated through serial monitoring

## Control Algorithms & Automation

The platform automates nutrient delivery and environmental adjustments by collecting, evaluating, and controlling data from sensors and relays. This ensures optimal conditions for hydroponic plant growth.

## Experimental Results

- **Sensor Accuracy**: Real-time and precise readings of pH, EC, temperature, humidity, and color intensity
- **Automation**: Effective nutrient and pH management
- **Stability**: Long-term testing confirmed reliable operation despite environmental fluctuations

## Challenges

- Calibration difficulties requiring frequent adjustments
- Fragility of thin PCB wires leading to connection issues
- Limited microcontroller pins constraining sensor integration
- Defective sensors causing delays and testing complications

## Future Work

- **Enclosed Environment**: Develop a fully controlled environment for optimal growth
- **Water Integration**: Directly connect a water source to streamline hydration
- **Harvest Automation**: Introduce automated harvesting systems to enhance efficiency

## Final View

The system integrates all necessary components and demonstrates a robust, automated solution for indoor farming.

---

Developed by Fuat Erten, Zeynep Kılınçer, Agit Balcı, and Yağız Işık.
