#include "sectionutil.h"

#include "path.h"
#include "site.h"

namespace SectionUtil {

util::Result useOrSectionTranslate(const Path& path, const std::shared_ptr<Site>& site, std::string& section, Path& outpath) {
  if (path.isRelative()) {
    section = path.level(0).toString();
    if (site->hasSection(section)) {
      outpath = site->getSectionPath(section) / path.cutLevels(-1);
    }
    else {
      return util::Result(false, "Path must be absolute or a section name on " + site->getName() + ": " + path.toString());
    }
  }
  else {
    section.clear();
    outpath = path;
  }
  return true;
}

util::Result useOrSectionTranslate(const Path& path, const std::shared_ptr<Site>& site, Path& outpath) {
  std::string section;
  return useOrSectionTranslate(path, site, section, outpath);
}

}
