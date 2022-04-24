#include "braillegraph.h"

#include <algorithm>
#include <cassert>
#include <set>

#include "termint.h"

namespace {

const unsigned int BRAILLE_UNICODE_BASE = 0x2800;

}

struct BrailleGraph::DataPoint {
  DataPoint(unsigned int data, unsigned int id) : data(data), id(id) {

  }
  unsigned int data;
  unsigned int id;
};

BrailleGraph::BrailleGraph(unsigned int row, unsigned int col,
  const std::string& title, const std::string& unit, unsigned int floor,
  unsigned int ceiling, bool dynamicceiling) : row(row), col(col), min(0), max(0), avg(0),
  graphfloor(floor), graphceiling(ceiling), graphceilingdynamic(ceiling),
  dynamicceiling(dynamicceiling), title(title), unit(unit), even(false),
  curroffset(0), idcounter(0)
{

}

BrailleGraph::~BrailleGraph() {

}

void BrailleGraph::resize(unsigned int row, unsigned int col, bool redraw) {
  redraw = redraw || this->row != row || this->col != col;
  this->row = row;
  this->col = col;
  if (redraw) {
    render();
  }
}

unsigned int BrailleGraph::rows() const {
  return row;
}

void BrailleGraph::setData(const std::list<unsigned int>& data) {
  this->data.clear();
  graphceilingdynamic = graphceiling;
  for (unsigned int ndata : data) {
    if (ndata > graphceilingdynamic) {
      if (!dynamicceiling) {
        ndata = graphceiling;
      }
      else {
        graphceilingdynamic = ndata;
      }
    }
    if (ndata < graphfloor) {
      ndata = graphfloor;
    }
    this->data.emplace_back(ndata, idcounter++);
  }
  render();
}

void BrailleGraph::addNewData(const std::list<unsigned int>& newdata) {
  if (newdata.empty()) {
    return;
  }
  renderNew(newdata);
}

std::list<BrailleGraph::GraphChar> BrailleGraph::getAllChars() const {
  std::list<BrailleGraph::GraphChar> out = (even ? graphcharseven : graphcharsodd);
  out.insert(out.end(), staticborderchars.begin(), staticborderchars.end());
  out.insert(out.end(), changingborderchars.begin(), changingborderchars.end());
  return out;
}

std::list<BrailleGraph::GraphChar> BrailleGraph::getGraphChars() const {
  return even ? graphcharseven : graphcharsodd;
}

std::list<BrailleGraph::GraphChar> BrailleGraph::getChangingBorderChars() const {
  return changingborderchars;
}

std::list<BrailleGraph::GraphChar> BrailleGraph::getStaticBorderChars() const {
  return staticborderchars;
}

void BrailleGraph::calcGraphChar(const std::map<unsigned int, std::set<unsigned int>>& biggraph, unsigned int j, unsigned int bgcol, unsigned int bigcols, unsigned int& evenval, unsigned int& oddval) {
  unsigned int bgrow = j * 4;
  oddval = 0;
  evenval = 0;
  if (bgcol == 0) {
    unsigned int firstoddval = 0;
    if (unfinishedcol.count(bgrow)) {
      firstoddval += 0x1;
    }
    if (unfinishedcol.count(bgrow + 1)) {
      firstoddval += 0x2;
    }
    if (unfinishedcol.count(bgrow + 2)) {
      firstoddval += 0x4;
    }
    if (biggraph.count(bgrow) && biggraph.at(bgrow).count(bgcol)) {
      firstoddval += 0x8;
    }
    if (biggraph.count(bgrow + 1) && biggraph.at(bgrow + 1).count(bgcol)) {
      firstoddval += 0x10;
    }
    if (biggraph.count(bgrow + 2) && biggraph.at(bgrow + 2).count(bgcol)) {
      firstoddval += 0x20;
    }
    if (unfinishedcol.count(bgrow + 3)) {
      firstoddval += 0x40;
    }
    if (biggraph.count(bgrow + 3) && biggraph.at(bgrow + 3).count(bgcol)) {
      firstoddval += 0x80;
    }
    if (firstoddval) {
      oddval = firstoddval;
    }
  }
  if (bgcol >= bigcols - 1) {
    return;
  }
  unsigned int val = 0;
  if (biggraph.count(bgrow) && biggraph.at(bgrow).count(bgcol)) {
    val += 0x1;
  }
  if (biggraph.count(bgrow + 1) && biggraph.at(bgrow + 1).count(bgcol)) {
    val += 0x2;
  }
  if (biggraph.count(bgrow + 2) && biggraph.at(bgrow + 2).count(bgcol)) {
    val += 0x4;
  }
  if (biggraph.count(bgrow) && biggraph.at(bgrow).count(bgcol + 1)) {
    val += 0x8;
  }
  if (biggraph.count(bgrow + 1) && biggraph.at(bgrow + 1).count(bgcol + 1)) {
    val += 0x10;
  }
  if (biggraph.count(bgrow + 2) && biggraph.at(bgrow + 2).count(bgcol + 1)) {
    val += 0x20;
  }
  if (biggraph.count(bgrow + 3) && biggraph.at(bgrow + 3).count(bgcol)) {
    val += 0x40;
  }
  if (biggraph.count(bgrow + 3) && biggraph.at(bgrow + 3).count(bgcol + 1)) {
    val += 0x80;
  }
  if (bgcol % 2) {
    oddval = val;
  }
  else {
    evenval = val;
  }
}

