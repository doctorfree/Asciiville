#include "menuselectoptiontextfield.h"

#include "ncurseswrap.h"

MenuSelectOptionTextField::MenuSelectOptionTextField() {
  textfield = TextInputField("", 0, 0, false);
  init("none", 0, 0, "none");
}
MenuSelectOptionTextField::MenuSelectOptionTextField(const std::string& identifier,
    int row, int col, const std::string& label, const std::string& starttext,
    int visiblelen, int maxlen, bool secret)
{
  textfield = TextInputField(starttext, visiblelen, maxlen, secret);
  init(identifier, row, col, label);
}

std::string MenuSelectOptionTextField::getContentText() const {
  return textfield.getVisualText();
}

bool MenuSelectOptionTextField::activate() {
  active = true;
  textfield.moveCursorEnd();
  return true;
}

bool MenuSelectOptionTextField::inputChar(int ch) {
  if (ch >= 32 && ch <= 126) {
    textfield.addchar(ch);
  }
  else if (ch == KEY_BACKSPACE || ch == 8 || ch == 127) {
    textfield.erase();
  }
  else if (ch == KEY_LEFT) {
    textfield.moveCursorLeft();
  }
  else if (ch == KEY_RIGHT) {
    textfield.moveCursorRight();
  }
  else if (ch == KEY_HOME) {
    textfield.moveCursorHome();
  }
  else if (ch == KEY_END) {
    textfield.moveCursorEnd();
  }
  else if (ch == KEY_DC) {
    textfield.eraseForward();
  }
  else if (ch == 23) { // ctrl+w
    textfield.eraseCursoredWord();
  }
  else if (ch == 544) { // ctrl+left
    textfield.moveCursorPreviousWord();
  }
  else if (ch == 559) { // ctrl+right
    textfield.moveCursorNextWord();
  }
  else if (ch == 21) { // ctrl+u
    textfield.eraseAllBefore();
  }
  else {
    return false;
  }
  return true;
}

int MenuSelectOptionTextField::cursorPosition() const {
  return textfield.getVisualCursorPosition();
}

std::string MenuSelectOptionTextField::getData() const {
  return textfield.getText();
}

void MenuSelectOptionTextField::clear() {
  textfield.clear();
}

void MenuSelectOptionTextField::setText(const std::string& text) {
  textfield.setText(text);
}

std::string MenuSelectOptionTextField::getLegendText() const {
  return "[Enter] Finish editing - [Any] Input to text" + extralegend;
}

void MenuSelectOptionTextField::setExtraLegendText(const std::string& extra) {
  extralegend = " - " + extra;
}

unsigned int MenuSelectOptionTextField::wantedWidth() const {
  return textfield.getVisualText().length();
}

void MenuSelectOptionTextField::setMaxWidth(unsigned int maxwidth) {
  this->maxwidth = maxwidth;
  textfield.setVisibleLength(maxwidth);
}
