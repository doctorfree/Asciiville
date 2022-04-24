#pragma once

#include <memory>
#include <ostream>
#include <string>


class SiteLogic;
class FileList;
class Race;
class SiteRace;

enum class PrioType {
  PRIO,
  PRIO_LATER,
  NORMAL
};

class ScoreBoardElement {
  private:
    std::string filename;
    std::shared_ptr<SiteLogic> src;
    std::shared_ptr<SiteLogic> dst;
    std::shared_ptr<FileList> fls;
    std::shared_ptr<FileList> fld;
    std::shared_ptr<SiteRace> srcsr;
    std::shared_ptr<SiteRace> dstsr;
    std::shared_ptr<Race> race;
    unsigned short score;
    PrioType priotype;
    bool attempted;
    std::string subdir;
    unsigned long long int filesize;
    bool skipchecked;
  public:
    ScoreBoardElement(const std::string&, unsigned short, unsigned long long int filesize, PrioType priotype, const std::shared_ptr<SiteLogic>&,
        const std::shared_ptr<FileList>& fls, const std::shared_ptr<SiteRace>& srcsr, const std::shared_ptr<SiteLogic>&, const std::shared_ptr<FileList>& fld, const std::shared_ptr<SiteRace>& dstsr, const std::shared_ptr<Race>&, const std::string&);
    ~ScoreBoardElement();
    void reset(const std::string&, unsigned short, unsigned long long int filesize, PrioType priotype, const std::shared_ptr<SiteLogic>&,
        const std::shared_ptr<FileList>& fls, const std::shared_ptr<SiteRace>&, const std::shared_ptr<SiteLogic>&, const std::shared_ptr<FileList>& fld, const std::shared_ptr<SiteRace>&, const std::shared_ptr<Race>&, const std::string &);
    void reset(const ScoreBoardElement& other);
    void update(unsigned short);
    void update(unsigned short, unsigned long long int filesize);
    const std::string& fileName() const;
    const std::string& subDir() const;
    const std::shared_ptr<SiteLogic>& getSource() const;
    const std::shared_ptr<SiteLogic>& getDestination() const;
    const std::shared_ptr<FileList>& getSourceFileList() const;
    const std::shared_ptr<FileList>& getDestinationFileList() const;
    const std::shared_ptr<SiteRace>& getSourceSiteRace() const;
    const std::shared_ptr<SiteRace>& getDestinationSiteRace() const;
    const std::shared_ptr<Race>& getRace() const;
    unsigned short getScore() const;
    PrioType getPriorityType() const;
    unsigned long long int getFileSize() const;
    bool wasAttempted() const;
    void setAttempted();
    bool skipChecked() const;
    void setSkipChecked();
    void resetSkipChecked();
};

std::ostream& operator<<(std::ostream& out, const ScoreBoardElement& sbe);
