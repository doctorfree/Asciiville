#pragma once

namespace Core {

class EventReceiver;

class AsyncTask {
public:
  AsyncTask(EventReceiver* er, int type, void (*taskfunction)(EventReceiver*, int), int data);
  AsyncTask(EventReceiver* er, int type, void (*taskfunctionp)(EventReceiver*, void*), void* data);
  bool execute();
  EventReceiver* getReceiver() const;
  int getType() const;
  bool dataIsPointer() const;
  void* getData() const;
  int getNumData() const;
private:
  EventReceiver* receiver;
  int type;
  void (*taskfunction)(EventReceiver*, int);
  void (*taskfunctionp)(EventReceiver*, void*);
  int data;
  void* datap;
};

} // namespace Core
