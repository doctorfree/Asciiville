#pragma once

#include <string>

class TextInputField {
public:
  TextInputField();
  TextInputField(int, int);
  TextInputField(int, int, bool);
  TextInputField(std::string, int, int, bool);
  std::string getText() const;
  std::string getVisualText() const;
  unsigned int getVisualCursorPosition() const;
  bool addchar(char);
  void erase();
  void eraseForward();
  void eraseCursoredWord();
  void eraseAllBefore();
  bool moveCursorLeft();
  bool moveCursorRight();
  void moveCursorHome();
  void moveCursorEnd();
  void moveCursorPreviousWord();
  void moveCursorNextWord();
  void setText(std::string);
  void clear();
  void setVisibleLength(unsigned int);
private:
  void construct(std::string, int, int, bool);
  std::string text;
  unsigned int cursor;
  bool secret;
  unsigned int maxlen;
  unsigned int visiblelen;
};
