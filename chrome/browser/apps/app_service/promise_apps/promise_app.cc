// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/apps/app_service/promise_apps/promise_app.h"

#include <iostream>

#include "base/strings/string_number_conversions.h"
#include "components/services/app_service/public/cpp/macros.h"

namespace apps {

APP_ENUM_TO_STRING(PromiseStatus, kUnknown, kPending, kInstalling)

PromiseApp::PromiseApp(const apps::PackageId& package_id)
    : package_id(package_id) {}
PromiseApp::~PromiseApp() = default;

PromiseAppPtr PromiseApp::Clone() const {
  auto promise_app = std::make_unique<PromiseApp>(package_id);
  if (name.has_value()) {
    promise_app->name = name;
  }
  if (progress.has_value()) {
    promise_app->progress = progress;
  }
  promise_app->status = status;
  promise_app->should_show = should_show;
  return promise_app;
}

std::ostream& operator<<(std::ostream& out, const PromiseApp& promise_app) {
  out << "Package_id: " << promise_app.package_id.ToString() << std::endl;
  out << "- Name: " << promise_app.name.value_or("N/A") << std::endl;
  out << "- Progress: "
      << (promise_app.progress.has_value()
              ? base::NumberToString(promise_app.progress.value())
              : "N/A")
      << std::endl;
  out << "- Status: " << EnumToString(promise_app.status) << std::endl;
  out << "- Should Show: " << promise_app.should_show << std::endl;
  return out;
}

}  // namespace apps
