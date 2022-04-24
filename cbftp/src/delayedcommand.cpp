#include "delayedcommand.h"

DelayedCommand::DelayedCommand() :
  active(false),
  released(false),
  persisting(false) {
}

void DelayedCommand::set(const std::string & command, unsigned long long int triggertime, const std::weak_ptr<CommandOwner> & co, bool persisting) {
  this->command = command;
  this->triggertime = triggertime;
  this->co = co;
  this->persisting = persisting;
  this->active = true;
  this->released = false;
}

void DelayedCommand::currentTime(unsigned long long int time) {
  if (active && !released && time >= triggertime) {
    released = true;
  }
}

void DelayedCommand::reset() {
  active = false;
  released = false;
}

void DelayedCommand::weakReset() {
  if (!persisting) {
    reset();
  }
}

std::string DelayedCommand::getCommand() const {
  return command;
}

std::shared_ptr<CommandOwner> DelayedCommand::getCommandOwner() const {
  return co.lock();
}

bool DelayedCommand::isActive() const {
  return active;
}

bool DelayedCommand::isReleased() const {
  return released;
}
