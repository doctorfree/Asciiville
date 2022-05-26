from aewan import AeDoc, AeLayer
import curses
import sys

def setattr(wnd, at):
   at = int(at)
   fg = at >> 4
   bg = at & 0x0f
   bold = 0
   blink = 0
   
   if fg > 7:
      bold = 1
      fg = fg - 8

   if bg > 7:
      blink = 1
      bg = bg - 8

   if fg == 7 and bg == 0: wnd.attrset(curses.A_NORMAL)
   elif fg == 0 and bg == 0: wnd.attrset(curses.color_pair(7))
   else: wnd.attrset(curses.color_pair(bg * 8 + fg))

   if bold: wnd.attron(curses.A_BOLD)
   if blink: wnd.attron(curses.A_BLINK)

stdscr = curses.initscr()
curses.noecho()
curses.start_color()

for bg in range(8):
   for fg in range(8):
      if bg > 0 or fg > 0: curses.init_pair(bg*8 + fg, fg, bg)

curses.init_pair(7, 0, 0)


stdscr.clear()

d = AeDoc("/tmp/ut.aes.gz")

sx = sy = 0
while 1:
   d.layers[0].render(stdscr, 20, 8, 40, 10, sx, sy, setattr)
   stdscr.refresh()
   ch = chr(stdscr.getch())
   if ch == 'h': sx = sx - 1
   elif ch == 'l': sx = sx + 1
   elif ch == 'k': sy = sy - 1
   elif ch == 'j': sy = sy + 1
   elif ch == 'q':
      curses.endwin()
      sys.exit(0)