void BrailleGraph::renderNew(const std::list<unsigned int>& newdata) {
  unsigned int datapos = 0;
  float advance = curroffset;
  unrendereddata.insert(unrendereddata.end(), newdata.begin(), newdata.end());
  std::list<unsigned int> renderdata;
  unsigned int newbiggraphcols = 0;
  for (std::list<unsigned int>::iterator it = unrendereddata.begin(); it != unrendereddata.end();) {
    advance += colfactor;
    ++it;
    int currcol = advance;
    if (currcol < static_cast<int>(advance + colfactor)) { // the next data point would end up in a later graph col
      renderdata.insert(renderdata.end(), unrendereddata.begin(), it);
      unrendereddata.erase(unrendereddata.begin(), it);
      newbiggraphcols += currcol + 1;
      advance -= currcol + 1;
    }
  }
  std::map<unsigned int, std::set<unsigned int>> biggraph;
  int lastrow = biggraphrows - ((rowfactor * data.back().data) + 0.5);
  int lastcol = -1;
  for (unsigned int& datapoint : renderdata) {
    if (datapoint > graphceilingdynamic) {
      if (!dynamicceiling) {
        datapoint = graphceilingdynamic;
      }
      else {
        graphceilingdynamic = datapoint;
      }
    }
    if (datapoint < graphfloor) {
      datapoint = graphfloor;
    }
    addToBigGraph(biggraph, datapoint, datapos, lastrow, lastcol);
    ++datapos;
  }
  if (renderdata.empty()) {
    return;
  }
  curroffset = advance;
  unsigned int newgraphcolstartfull = graphcolstart + graphcols - newbiggraphcols / 2;
  unsigned int newgraphcolstarthalf = graphcolstart + graphcols - (newbiggraphcols + 1) / 2;
  std::list<GraphChar> newevengraphchars;
  std::list<GraphChar> newoddgraphchars;
  for (unsigned int i = 0; i < newbiggraphcols; ++i) {
    for (unsigned int j = 0; j < graphrows; ++j) {
      unsigned int evenval;
      unsigned int oddval;
      calcGraphChar(biggraph, j, i, newbiggraphcols, evenval, oddval);
      unsigned int idx = (i + 1) / 2;
      if (even) {
        if (evenval) {
          newevengraphchars.emplace_back(j + graphrowstart, idx + newgraphcolstartfull, evenval + BRAILLE_UNICODE_BASE);
        }
        if (oddval) {
          newoddgraphchars.emplace_back(j + graphrowstart, idx + newgraphcolstarthalf, oddval + BRAILLE_UNICODE_BASE);
        }
      }
      else {
        if (evenval) {
          newoddgraphchars.emplace_back(j + graphrowstart, idx + newgraphcolstartfull, evenval + BRAILLE_UNICODE_BASE);
        }
        if (oddval) {
          newevengraphchars.emplace_back(j + graphrowstart, idx + newgraphcolstarthalf, oddval + BRAILLE_UNICODE_BASE);
        }
      }
    }
  }
  unsigned int maxcolidx = graphcols - 1 + graphcolstart;
  if (!newevengraphchars.empty()) {
    unsigned int diff = maxcolidx - (even ? newgraphcolstartfull : newgraphcolstarthalf) + 1;
    while (graphcharseven.front().col < diff + graphcolstart) {
      graphcharseven.pop_front();
    }
    for (GraphChar& ch : graphcharseven) {
      assert (ch.col < 1000);
      ch.col -= diff;
      assert (ch.col < 1000);
    }
  }
  graphcharseven.insert(graphcharseven.end(), newevengraphchars.begin(), newevengraphchars.end());
  if (!newoddgraphchars.empty()) {
    unsigned int diff = maxcolidx - (even ? newgraphcolstarthalf : newgraphcolstartfull) + 1;
    while (graphcharsodd.front().col < diff + graphcolstart) {
      graphcharsodd.pop_front();
    }
    for (GraphChar& ch : graphcharsodd) {
      assert (ch.col < 1000);
      ch.col -= diff;
      assert (ch.col < 1000);
    }
  }
  graphcharsodd.insert(graphcharsodd.end(), newoddgraphchars.begin(), newoddgraphchars.end());
  if (newbiggraphcols % 2) {
    even ^= true;
  }
  unfinishedcol.clear();
  for (unsigned int j = 0; j < biggraphrows; j++) {
    if (biggraph.count(j) && biggraph.at(j).count(newbiggraphcols - 1)) {
      unfinishedcol.insert(j);
    }
  }
  unsigned int oldmin = min;
  unsigned int oldmax = max;
  unsigned int oldavg = avg;
  unsigned int oldnow = data.back().data;
  bool newminmax = false;
  for (unsigned int ndata : renderdata) {
    unsigned int id = idcounter++;
    data.emplace_back(ndata, id);
    avgsum += ndata;
    if (ndata <= min) {
      min = ndata;
      idmin = id;
    }
    if (ndata >= max) {
      max = ndata;
      idmax = id;
    }
    const DataPoint& front = data.front();
    avgsum -= front.data;
    if (front.id == idmin || front.id == idmax) {
      newminmax = true;
    }
    data.pop_front();
  }
  if (newminmax) {
    calcMinMax();
  }
  if (oldmax != max && (oldmax > graphceiling || max > graphceiling)) {
    graphceilingdynamic = std::max(graphceiling, max);
    render();
    return;
  }
  avg = avgsum / data.size();
  if (oldmin != min || oldmax != max || oldavg != avg || oldnow != data.back().data) {
    renderChangingBorderParts();
  }
}

