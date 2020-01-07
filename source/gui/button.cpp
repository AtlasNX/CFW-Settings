#include "button.hpp"

extern "C" {
  #include "theme.h"
}

Button::Button(u16 x, u16 y, u16 w, u16 h, std::function<void(Gui*, u16, u16, bool*)> drawAction, std::function<void(u32, bool*)> inputAction, std::vector<s32> adjacentButton, bool activatable, std::function<bool()> usableCondition)
 : m_x(x), m_y(y), m_w(w), m_h(h), m_drawAction(drawAction), m_inputAction(inputAction), m_usableCondition(usableCondition), m_activatable(activatable), m_adjacentButton(adjacentButton){
  Button::g_buttons.push_back(this);

  m_isActivated = false;
  m_isSelected = false;

  select(0);
}

s16 Button::getSelectedIndex() {
  for(size_t i=0; i != g_buttons.size(); ++i) {
    if (g_buttons[i]->m_isSelected)
      return i;
  }
  return -1;
}

void Button::select(s16 buttonIndex) {
  if (buttonIndex < 0) return;
  if (Button::g_buttons.size() <= static_cast<u16>(buttonIndex)) return;

  for(Button *btn : Button::g_buttons) {
    btn->m_isSelected = false;
    btn->m_isActivated = false;
  }

  auto button = Button::g_buttons[buttonIndex];
  button->m_isSelected = true;

  if (scrollBlocked)
    return;

  auto leftmostDiff = (button->m_x) - (pageOffsetX) - pageLeftmostBoundary;
  auto topmostDiff = (button->m_y) - (pageOffsetY) - pageTopmostBoundary;
  auto rightmostDiff = (button->m_x + button->m_w) - (pageOffsetX) - pageRightmostBoundary;
  auto bottommostDiff = (button->m_y + button->m_h) - (pageOffsetY) - pageBottommostBoundary;

  if (leftmostDiff < 0)
    pageOffsetX += leftmostDiff;

  if (rightmostDiff > 0)
    pageOffsetX += rightmostDiff;

  if (topmostDiff < 0)
    pageOffsetY += topmostDiff;

  if (bottommostDiff > 0)
    pageOffsetY += bottommostDiff;
}

void Button::draw(Gui *gui) {
  // Offset calculation
  s32 resultX = m_x - targetOffsetX;
  s32 resultY = m_y - targetOffsetY;
  if (resultX + m_w < 0 || resultY + m_h < 0 || resultX > SCREEN_WIDTH || resultY > SCREEN_HEIGHT)
    return;
  s32 borderX = resultX - 5;
  s32 borderY = resultY - 5;
  if(m_isSelected) {
    gui->drawRectangled(borderX, borderY, m_w + 10, m_h + 10, m_isActivated ? currTheme.activatedColor : currTheme.highlightColor);
    gui->drawShadow(borderX, borderY, m_w + 10, m_h + 10);
  } else gui->drawShadow(resultX, resultY, m_w, m_h);

  gui->drawRectangled(resultX, resultY, m_w, m_h, currTheme.selectedButtonColor);

  m_drawAction(gui, resultX, resultY, &m_isActivated);

  if (!m_usableCondition())
    gui->drawRectangled(resultX, resultY, m_w, m_h, gui->makeColor(0x80, 0x80, 0x80, 0x80));
}

bool Button::onInput(u32 kdown) {
  if (!m_isSelected)
    return false;

  if (!m_isActivated) {
    if ((kdown & KEY_A) && m_activatable) {
      m_isActivated = true;
      kdown = 0;
    } else {
      if (kdown & KEY_UP)    Button::select(m_adjacentButton[0]);
      if (kdown & KEY_DOWN)  Button::select(m_adjacentButton[1]);
      if (kdown & KEY_LEFT)  Button::select(m_adjacentButton[2]);
      if (kdown & KEY_RIGHT) Button::select(m_adjacentButton[3]);
      if (kdown & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) return true;
    }
  }

  if(!m_usableCondition()) return false;

  m_inputAction(kdown, &m_isActivated);

  return false;
}

void Button::onTouch(touchPosition &touch) {
  if (!m_usableCondition()) return;

  if (touch.px >= m_x && touch.px <= (m_x + m_w) && touch.py >= m_y && touch.py <= (m_y + m_h)) {
    if (m_isSelected) {
      if (m_activatable) m_isActivated = true;
      else m_inputAction(KEY_A, &m_isActivated);
      return;
    }

    for(Button *btn : Button::g_buttons) {
      btn->m_isSelected = false;
      btn->m_isActivated = false;
    }

    m_isSelected = true;
  }
}
