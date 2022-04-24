#include "path.h"

Path::Path() : separatorchar('/'), absolute(false), separatorcertain(false) {

}

Path::Path(const char * inpath) : path(inpath), separatorchar('/'), absolute(false), separatorcertain(false) {
  setSeparatorAbsolute();
  if (separatorcertain) {
    flipSlashes();
    cleanPath();
  }
  setPathParts();
}

Path::Path(const char * inpath, char separator, bool separatorcertain) :
    path(inpath), separatorchar(separator), absolute(false), separatorcertain(separatorcertain)
{
  if (path.length() && path[0] == separator) {
    absolute = true;
  }
  setPathParts();
}

Path::Path(const std::string & inpath) : path(inpath), separatorchar('/'), absolute(false), separatorcertain(false) {
  setSeparatorAbsolute();
  if (separatorcertain) {
    flipSlashes();
    cleanPath();
  }
  setPathParts();
}

Path::Path(const std::string & inpath, char separator, bool separatorcertain) :
    path(inpath), separatorchar(separator), absolute(false), separatorcertain(separatorcertain)
{
  if (path.length() && path[0] == separator) {
    absolute = true;
  }
  setPathParts();
}

Path::Path(const Path & base, const Path & add) {
  separatorchar = base.separatorcertain ? base.separatorchar : add.separatorchar;
  absolute = base.path.length() ? base.absolute : add.absolute;
  separatorcertain = base.separatorcertain || add.separatorcertain;
  path = base.path;
  if (!add.absolute && base.path.length() > 1 && add.path.length()) {
    path += separatorchar;
  }
  path += add.path;
  flipSlashes();
  setPathParts();
}

void Path::setSeparatorAbsolute() {
  if (path.length()) {
    if (path[0] == '/' || path[0] == '\\') {
      absolute = true;
      separatorchar = path[0];
      separatorcertain = true;
    }
    else {
      if (path.find("/") != std::string::npos) {
        separatorchar = '/';
        separatorcertain = true;
      }
      else if (path.find("\\") != std::string::npos) {
        separatorchar = '\\';
        separatorcertain = true;
      }
    }
  }
}

void Path::flipSlashes() {
  size_t pos;
  if (separatorchar == '/') {
    while ((pos = path.find("\\")) != std::string::npos) {
      path[pos] = '/';
    }
  }
  else if (separatorchar == '\\') {
    while ((pos = path.find("/")) != std::string::npos) {
      path[pos] = '\\';
    }
  }
}

void Path::cleanPath() {
  size_t pos;
  if (separatorchar == '/') {
    while ((pos = path.find("//")) != std::string::npos) {
      path = path.substr(0, pos) + path.substr(pos + 1);
    }
  }
  else if (separatorchar == '\\') {
    while ((pos = path.find("\\\\")) != std::string::npos) {
      path = path.substr(0, pos) + path.substr(pos + 1);
    }
  }
  int ipos = path.length() - 1;
  if (ipos > 0 && (path[ipos] == '/' || path[ipos] == '\\')) {
    path = path.substr(0, ipos);
  }
}

void Path::setPathParts() {
  unixpath = path;
  size_t pos;
  if (separatorchar == '\\') {
    while ((pos = unixpath.find("\\")) != std::string::npos) {
      unixpath[pos] = '/';
    }
  }
  pos = unixpath.rfind("/");
  if (pos != std::string::npos) {
    if (pos == 0) {
      dirname = path.substr(0, 1);
    }
    else {
      dirname = path.substr(0, pos);
    }
    basename = path.substr(pos + 1);
  }
  else {
    basename = path;
  }
}

const std::string & Path::toString() const {
  return path;
}

const std::string & Path::unixPath() const {
  return unixpath;
}

const std::string & Path::dirName() const {
  return dirname;
}

const std::string & Path::baseName() const {
  return basename;
}

bool Path::isAbsolute() const {
  return absolute;
}

bool Path::isRelative() const {
  return !absolute;
}

Path Path::operator/(const std::string & rhs) const {
  return Path(*this, Path(rhs));
}

Path Path::operator/(const Path & rhs) const {
  return Path(*this, rhs);
}

Path Path::operator/(const char * rhs) const {
  return Path(*this, rhs);
}

bool Path::operator==(const Path & rhs) const {
  return unixpath == rhs.unixpath;
}

bool Path::operator!=(const Path & rhs) const {
  return unixpath != rhs.unixpath;
}

