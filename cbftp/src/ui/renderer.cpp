#include "renderer.h"

#include <clocale>
#include <sys/ioctl.h>
#include <unistd.h>

#include "misc.h"
#include "termint.h"

Renderer::Renderer() : main(nullptr), dead(false) {
}

bool Renderer::init() {
  thread.start("cbftp-render", this);

  initret = true;
  uiqueue.push(UICommand(UI_COMMAND_INIT));
  eventcomplete.wait();
  if (!initret) {
    return false;
  }
  return true;
}

unsigned int Renderer::getCol() const {
  return col;
}

unsigned int Renderer::getRow() const {
  return row;
}

WINDOW* Renderer::getWindow(Window window) {
  WINDOW* w = nullptr;
  switch (window) {
    case Window::MAIN:
      w = main;
      break;
    case Window::INFO:
      w = info;
      break;
    case Window::LEGEND:
      w = legend;
      break;
  }
  return w;
}

void Renderer::initIntern() {
  setlocale(LC_ALL, "");
  initscr();
  cbreak();
  curs_set(0);
  ::refresh();
  getmaxyx(stdscr, row, col);
  if (row < 24 || col < 80) {
    endwin();
    printf("Error: terminal too small. 80x24 required. (Current %dx%d)\n", col, row);
    initret = false;
    eventcomplete.post();
    return;
  }
#if NCURSES_EXT_FUNCS >= 20081102
  set_escdelay(25);
#else
  ESCDELAY = 25;
#endif
#ifdef NCURSES_VERSION
  define_key("\x1B[1;2A", TERMINT_SHIFT_UP);
  define_key("\x1B[1;2B", TERMINT_SHIFT_DOWN);
  define_key("\x1B[1;2C", TERMINT_SHIFT_RIGHT);
  define_key("\x1B[1;2D", TERMINT_SHIFT_LEFT);
  define_key("\x1B[1;5A", TERMINT_CTRL_UP);
  define_key("\x1B[1;5B", TERMINT_CTRL_DOWN);
  define_key("\x1B[1;5C", TERMINT_CTRL_RIGHT);
  define_key("\x1B[1;5D", TERMINT_CTRL_LEFT);
  define_key("\x1B[1;6C", TERMINT_CTRL_SHIFT_RIGHT);
  define_key("\x1B[1;6D", TERMINT_CTRL_SHIFT_LEFT);
  define_key("\x1BOH", TERMINT_HOME);
  define_key("\x1BOF", TERMINT_END);
  define_key("\x1BOa", TERMINT_CTRL_UP);
  define_key("\x1BOb", TERMINT_CTRL_DOWN);
  define_key("\x1BOc", TERMINT_CTRL_RIGHT);
  define_key("\x1BOd", TERMINT_CTRL_LEFT);
  define_key("\x1B[a", TERMINT_SHIFT_UP);
  define_key("\x1B[b", TERMINT_SHIFT_DOWN);
  define_key("\x1B[c", TERMINT_SHIFT_RIGHT);
  define_key("\x1B[d", TERMINT_SHIFT_LEFT);
  define_key("\x1B[7~", TERMINT_HOME);
  define_key("\x1B[8~", TERMINT_END);
#endif
  main = newwin(row, col, 0, 0);
  legend = newwin(2, col, row - 2, 0);
  info = newwin(2, col, 0, 0);
  keypad(stdscr, TRUE);
  start_color();
  assume_default_colors(-1, -1);
  for (int i = -1; i < 8; i++) {
    for (int j = -1; j < 8; j++) {
      init_pair(encodeColorRepresentation(i, j), i, j);
    }
  }
  noecho();
  eventcomplete.post();
}

void Renderer::refresh(bool infoenabled, bool legendenabled) {
  uiqueue.push(UICommand(UI_COMMAND_REFRESH, infoenabled, legendenabled));
}

void Renderer::resize(bool infoenabled, bool legendenabled) {
  uiqueue.push(UICommand(UI_COMMAND_RESIZE, infoenabled, legendenabled));
  eventcomplete.wait();
}

void Renderer::adjust(bool infoenabled, bool legendenabled) {
  uiqueue.push(UICommand(UI_COMMAND_ADJUST, infoenabled, legendenabled));
}

void Renderer::bell() {
  uiqueue.push(UICommand(UI_COMMAND_BELL));
}