void BrailleGraph::calcMinMax() {
  min = graphceiling;
  max = graphfloor;
  for (const DataPoint& datapoint : data) {
    if (datapoint.data <= min) {
      min = datapoint.data;
      idmin = datapoint.id;
    }
    if (datapoint.data >= max) {
      max = datapoint.data;
      idmax = datapoint.id;
    }
  }
}

void BrailleGraph::render() {
  renderGraph();
  renderStaticBorderParts();
  renderChangingBorderParts();
}

void BrailleGraph::addToBigGraph(std::map<unsigned int, std::set<unsigned int>>& biggraph, unsigned int datapoint, unsigned int datapos, int& lastrow, int& lastcol) {
  int datarow = biggraphrows - ((rowfactor * datapoint) + 0.5);
  int datacol = curroffset + colfactor * (datapos + 1);
  biggraph[datarow].insert(datacol);
  if (lastrow != -1 && (lastrow != datarow || lastcol != datacol - 1)) { // needs interpolation
    unsigned int coldiff = datacol - lastcol;
    if (datarow > lastrow) {
      float incpercol = static_cast<float>(datarow - lastrow) / (coldiff ? coldiff : 1);
      float rowincreased = lastrow;
      for (int intercol = (lastcol == datacol ? lastcol : lastcol + 1); intercol <= datacol; intercol++) {
        for (float currinterrow = rowincreased; currinterrow < rowincreased + incpercol; currinterrow++) {
          biggraph[currinterrow].insert(intercol);
        }
        rowincreased += incpercol;
        if (rowincreased > datarow) {
          break;
        }
      }
    }
    else {
      float decpercol = static_cast<float>(lastrow - datarow) / (coldiff ? coldiff : 1);
      float rowdecreased = lastrow;
      for (int intercol = (lastcol == datacol ? lastcol : lastcol + 1); intercol <= datacol; intercol++) {
        for (float currinterrow = rowdecreased; currinterrow >= rowdecreased - decpercol && rowdecreased >= decpercol; currinterrow--) {
          biggraph[currinterrow].insert(intercol);
        }
        rowdecreased -= decpercol;
        if (rowdecreased < datarow) {
          break;
        }
      }
    }
  }
  lastrow = datarow;
  lastcol = datacol;
}

