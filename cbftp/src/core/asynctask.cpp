#include "asynctask.h"

namespace Core {

AsyncTask::AsyncTask(EventReceiver* er, int type, void (*taskfunction)(EventReceiver*, int), int data) :
  receiver(er), type(type), taskfunction(taskfunction), taskfunctionp(nullptr), data(data), datap(nullptr)
{
}

AsyncTask::AsyncTask(EventReceiver* er, int type, void (*taskfunction)(EventReceiver*, void*), void* data) :
  receiver(er), type(type), taskfunction(nullptr), taskfunctionp(taskfunction), data(0), datap(data)
{
}

bool AsyncTask::execute() {
  if (taskfunctionp) {
    taskfunctionp(receiver, datap);
  }
  else if (taskfunction) {
    taskfunction(receiver, data);
  }
  else {
    return false;
  }
  return true;
}

EventReceiver* AsyncTask::getReceiver() const {
  return receiver;
}

bool AsyncTask::dataIsPointer() const {
  return taskfunctionp;
}

int AsyncTask::getType() const {
  return type;
}

void* AsyncTask::getData() const {
  return datap;
}

int AsyncTask::getNumData() const {
  return data;
}

} // namespace Core