void Renderer::hideCursor() {
  uiqueue.push(UICommand(UI_COMMAND_CURSOR_HIDE));
}

void Renderer::showCursor() {
  uiqueue.push(UICommand(UI_COMMAND_CURSOR_SHOW));
}

void Renderer::moveCursor(unsigned int row, unsigned int col) {
  uiqueue.push(UICommand(UI_COMMAND_CURSOR_MOVE, main, row, col));
}

void Renderer::erase(Window window) {
  uiqueue.push(UICommand(UI_COMMAND_ERASE, getWindow(window)));
}

void Renderer::setDefaultColors(int fgcolor, int bgcolor) {
  uiqueue.push(UICommand(UI_COMMAND_DEFAULT_COLORS, fgcolor, bgcolor));
}

void Renderer::run() {
  while (true) {
    UICommand command = uiqueue.pop();
    switch (command.getCommand()) {
      case UI_COMMAND_INIT:
        initIntern();
        break;
      case UI_COMMAND_REFRESH:
        refreshIntern(command.getInfoEnabled(), command.getLegendEnabled());
        break;
      case UI_COMMAND_HIGHLIGHT_OFF:
        wattroff(command.getWindow(), A_REVERSE);
        break;
      case UI_COMMAND_HIGHLIGHT_ON:
        wattron(command.getWindow(), A_REVERSE);
        break;
      case UI_COMMAND_CURSOR_SHOW:
        curs_set(1);
        break;
      case UI_COMMAND_CURSOR_HIDE:
        curs_set(0);
        break;
      case UI_COMMAND_CURSOR_MOVE:
        TermInt::moveCursor(command.getWindow(), command.getRow(), command.getCol());
        break;
      case UI_COMMAND_ERASE:
        werase(command.getWindow());
        break;
      case UI_COMMAND_PRINT_STR:
        TermInt::printStr(command.getWindow(), command.getRow(), command.getCol(),
            command.getText(), command.getMaxlen(), command.getRightAlign());
        break;
      case UI_COMMAND_PRINT_WIDE_STR:
        TermInt::printStr(command.getWindow(), command.getRow(), command.getCol(),
            command.getWideText(), command.getMaxlen(), command.getRightAlign());
        break;
      case UI_COMMAND_PRINT_CHAR:
        TermInt::printChar(command.getWindow(), command.getRow(), command.getCol(),
            command.getChar());
        break;
      case UI_COMMAND_RESIZE:
        resizeIntern(command.getInfoEnabled(), command.getLegendEnabled());
        break;
      case UI_COMMAND_ADJUST:
        adjustIntern(command.getInfoEnabled(), command.getLegendEnabled());
        break;
      case UI_COMMAND_KILL:
        endwin();
        dead = true;
        return;
      case UI_COMMAND_BELL:
        beep();
        break;
      case UI_COMMAND_COLOR_ON:
        wattron(command.getWindow(), COLOR_PAIR(command.getColor()));
        break;
      case UI_COMMAND_COLOR_OFF:
        wattroff(command.getWindow(), COLOR_PAIR(command.getColor()));
        break;
      case UI_COMMAND_BOLD_ON:
        wattron(command.getWindow(), A_BOLD);
        break;
      case UI_COMMAND_BOLD_OFF:
        wattroff(command.getWindow(), A_BOLD);
        break;
      case UI_COMMAND_DEFAULT_COLORS:
        assume_default_colors(command.getColor(), command.getBgColor());
        redrawwin(legend);
        redrawwin(info);
        redrawwin(main);
        break;
    }
  }
}

void Renderer::kill() {
  uiqueue.push(UICommand(UI_COMMAND_KILL));
  for (int i = 0; i < 10; i++) {
    usleep(100000);
    if (dead) {
      break;
    }
  }
  if (!dead) {
    endwin();
  }
}

unsigned int Renderer::read() {
  wint_t wch;
  get_wch(&wch);
  return wch;
}

