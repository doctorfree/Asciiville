# Copyright (c) 2004 Bruno T. C. de Oliveira
# 
# LICENSE INFORMATION:
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
# 
# You should have received a copy of the GNU General Public
# License along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
# Copyright (c) 2002 Bruno T. C. de Oliveira
# 
# INFORMAÇÕES DE LICENÇA:
# Este programa é um software de livre distribuição; você pode
# redistribuí-lo e/ou modificá-lo sob os termos da GNU General
# Public License, conforme publicado pela Free Software Foundation,
# pela versão 2 da licença ou qualquer versão posterior.
# 
# Este programa é distribuído na esperança de que ele será útil
# aos seus usuários, porém, SEM QUAISQUER GARANTIAS; sem sequer
# a garantia implícita de COMERCIABILIDADE ou DE ADEQUAÇÃO A
# QUALQUER FINALIDADE ESPECÍFICA. Consulte a GNU General Public
# License para obter mais detalhes (uma cópia acompanha este
# programa, armazenada no arquivo COPYING).
 
import sys, os, string
import gzip
import curses

class AeError(Exception):
   def __init__(self, arg):
      Exception.__init__(self, arg)

class AeDoc:
   """
   Represents an aewan document loaded from a file.
   Attributes:

   layers[] - list of layers (AeLayer objects)
   """
   def __init__(self, file):
      """
      Creates a new aewan document object by loading it from the
      specified file. The file must be in AES format, as generated
      by the ae2aes conversion utility. The file may be gzip-compressed.
      
      If there is a format-related error while loading the document, 
      raises an AeError. If there is an I/O related error, raises an IOError.
      """
      self.layers = []

      f = gzip.open(file, 'r')
      _read_marker(f, "AES v1.0")
      _read_marker(f, "begin document")

      layercount = _read_field_int(f, "layer-count")
      for i in range(layercount):
         self.layers.append(AeLayer(f))
      
      _read_marker(f, "end document")


class AeLayer:
   """
   Represents each of the layers in an aewan document. Attributes:

   name
   width
   height
   visible - 1 if layer is marked as visible, 0 otherwise
   transparent - 1 if layer is marked as transparent, 0 otherwise

   char[height][width] - the character at each position in the layer
   attr[height][width] - the attribute of each position in the layer

   Please note that the char and attr matrices are addressed
   with indexes [y][x], NOT the more intuitive [x][y].
   """
   def __init__(self, f):
      """
      Initializes a new layer by loading its data from file object f.
      Raises an AeError if a format-related error occurs, or an IOError
      if an i/o-related error occurs.
      """
      _read_marker(f, "begin layer")
      self.name        = _read_field(f, "name")
      self.width       = _read_field_int(f, "width")
      self.height      = _read_field_int(f, "height")
      self.visible     = _read_field_int(f, "visible")
      self.transparent = _read_field_int(f, "transparent")
      _read_marker(f, "begin canvas")

      self.char = []
      self.attr = []
      for y in range(self.height):
         a = string.split(string.strip(f.readline()))
         if len(a) != self.width * 2:
            raise AeError("Canvas line with insufficient cells:\n" + a)

         char_row = []
         attr_row = []
         for x in range(self.width):
            try:
               char_row.append(int(a[x * 2], 16))
               attr_row.append(int(a[x * 2 + 1], 16))
            except ValueError:
               raise AeError("Error: map cell x=%d, y=%d has invalid data\n"
                             "(not a hexadecimal number)" % (x,y))

         self.char.append(char_row)
         self.attr.append(attr_row)
      
      _read_marker(f, "end canvas")
      _read_marker(f, "end layer")

   def render(self, wnd, dest_x, dest_y, dest_w, dest_h, src_x, src_y, \
              setattr_cb):
      """
      Render the contents of this layer using curses. The image will
      be rendered on the curses window identified by wnd, and will
      be confined to the rectangle defined by the dest_* arguments.
      The image will be translated in such a way that the left-top
      corner of the destination rectangle will show the layer cell
      whose coordinates are src_x, src_y.

      In order to set curses attributes for each cell, this function
      will call setattr_cb passing it the window and the attribute as 
      parameters.
      """
      for y in range(dest_h):
         wnd.move(y + dest_y, dest_x)
         for x in range(dest_w):
            sx = x + src_x
            sy = y + src_y
            if 0 <= sx < self.width and 0 <= sy < self.height:
               ch = self.char[sy][sx]
               if 0 <= ch < 32: ch = 32 # don't display control chars
               # FIXME: this doesn't display special line-graphics
               # characters. Ideally, we should convert special
               # characters into the appropriate ACS_* characters
               # at this point.

               at = self.attr[sy][sx]
            else:
               ch = 0x20
               at = 0x70

            setattr_cb(wnd, at)
            wnd.addch(chr(ch))


# ------------------------------------------------------------------------
def _read_marker(f, str):
   line = string.strip(f.readline())
   if line == str: return
   raise AeError("Incorrectly formatted AES document.\n"
                 "Expected marker: '%s'\nBut found: '%s'\n" % (str,line))

def _read_field(f, fieldname):
   line = string.strip(f.readline())
   a = string.split(line, maxsplit=1)

   if len(a) != 2:
      raise AeError("Incorrect field syntax in AES document:\n%s\n" % line) 

   if a[0] != fieldname:
      raise AeError("AES field name mismatch.\nExpected: '%s'\nFound: '%s'" \
                  % (fieldname, a[0]))
   
   return a[1]
   
def _read_field_int(f, fieldname):
   x = _read_field(f, fieldname);
   try:
      return int(x)
   except ValueError:
      raise AeError("AES field type mismatch: expected an integer\n"
                    "Field name: '%s'\nValue: '%s'" % (fieldname, x))