void BrailleGraph::renderGraph() {
  graphcharseven.clear();
  graphcharsodd.clear();
  unfinishedcol.clear();
  graphrows = 1;
  graphcols = col;
  graphrowstart = 0;
  graphcolstart = 0;
  if (row > 3) {
    graphrows = row - 2;
    graphrowstart = 1;
    graphcols -= 2;
    graphcolstart = 1;
  }
  else if (row > 1) {
    graphrows = row - 1;
  }
  biggraphcols = graphcols * 2;
  biggraphrows = graphrows * 4;
  std::map<unsigned int, std::set<unsigned int>> biggraph;
  colfactor = static_cast<float>(biggraphcols) / data.size();
  rowfactor = static_cast<float>(biggraphrows) / (graphceilingdynamic - graphfloor + 1);
  int datapos = 0;
  min = graphceilingdynamic;
  max = graphfloor;
  avgsum = 0;
  int lastrow = -1;
  int lastcol = -1;
  for (const DataPoint& datapoint : data) {
    avgsum += datapoint.data;
    if (datapoint.data > max) {
      max = datapoint.data;
    }
    if (datapoint.data < min) {
      min = datapoint.data;
    }
    addToBigGraph(biggraph, datapoint.data, datapos, lastrow, lastcol);
    ++datapos;
  }
  avg = avgsum / data.size();
  for (unsigned int i = 0; i < biggraphcols; i++) {
    for (unsigned int j = 0; j < graphrows; j++) {
      unsigned int evenval;
      unsigned int oddval;
      calcGraphChar(biggraph, j, i, biggraphcols, evenval, oddval);
      unsigned int idx = (i + 1) / 2;
      if (evenval) {
        graphcharseven.emplace_back(j + graphrowstart, idx + graphcolstart, evenval + BRAILLE_UNICODE_BASE);
      }
      if (oddval) {
        graphcharsodd.emplace_back(j + graphrowstart, idx + graphcolstart, oddval + BRAILLE_UNICODE_BASE);
      }
    }
  }
  for (unsigned int j = 0; j < biggraphrows; j++) {
    if (biggraph.count(j) && biggraph.at(j).count(biggraphcols - 1)) {
      unfinishedcol.insert(j);
    }
  }
  even = true;
}

void BrailleGraph::renderStaticBorderParts() {
  staticborderchars.clear();
  unsigned int graphrows = 1;
  unsigned int graphcols = col;
  if (row > 3) {
    graphrows = row - 2;
    graphcols -= 2;
    for (unsigned int i = 0; i < graphcols; i++) {
      staticborderchars.emplace_back(0, i + 1, BOX_HLINE);
    }
    for (unsigned int i = 0; i < graphrows; i++) {
      staticborderchars.emplace_back(i + 1, 0, BOX_VLINE);
      staticborderchars.emplace_back(i + 1, col - 1, BOX_VLINE);
    }
    staticborderchars.emplace_back(0, 0, BOX_CORNER_TL);
    staticborderchars.emplace_back(0, col - 1, BOX_CORNER_TR);
    staticborderchars.emplace_back(row - 1, 0, BOX_CORNER_BL);
    staticborderchars.emplace_back(row - 1, col - 1, BOX_CORNER_BR);
  }
  else if (row > 1) {
    graphrows = row - 1;
  }
}

