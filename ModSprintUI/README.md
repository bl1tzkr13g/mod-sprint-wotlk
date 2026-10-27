# Sprint UI for WoW 3.3.5a

This is a new addon implementation for the `mod-sprint-wotlk` server module. Copy the entire `ModSprintUI` directory to your WoW client's `Interface/AddOns/` directory. Restart the client or use `/reload`.

- Hold **Shift** to sprint by default (also supports Ctrl or Alt).
- Displays sprint energy from server messages `AC_SPRINT:<percent>`.
- `/sprintbind SHIFT`, `/sprintbind CTRL`, `/sprintbind ALT` choose the modifier.
- `/sprintlock` toggles meter dragging.
- `/sprintfixjump` manually checks for a Shift+Space conflict.

## Shift + Space
On login, when Shift is selected and SHIFT-SPACE has an existing action, the addon offers a **one-time consent dialog** to unbind *only* SHIFT-SPACE. It does not modify the regular SPACE jump binding or other Shift shortcuts. The choice is saved per character. If accepted during combat, the binding repair is postponed until combat ends.

## Notes
This is a new addon, not a copy of the upstream NSCairns Lua code. It currently supports modifier keys only. Test in-game with WoW 3.3.5a and an updated server module; WoW protected-action and chat-command behavior may vary by client.
