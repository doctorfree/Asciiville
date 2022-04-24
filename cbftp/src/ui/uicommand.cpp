#include "uicommand.h"

UICommand::UICommand(int command) :
  command(command)
{
}

UICommand::UICommand(int command, int fgcolor, int bgcolor) :
  command(command),
  color(fgcolor),
  bgcolor(bgcolor)
{
}

UICommand::UICommand(int command, bool show) :
  command(command),
  show(show)
{
}

UICommand::UICommand(int command, bool infoenabled, bool legendenabled) :
  command(command),
  infoenabled(infoenabled),
  legendenabled(legendenabled)
{
}

UICommand::UICommand(int command, WINDOW * window, int color) :
  command(command),
  window(window),
  color(color)
{
}

UICommand::UICommand(int command, WINDOW * window, unsigned int row, unsigned int col) :
  command(command),
  window(window),
  row(row),
  col(col)
{
}

UICommand::UICommand(int command, WINDOW * window) :
  command(command),
  window(window)
{
}

UICommand::UICommand(int command, WINDOW * window, unsigned int row, unsigned int col, std::string text, int maxlen, bool rightalign) :
  command(command),
  window(window),
  row(row),
  col(col),
  text(text),
  maxlen(maxlen),
  rightalign(rightalign)
{
}

UICommand::UICommand(int command, WINDOW * window, unsigned int row, unsigned int col, std::basic_string<unsigned int> text, int maxlen, bool rightalign) :
  command(command),
  window(window),
  row(row),
  col(col),
  wtext(text),
  maxlen(maxlen),
  rightalign(rightalign)
{
}

UICommand::UICommand(int command, WINDOW * window, unsigned int row, unsigned int col, unsigned int c) :
  command(command),
  window(window),
  row(row),
  col(col),
  c(c)
{
}

int UICommand::getCommand() const {
  return command;
}

WINDOW * UICommand::getWindow() const {
  return window;
}

unsigned int UICommand::getRow() const {
  return row;
}

unsigned int UICommand::getCol() const {
  return col;
}

std::string UICommand::getText() const {
  return text;
}

std::basic_string<unsigned int> UICommand::getWideText() const {
  return wtext;
}

int UICommand::getMaxlen() const {
  return maxlen;
}

bool UICommand::getRightAlign() const {
  return rightalign;
}

unsigned int UICommand::getChar() const {
  return c;
}

int UICommand::getColor() const {
  return color;
}

int UICommand::getBgColor() const {
  return bgcolor;
}

bool UICommand::getShow() const {
  return show;
}

bool UICommand::getInfoEnabled() const {
  return infoenabled;
}

bool UICommand::getLegendEnabled() const {
  return legendenabled;
}
