#pragma once

class RestApiCallback {
public:
  virtual ~RestApiCallback() = default;
  virtual void requestHandled(int requestid, const http::Response& response) = 0;
};
