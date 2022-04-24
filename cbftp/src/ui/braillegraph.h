#pragma once

#include <list>
#include <map>
#include <set>
#include <string>

class BrailleGraph {
public:
  struct GraphChar {
    GraphChar(unsigned int row, unsigned int col, unsigned int ch) : row(row), col(col), ch(ch) {}
    unsigned int row;
    unsigned int col;
    unsigned int ch;
  };
  BrailleGraph(unsigned int row, unsigned int col, const std::string& title, const std::string& unit, unsigned int floor, unsigned int ceiling, bool dynamicceiling = false);
  ~BrailleGraph();
  void resize(unsigned int row, unsigned int col, bool redraw);
  void setData(const std::list<unsigned int>& data);
  void addNewData(const std::list<unsigned int>& newdata);
  std::list<GraphChar> getAllChars() const;
  std::list<GraphChar> getGraphChars() const;
  std::list<GraphChar> getChangingBorderChars() const;
  std::list<GraphChar> getStaticBorderChars() const;
  unsigned int rows() const;
private:
  struct DataPoint;
  void renderNew(const std::list<unsigned int>& newdata);
  void render();
  void renderGraph();
  void renderStaticBorderParts();
  void renderChangingBorderParts();
  void addToBigGraph(std::map<unsigned int, std::set<unsigned int>>& biggraph, unsigned int datapoint, unsigned int datapos, int& lastrow, int& lastcol);
  void calcGraphChar(const std::map<unsigned int, std::set<unsigned int>>& biggraph, unsigned int j, unsigned int bgcol, unsigned int cols, unsigned int& evenval, unsigned int& oddval);
  void calcMinMax();
  unsigned int row;
  unsigned int col;
  std::list<DataPoint> data;
  std::list<unsigned int> unrendereddata;
  unsigned int min;
  unsigned int idmin;
  unsigned int max;
  unsigned int idmax;
  unsigned int avg;
  unsigned long long int avgsum;
  unsigned int graphfloor;
  unsigned int graphceiling;
  unsigned int graphceilingdynamic;
  bool dynamicceiling;
  float colfactor;
  float rowfactor;
  unsigned int graphrows;
  unsigned int graphcols;
  unsigned int graphrowstart;
  unsigned int graphcolstart;
  unsigned int biggraphcols;
  unsigned int biggraphrows;
  std::string title;
  std::string unit;
  std::list<GraphChar> graphcharseven;
  std::list<GraphChar> graphcharsodd;
  std::set<unsigned int> unfinishedcol;
  std::list<GraphChar> staticborderchars;
  std::list<GraphChar> changingborderchars;
  bool even;
  float curroffset;
  unsigned int idcounter;
};
