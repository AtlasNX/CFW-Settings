#pragma once

#include "gui.hpp"
#include "override_key.hpp"


class GuiOverrideKey : public Gui {
public:
  GuiOverrideKey();
  ~GuiOverrideKey();

  void update();
  void draw();
  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);
  void onGesture(touchPosition &startPosition, touchPosition &endPosition);

private:
  ProgramOverrideKey m_override{};
  bool m_inputBlocked = false;
};
