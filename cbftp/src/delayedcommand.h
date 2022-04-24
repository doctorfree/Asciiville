#pragma once

#include <string>
#include <memory>

class CommandOwner;

class DelayedCommand {
private:
  std::string command;
  unsigned long long int triggertime;
  std::weak_ptr<CommandOwner> co;
  bool active;
  bool released;
  bool persisting;
public:
  DelayedCommand();
  void set(const std::string & command, unsigned long long int triggertime, const std::weak_ptr<CommandOwner> & co = std::weak_ptr<CommandOwner>(), bool persisting = false);
  void currentTime(unsigned long long int);
  void reset();
  void weakReset();
  std::string getCommand() const;
  std::shared_ptr<CommandOwner> getCommandOwner() const;
  bool isActive() const;
  bool isReleased() const;
  bool isPersisting() const;
};