void Renderer::resizeIntern(bool infoenabled, bool legendenabled) {
  struct winsize size;
  bool trytodraw = false;
  if (ioctl(fileno(stdout), TIOCGWINSZ, &size) == 0) {
    if (size.ws_row >= 5 && size.ws_col >= 10) {
      trytodraw = true;
    }
  }
  if (trytodraw) {
    resizeterm(size.ws_row, size.ws_col);
    endwin();
    timeout(0);
    while (getch() != ERR);
    timeout(-1);
    ::refresh();
    getmaxyx(stdscr, row, col);
    wresize(legend, 2, col);
    wresize(info, 2, col);
    adjustIntern(infoenabled, legendenabled);
  }
  eventcomplete.post();
}

void Renderer::adjustIntern(bool infoenabled, bool legendenabled) {
  unsigned int mainrow = row;
  if (infoenabled) {
    mainrow -= 2;
    mvwin(main, 2, 0);
  }
  else {
    mvwin(main, 0, 0);
  }
  if (legendenabled) {
    mainrow -= 2;
    mvwin(legend, row - 2, 0);
  }
  wresize(main, mainrow, col);
  refreshIntern(infoenabled, legendenabled);
}

void Renderer::refreshIntern(bool refreshinfo, bool refreshlegend) {
  if (refreshlegend) {
    wnoutrefresh(legend);
  }
  if (refreshinfo) {
    wnoutrefresh(info);
  }
  wnoutrefresh(main);
  doupdate();
}

void Renderer::highlight(bool highlight, Window window) {
  if (highlight) {
    uiqueue.push(UICommand(UI_COMMAND_HIGHLIGHT_ON, getWindow(window)));
  }
  else {
    uiqueue.push(UICommand(UI_COMMAND_HIGHLIGHT_OFF, getWindow(window)));
  }
}

void Renderer::bold(bool bold, Window window) {
  if (bold) {
    uiqueue.push(UICommand(UI_COMMAND_BOLD_ON, getWindow(window)));
  }
  else {
    uiqueue.push(UICommand(UI_COMMAND_BOLD_OFF, getWindow(window)));
  }
}

void Renderer::color(bool enabled, int color, Window window) {
  if (enabled) {
    uiqueue.push(UICommand(UI_COMMAND_COLOR_ON, getWindow(window), color));
  }
  else {
    uiqueue.push(UICommand(UI_COMMAND_COLOR_OFF, getWindow(window), color));
  }
}

void Renderer::printStr(unsigned int row, unsigned int col, const std::string & str, bool highlight, bool bold, int color, int maxlen, bool rightalign, Window window) {
  if (highlight) {
    this->highlight(true, window);
  }
  if (bold) {
    this->bold(true, window);
  }
  if (color != -1 && color != encodeColorRepresentation()) {
    this->color(true, color, window);
  }
  uiqueue.push(UICommand(UI_COMMAND_PRINT_STR, getWindow(window), row, col, str, maxlen, rightalign));
  if (highlight) {
    this->highlight(false, window);
  }
  if (bold) {
    this->bold(false, window);
  }
  if (color != -1 && color != encodeColorRepresentation()) {
    this->color(false, color, window);
  }
}

void Renderer::printStr(unsigned int row, unsigned int col, const std::basic_string<unsigned int> & str, bool highlight, bool bold, int color, int maxlen, bool rightalign, Window window) {
  if (highlight) {
    this->highlight(true, window);
  }
  if (bold) {
    this->bold(true, window);
  }
  if (color != -1 && color != encodeColorRepresentation()) {
    this->color(true, color, window);
  }
  uiqueue.push(UICommand(UI_COMMAND_PRINT_WIDE_STR, getWindow(window), row, col, str, maxlen, rightalign));
  if (highlight) {
    this->highlight(false, window);
  }
  if (bold) {
    this->bold(false, window);
  }
  if (color != -1 && color != encodeColorRepresentation()) {
    this->color(false, color, window);
  }
}

void Renderer::printChar(unsigned int row, unsigned int col, unsigned int c, bool highlight, bool bold, int color, Window window) {
  if (highlight) {
    this->highlight(true, window);
  }
  if (bold) {
    this->bold(true, window);
  }
  if (color != -1 && color != encodeColorRepresentation()) {
    this->color(true, color, window);
  }
  uiqueue.push(UICommand(UI_COMMAND_PRINT_CHAR, getWindow(window), row, col, c));
  if (highlight) {
    this->highlight(false, window);
  }
  if (bold) {
    this->bold(false, window);
  }
  if (color != -1 && color != encodeColorRepresentation()) {
    this->color(false, color, window);
  }
}
