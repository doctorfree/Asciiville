#pragma once

#include <csignal>

namespace Core {

class EventReceiver;
class WorkManager;

void registerSignalDispatch(int sig, WorkManager* wm, EventReceiver* er);
void registerSignalHandler(int sig, void (*sighandler)(int));
void blockAllRegisteredSignals();

} // namespace Core
