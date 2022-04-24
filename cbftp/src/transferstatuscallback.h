#pragma once

#include <memory>

class TransferStatus;

enum class CallbackType {
  RACE,
  TRANSFERJOB
};

class TransferStatusCallback {
public:
  virtual ~TransferStatusCallback() {}
  virtual void transferFailed(const std::shared_ptr<TransferStatus> &, int) = 0;
  virtual void transferSuccessful(const std::shared_ptr<TransferStatus> &) = 0;
  virtual CallbackType callbackType() const = 0;
};
