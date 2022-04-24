#include "infowindow.h"

#include "ui.h"
#include "termint.h"

InfoWindow::InfoWindow(Ui* ui, unsigned int row, unsigned int col) : UIWindow(ui, "InfoWindow") {
  split = false;
  init(row, col);
}

void InfoWindow::redraw() {
  ui->getRenderer().erase(Window::INFO);
  ui->getRenderer().printChar(0, 1, BOX_CORNER_TR, false, false, -1, Window::INFO);
  ui->getRenderer().printChar(0, 0, BOX_HLINE, false, false, -1, Window::INFO);
  ui->getRenderer().printChar(1, 1, BOX_CORNER_BL, false, false, -1, Window::INFO);
  ui->getRenderer().printChar(0, col - 1, BOX_HLINE, false, false, -1, Window::INFO);
  ui->getRenderer().printChar(1, col - 2, BOX_CORNER_BR, false, false, -1, Window::INFO);
  ui->getRenderer().printChar(0, col - 2, BOX_CORNER_TL, false, false, -1, Window::INFO);
  for (unsigned int i = 2; i < col - 2; i++) {
    ui->getRenderer().printChar(1, i, BOX_HLINE, false, false, -1, Window::INFO);
  }
  if (split) {
    ui->getRenderer().printChar(1, col / 2, BOX_HLINE_BOT, false, false, -1, Window::INFO);
  }
  update();
}

void InfoWindow::update() {
  for (unsigned int i = 2; i < col - 2; i++) {
    ui->getRenderer().printChar(0, i, ' ', false, false, -1, Window::INFO);
  }
  unsigned int labellen = label.length();
  ui->getRenderer().printStr(0, 4, label, false, false, -1, labellen, false, Window::INFO);
  ui->getRenderer().printStr(0, 4 + labellen + 2, text, false, false, -1, col - 4 - 4 - labellen - 2, true, Window::INFO);
}

void InfoWindow::setLabel(const std::basic_string<unsigned int>& label) {
  this->label = label;
  update();
}
void InfoWindow::setText(const std::basic_string<unsigned int>& text) {
  this->text = text;
  update();

}

void InfoWindow::setSplit(bool split) {
  this->split = split;
}
