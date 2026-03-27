# PE2_Code

Repository voor het delen van STM32CubeIDE en STM32CubeMX projectcode van een zelfgebouwde drone.

## Projectstructuur

De repository bevat twee afzonderlijke STM32CubeIDE/CubeMX projecten:

| Map | Omschrijving |
|-----|-------------|
| `ESC/` | Code voor de Electronic Speed Controller (ESC) van de drone |
| `FlightController/` | Code voor de flight controller van de drone |

## Aan de slag

1. Clone de repository:
   ```
   git clone https://github.com/Simon-VDVelde/PE2_Code.git
   ```
2. Open STM32CubeIDE.
3. Importeer het gewenste project via **File → Import → Existing Projects into Workspace** en selecteer de map `ESC/` of `FlightController/`.
4. Het `.ioc` bestand kan geopend worden in STM32CubeMX om de pinout en klokken te bekijken of aan te passen.

## Hardware

- **ESC-project**: draait op een STM32-microcontroller die via PWM de motorsnelheden aanstuurt.
- **FlightController-project**: draait op een STM32-microcontroller die sensordata verwerkt en commando's doorstuurt naar de ESC.