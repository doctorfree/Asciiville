#include "uiwindow.h"

#include "ui.h"

void UIWindow::init(unsigned int row, unsigned int col) {
  resize(row, col);
  vv->reset();
  redraw();
}

UIWindow::UIWindow(Ui* ui, const std::string& name) : name(name), row(0), col(0),
    autoupdate(false), expectbackendpush(false), ui(ui), keybinds(name), allowimplicitgokeybinds(true)
{
  vv = &ui->getVirtualView();
  ui->addKeyBinds(&keybinds);
}

UIWindow::~UIWindow() {
  ui->removeKeyBinds(&keybinds);
}

void UIWindow::resize(unsigned int row, unsigned int col) {
  this->row = row;
  this->col = col;
}

void UIWindow::update() {
  redraw();
}

void UIWindow::command(const std::string& command) {
  this->command(command, "");
}

void UIWindow::command(const std::string& command, const std::string& arg) {

}

bool UIWindow::keyPressedBase(unsigned int key) {
  bool caught = keyPressed(key);
  if (caught) {
    return true;
  }
  int action = keybinds.getKeyAction(key);
  if (action == KEYACTION_KEYBINDS && allowimplicitgokeybinds) {
    ui->goKeyBinds(&keybinds);
    return true;
  }
  if (action == KEYACTION_UP || (key == KEY_UP && !keybinds.hasBind(KEYACTION_UP, KEYSCOPE_ALL))) {
    bool moved = keyUp();
    if (!moved) {
      moved = vv->goUp();
    }
    if (moved) {
      ui->redraw();
    }
    return moved;
  }
  if (action == KEYACTION_DOWN || (key == KEY_DOWN && !keybinds.hasBind(KEYACTION_DOWN, KEYSCOPE_ALL))) {
    bool moved = keyDown();
    if (!moved) {
      moved = vv->goDown();
    }
    if (moved) {
      ui->redraw();
    }
    return moved;
  }
  if (action == KEYACTION_LEFT || (key == KEY_LEFT && !keybinds.hasBind(KEYACTION_LEFT, KEYSCOPE_ALL))) {
    bool moved = vv->goLeft();
    if (moved) {
      ui->redraw();
    }
    return moved;
  }
  if (action == KEYACTION_RIGHT || (key == KEY_RIGHT && !keybinds.hasBind(KEYACTION_RIGHT, KEYSCOPE_ALL))) {
    bool moved = vv->goRight();
    if (moved) {
      ui->redraw();
    }
    return moved;
  }
  if (action == KEYACTION_NEXT_PAGE || (key == KEY_NPAGE && !keybinds.hasBind(KEYACTION_NEXT_PAGE, KEYSCOPE_ALL))) {
    if (keyDown()) {
      for (unsigned int i = 1; i < row / 2; i++) {
        keyDown();
      }
    }
    else if (keyPressed(KEY_DOWN))
    {
      for (unsigned int i = 1; i < row / 2; i++) {
        keyPressed(KEY_DOWN);
      }
    }
    else {
      vv->pageDown();
    }
    ui->redraw();
    return true;
  }
  if (action == KEYACTION_PREVIOUS_PAGE || (key == KEY_PPAGE && !keybinds.hasBind(KEYACTION_PREVIOUS_PAGE, KEYSCOPE_ALL))) {
    if (keyUp()) {
      for (unsigned int i = 1; i < row / 2; i++) {
        keyUp();
      }
    }
    else if (keyPressed(KEY_UP))
    {
      for (unsigned int i = 1; i < row / 2; i++) {
        keyPressed(KEY_UP);
      }
    }
    else {
      vv->pageUp();
    }
    ui->redraw();
    return true;
  }

  return false;
}

bool UIWindow::keyUp() {
  return false;
}

bool UIWindow::keyDown() {
  return false;
}

bool UIWindow::keyPressed(unsigned int key) {
  return false;
}

std::string UIWindow::getInfoLabel() const {
  return "";
}

std::string UIWindow::getInfoText() const {
  return "";
}

std::string UIWindow::getLegendText() const {
  return keybinds.getLegendSummary();
}

std::basic_string<unsigned int> UIWindow::getWideInfoLabel() const {
  std::string label = getInfoLabel();
  return std::basic_string<unsigned int>(label.begin(), label.end());
}

std::basic_string<unsigned int> UIWindow::getWideInfoText() const {
  std::string text = getInfoText();
  return std::basic_string<unsigned int>(text.begin(), text.end());
}

std::basic_string<unsigned int> UIWindow::getWideLegendText() const {
  std::string text = getLegendText();
  return std::basic_string<unsigned int>(text.begin(), text.end());
}

bool UIWindow::autoUpdate() const {
  return autoupdate;
}

bool UIWindow::expectBackendPush() const {
  return expectbackendpush;
}

bool UIWindow::isTop() const {
  return ui->isTop(this);
}