void BrailleGraph::renderChangingBorderParts() {
  changingborderchars.clear();
  unsigned int graphcols = col;
  if (row > 3) {
    graphcols -= 2;
    for (unsigned int i = 0; i < graphcols; i++) {
      changingborderchars.emplace_back(row - 1, i + 1, BOX_HLINE);
    }
    std::string minstr = std::to_string(min) + unit;
    std::string avgstr = std::to_string(avg) + unit;
    std::string maxstr = std::to_string(max) + unit;
    std::string nowstr = std::to_string(data.back().data) + unit;
    std::string floorstr = std::to_string(graphfloor) + unit;
    std::string ceilingstr = std::to_string(graphceilingdynamic) + unit;
    unsigned int pos = 1;
    changingborderchars.emplace_back(0, pos++, ' ');
    for (unsigned int i = 0; i < ceilingstr.length(); ++i) {
      changingborderchars.emplace_back(0, pos++, ceilingstr[i]);
    }
    changingborderchars.emplace_back(0, pos++, ' ');
    pos = 1;
    changingborderchars.emplace_back(row - 1, pos++, ' ');
    for (unsigned int i = 0; i < floorstr.length(); ++i) {
      changingborderchars.emplace_back(row - 1, pos++, floorstr[i]);
    }
    changingborderchars.emplace_back(row - 1, pos++, ' ');
    pos = 10;
    changingborderchars.emplace_back(row - 1, pos++, ' ');
    for (unsigned int i = 0; i < title.length(); ++i) {
      changingborderchars.emplace_back(row - 1, pos++, title[i]);
    }
    changingborderchars.emplace_back(row - 1, pos++, ' ');
    ++pos;
    changingborderchars.emplace_back(row - 1, pos++, ' ');
    changingborderchars.emplace_back(row - 1, pos++, 'N');
    changingborderchars.emplace_back(row - 1, pos++, 'o');
    changingborderchars.emplace_back(row - 1, pos++, 'w');
    changingborderchars.emplace_back(row - 1, pos++, ':');
    changingborderchars.emplace_back(row - 1, pos++, ' ');
    for (unsigned int i = 0; i < nowstr.length(); ++i) {
      changingborderchars.emplace_back(row - 1, pos++, nowstr[i]);
    }
    changingborderchars.emplace_back(row - 1, pos++, ' ');
    ++pos;
    changingborderchars.emplace_back(row - 1, pos++, ' ');
    changingborderchars.emplace_back(row - 1, pos++, 'M');
    changingborderchars.emplace_back(row - 1, pos++, 'i');
    changingborderchars.emplace_back(row - 1, pos++, 'n');
    changingborderchars.emplace_back(row - 1, pos++, ':');
    changingborderchars.emplace_back(row - 1, pos++, ' ');
    for (unsigned int i = 0; i < minstr.length(); ++i) {
      changingborderchars.emplace_back(row - 1, pos++, minstr[i]);
    }
    changingborderchars.emplace_back(row - 1, pos++, ' ');
    pos++;
    changingborderchars.emplace_back(row - 1, pos++, ' ');
    changingborderchars.emplace_back(row - 1, pos++, 'A');
    changingborderchars.emplace_back(row - 1, pos++, 'v');
    changingborderchars.emplace_back(row - 1, pos++, 'g');
    changingborderchars.emplace_back(row - 1, pos++, ':');
    changingborderchars.emplace_back(row - 1, pos++, ' ');
    for (unsigned int i = 0; i < avgstr.length(); ++i) {
      changingborderchars.emplace_back(row - 1, pos++, avgstr[i]);
    }
    changingborderchars.emplace_back(row - 1, pos++, ' ');
    pos++;
    changingborderchars.emplace_back(row - 1, pos++, ' ');
    changingborderchars.emplace_back(row - 1, pos++, 'M');
    changingborderchars.emplace_back(row - 1, pos++, 'a');
    changingborderchars.emplace_back(row - 1, pos++, 'x');
    changingborderchars.emplace_back(row - 1, pos++, ':');
    changingborderchars.emplace_back(row - 1, pos++, ' ');
    for (unsigned int i = 0; i < maxstr.length(); ++i) {
      changingborderchars.emplace_back(row - 1, pos++, maxstr[i]);
    }
    changingborderchars.emplace_back(row - 1, pos++, ' ');
  }
}