bool Path::contains(const Path & rhs) const {
  if (unixpath.length() >= rhs.unixpath.length() &&
      unixpath.compare(0, rhs.unixpath.length(), rhs.unixpath) == 0)
  {
    if (unixpath.length() == rhs.unixpath.length() ||
        unixpath[rhs.unixpath.length()] == '/')
    {
      return true;
    }
  }
  return false;
}

Path Path::operator-(const Path & rhs) const {
  if (absolute && rhs.absolute) {
    if (unixpath.length() > rhs.unixpath.length()) {
      if (unixpath.compare(0, rhs.unixpath.length() + 1, rhs.unixpath + '/') == 0) {
        return Path(unixpath.substr(rhs.unixpath.length() + 1), separatorchar, separatorcertain);
      }
      else if (rhs.unixpath.length() == 1 && unixpath.compare(0, rhs.unixpath.length(), rhs.unixpath) == 0) {
        return Path(unixpath.substr(rhs.unixpath.length()), separatorchar, separatorcertain);
      }
    }
  }
  else if (!rhs.absolute) {
    int lendiff = unixpath.length() - rhs.unixpath.length();
      if (lendiff > 0 &&
          unixpath.compare(lendiff - 1, rhs.unixpath.length() + 1, '/' + rhs.unixpath) == 0)
      {
        return Path(unixpath.substr(0, lendiff), separatorchar, separatorcertain);
      }
  }
  if (unixpath == rhs.unixpath) {
    return Path("", separatorchar, separatorcertain);
  }
  return *this;
}

std::list<std::string> Path::split() const {
  std::list<std::string> parts;
  size_t lastpos = 0;
  while (true) {
    size_t splitpos = unixpath.find("/", lastpos);
    if (splitpos != std::string::npos) {
      parts.push_back(unixpath.substr(lastpos, splitpos - lastpos));
    }
    else {
      parts.push_back(unixpath.substr(lastpos));
      break;
    }
    lastpos = splitpos + 1;
  }
  return parts;
}

int Path::levels() const {
  size_t lastpos = 0;
  int count = absolute ? 0 : 1;
  size_t splitpos;
  while ((splitpos = unixpath.find("/", lastpos)) != std::string::npos) {
    lastpos = splitpos + 1;
    ++count;
  }
  return count;
}

Path Path::cutLevels(int levels) const {
  if (levels > 0) {
    size_t splitpos;
    int lastpos = unixpath.length();
    int cut = 0;
    while (lastpos > 0 && (splitpos = unixpath.rfind("/", lastpos)) != std::string::npos) {
      if (++cut == levels) {
        if (splitpos == 0) {
          splitpos++;
        }
        return Path(unixpath.substr(0, splitpos), separatorchar, separatorcertain);
      }
      lastpos = splitpos - 1;
    }
    return absolute ? "/" : Path("", separatorchar, separatorcertain);
  }
  else if (levels < 0) {
    size_t lastpos = 0;
    int atlevel = absolute ? 0 : 1;
    size_t splitpos;
    int targetlevels = -levels;
    while ((splitpos = unixpath.find("/", lastpos)) != std::string::npos) {
      lastpos = splitpos + 1;
      if (atlevel++ == targetlevels) {
        if (lastpos == 1) {
          --lastpos;
        }
        return Path(unixpath.substr(lastpos), separatorchar, separatorcertain);
      }
    }
    return Path("", separatorchar, separatorcertain);
  }
  return *this;
}


Path Path::level(int level) const {
  size_t lastpos = 0;
  int atlevel = absolute ? -1 : 0;
  size_t splitpos;
  while ((splitpos = unixpath.find("/", lastpos)) != std::string::npos) {
    if (atlevel++ == level) {
      if (lastpos == 1) {
        --lastpos;
      }
      return Path(unixpath.substr(lastpos, splitpos - lastpos), separatorchar, separatorcertain);
    }
    lastpos = splitpos + 1;
  }
  if (level == atlevel) {
    if (lastpos == 1) {
      --lastpos;
    }
    return Path(unixpath.substr(lastpos), separatorchar, separatorcertain);
  }
  return Path("", separatorchar, separatorcertain);
}

bool Path::operator<(const Path& other) const {
  return toString() < other.toString();
}

std::ostream & operator<<(std::ostream & out, const Path & path) {
  return out << path.toString();
}

bool Path::empty() const {
  return unixpath.empty();
}
