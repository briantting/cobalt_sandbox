// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/url_identity.h"

#include <string>

#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/types/expected.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/url_constants.h"
#include "components/url_formatter/elide_url.h"
#include "components/url_formatter/url_formatter.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "url/gurl.h"

#if !BUILDFLAG(IS_ANDROID)
#include "chrome/browser/web_applications/isolated_web_apps/isolated_web_app_url_info.h"
#include "chrome/browser/web_applications/web_app_id.h"
#include "chrome/browser/web_applications/web_app_provider.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/common/constants.h"
#include "extensions/common/extension.h"
#endif  // !BUILDFLAG(IS_ANDROID)

using Type = UrlIdentity::Type;
using DefaultFormatOptions = UrlIdentity::DefaultFormatOptions;
using FormatOptions = UrlIdentity::FormatOptions;

namespace {

UrlIdentity CreateDefaultUrlIdentityFromUrl(const GURL& url,
                                            const FormatOptions& options) {
  std::u16string name;

  if (options.default_options.Has(DefaultFormatOptions::kRawSpec)) {
    return UrlIdentity{
        .type = Type::kDefault,
        .name = base::CollapseWhitespace(base::UTF8ToUTF16(url.spec()), false)};
  }
  if (options.default_options.Has(
          DefaultFormatOptions::kOmitCryptographicScheme)) {
    name = url_formatter::FormatUrlForSecurityDisplay(
        url, url_formatter::SchemeDisplay::OMIT_CRYPTOGRAPHIC);
  } else if (options.default_options.Has(DefaultFormatOptions::kHostname)) {
    name = url_formatter::IDNToUnicode(url.host());
  } else {
    name = url_formatter::FormatUrlForSecurityDisplay(url);
  }

  return UrlIdentity{.type = Type::kDefault, .name = name};
}

#if !BUILDFLAG(IS_ANDROID)
UrlIdentity CreateChromeExtensionIdentityFromUrl(Profile* profile,
                                                 const GURL& url,
                                                 const FormatOptions& options) {
  DCHECK(url.SchemeIs(extensions::kExtensionScheme));

  DCHECK(profile) << "Profile cannot be null when type is Chrome Extensions.";

  extensions::ExtensionRegistry* extension_registry =
      extensions::ExtensionRegistry::Get(profile);
  DCHECK(extension_registry);

  const extensions::Extension* extension = nullptr;
  extension = extension_registry->enabled_extensions().GetByID(url.host());

  if (!extension) {  // fallback to default
    return CreateDefaultUrlIdentityFromUrl(url, options);
  }

  return UrlIdentity{.type = Type::kChromeExtension,
                     .name = base::CollapseWhitespace(
                         base::UTF8ToUTF16(extension->name()), false)};
}

absl::optional<web_app::AppId> GetIsolatedWebAppIdFromUrl(const GURL& url) {
  base::expected<web_app::IsolatedWebAppUrlInfo, std::string> url_info =
      web_app::IsolatedWebAppUrlInfo::Create(url);
  return url_info.has_value() ? absl::make_optional(url_info.value().app_id())
                              : absl::nullopt;
}

UrlIdentity CreateIsolatedWebAppIdentityFromUrl(Profile* profile,
                                                const GURL& url,
                                                const FormatOptions& options) {
  DCHECK(url.SchemeIs(chrome::kIsolatedAppScheme));

  DCHECK(profile) << "Profile cannot be null when type is Isolated Web App.";

  web_app::WebAppProvider* provider =
      web_app::WebAppProvider::GetForWebApps(profile);
  if (!provider) {  // fallback to default
    // WebAppProvider can be null in ChromeOS depending on whether Lacros is
    // enabled or not.
    return CreateDefaultUrlIdentityFromUrl(url, options);
  }

  absl::optional<web_app::AppId> app_id = GetIsolatedWebAppIdFromUrl(url);
  if (!app_id.has_value()) {  // fallback to default
    return CreateDefaultUrlIdentityFromUrl(url, options);
  }

  const web_app::WebApp* web_app =
      provider->registrar_unsafe().GetAppById(app_id.value());

  if (!web_app) {  // fallback to default
    return CreateDefaultUrlIdentityFromUrl(url, options);
  }

  return UrlIdentity{
      .type = Type::kIsolatedWebApp,
      .name = base::CollapseWhitespace(
          base::UTF8ToUTF16(
              provider->registrar_unsafe().GetAppShortName(app_id.value())),
          false)};
}
#endif  // !BUILDFLAG(IS_ANDROID)

UrlIdentity CreateFileIdentityFromUrl(Profile* profile,
                                      const GURL& url,
                                      const FormatOptions& options) {
  DCHECK(url.SchemeIsFile());

  return UrlIdentity{
      .type = Type::kFile,
      .name = url_formatter::FormatUrlForSecurityDisplay(url),
  };
}
}  // namespace

UrlIdentity UrlIdentity::CreateFromUrl(Profile* profile,
                                       const GURL& url,
                                       const TypeSet& allowed_types,
                                       const FormatOptions& options) {
#if !BUILDFLAG(IS_ANDROID)
  if (url.SchemeIs(extensions::kExtensionScheme)) {
    DCHECK(allowed_types.Has(Type::kChromeExtension));
    return CreateChromeExtensionIdentityFromUrl(profile, url, options);
  }

  if (url.SchemeIs(chrome::kIsolatedAppScheme)) {
    DCHECK(allowed_types.Has(Type::kIsolatedWebApp));
    return CreateIsolatedWebAppIdentityFromUrl(profile, url, options);
  }
#endif  // !BUILDFLAG(IS_ANDROID)

  if (url.SchemeIsFile()) {
    DCHECK(allowed_types.Has(Type::kFile));
    return CreateFileIdentityFromUrl(profile, url, options);
  }

  DCHECK(allowed_types.Has(Type::kDefault));
  return CreateDefaultUrlIdentityFromUrl(url, options);
}
