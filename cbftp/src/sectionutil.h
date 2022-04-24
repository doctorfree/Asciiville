#pragma once

#include <string>
#include <memory>

#include "util.h"

class Path;
class Site;

namespace SectionUtil {

util::Result useOrSectionTranslate(const Path& path, const std::shared_ptr<Site>& site, std::string& section, Path& outpath);
util::Result useOrSectionTranslate(const Path& path, const std::shared_ptr<Site>& site, Path& outpath);

}
