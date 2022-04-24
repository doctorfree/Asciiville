#include "textinputfield.h"

TextInputField::TextInputField() {

}

TextInputField::TextInputField(int visiblelen, int maxlen) {
  construct("", visiblelen, maxlen, false);
}

TextInputField::TextInputField(int visiblelen, int maxlen, bool secret) {
  construct("", visiblelen, maxlen, secret);
}

TextInputField::TextInputField(std::string starttext, int visiblelen, int maxlen, bool secret) {
  construct(starttext, visiblelen, maxlen, secret);
}

void TextInputField::construct(std::string starttext, int visiblelen, int maxlen, bool secret) {
  this->text = starttext;
  this->visiblelen = visiblelen;
  this->maxlen = maxlen;
  this->secret = secret;
  this->cursor = starttext.length();
}

std::string TextInputField::getText() const {
  return text;
}

std::string TextInputField::getVisualText() const {
  std::string visualtext = "";
  unsigned int writelen = text.length();
  unsigned int start = 0;
  if (writelen > visiblelen) {
    if (cursor >= writelen - visiblelen / 2) {
      start = writelen - visiblelen;
    }
    else if (cursor >= visiblelen / 2) {
      start = cursor - visiblelen / 2;
    }
    else {
      start = 0;
    }
    writelen = visiblelen;
  }
  for (unsigned int i = 0; i < writelen; i++) {
    visualtext += secret ? '*' : text[start+i];
  }
  for (unsigned int i = writelen; i < visiblelen; i++) {
    visualtext += ' ';
  }
  return visualtext;
}

unsigned int TextInputField::getVisualCursorPosition() const {
  unsigned int writelen = text.length();
  if (writelen > visiblelen) {
    if (cursor >= writelen - visiblelen / 2) {
      return cursor + visiblelen - writelen;
    }
    else if (cursor >= visiblelen / 2) {
      return visiblelen / 2;
    }
    else {
      return cursor;
    }
  }
  return cursor;
}

bool TextInputField::addchar(char c) {
  if (text.length() < maxlen) {
    if (cursor == 0) {
      text = c + text;
    }
    else if (cursor < text.length()) {
      text = text.substr(0, cursor) + c + text.substr(cursor);
    }
    else {
      text+= c;
    }
    cursor++;
    return true;
  }
  return false;
}

bool TextInputField::moveCursorLeft() {
  if (cursor > 0) {
    cursor--;
    return true;
  }
  return false;
}

bool TextInputField::moveCursorRight() {
  if (cursor < text.length()) {
    cursor++;
    return true;
  }
  return false;
}

void TextInputField::moveCursorHome() {
  cursor = 0;
}

void TextInputField::moveCursorEnd() {
  cursor = text.length();
}

void TextInputField::erase() {
  if (cursor == 0) {
    return;
  }
  int len = text.length();
  if (len > 0) {
    if (cursor == 1) {
      text = text.substr(1);
    }
    else {
      text = text.substr(0, cursor - 1) + text.substr(cursor);
    }
    cursor--;
  }
}

void TextInputField::eraseForward() {
  if (moveCursorRight()) {
    erase();
  }
}

void TextInputField::eraseCursoredWord() {
  if (text.empty() || !cursor) {
    return;
  }
  unsigned int erasestart = cursor - 1;
  bool foundword = false;
  for (;; --erasestart) {
    bool erase = false;
    if (text[erasestart] == ' ' && foundword) {
      ++erasestart;
      erase = true;
    }
    if (erase || erasestart == 0) {
      std::string newtext = text.substr(0, erasestart) + text.substr(cursor);
      cursor -= (text.length() - newtext.length());
      text = newtext;
      return;
    }
    if (text[erasestart] != ' ') {
      foundword = true;
    }
  }
}

void TextInputField::eraseAllBefore() {
  while (cursor > 0) {
    erase();
  }
}

void TextInputField::moveCursorPreviousWord() {
  bool alphanumpassed = false;
  while (cursor > 0) {
    if (isalnum(text[cursor - 1])) {
      alphanumpassed = true;
    }
    else if (alphanumpassed) {
      return;
    }
    cursor--;
  }
}

void TextInputField::moveCursorNextWord() {
  bool alphanumpassed = false;
  while (cursor < text.length()) {
    if (isalnum(text[cursor])) {
      alphanumpassed = true;
    }
    else if (alphanumpassed) {
      return;
    }
    cursor++;
  }
}

void TextInputField::setText(std::string text) {
  this->text = text;
  moveCursorEnd();
}

void TextInputField::clear() {
  text = "";
  cursor = 0;
}

void TextInputField::setVisibleLength(unsigned int visiblelen) {
  this->visiblelen = visiblelen;
}
