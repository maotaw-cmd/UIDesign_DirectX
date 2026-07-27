# Sound Walk popup fix

The Sound Walk controls are now implemented in one dedicated file:

- `src/UI/SoundWalkSettings.hpp`

On the Visuals page, the Sound Walk row displays controls in this order:

1. settings gear
2. colour-picker icon
3. toggle

Click the gear to open the draggable Sound Walk settings popup. It contains:

- Animation style
- Speed
- Expansion
- Line width

The main Settings page does not render any Sound Walk animation controls.
