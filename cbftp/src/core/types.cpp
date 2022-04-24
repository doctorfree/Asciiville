#include "types.h"

namespace Core {

StringResult::StringResult() : success(false) {

}

StringResult::StringResult(const char* result) : success(true), result(result) {

}

StringResult::StringResult(const std::string& result) : success(true), result(result) {

}

StringResultError::StringResultError(const std::string& error) {
  this->error = error;
}

}
