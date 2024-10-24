// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_SMART_CARD_MOCK_SMART_CARD_CONTEXT_FACTORY_H_
#define CONTENT_BROWSER_SMART_CARD_MOCK_SMART_CARD_CONTEXT_FACTORY_H_

#include "mojo/public/cpp/bindings/receiver_set.h"
#include "services/device/public/mojom/smart_card.mojom.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace content {

class MockSmartCardContextFactory
    : public device::mojom::SmartCardContextFactory,
      public device::mojom::SmartCardContext {
 public:
  MockSmartCardContextFactory();
  ~MockSmartCardContextFactory() override;

  mojo::PendingRemote<device::mojom::SmartCardContextFactory> GetRemote();

  // `device::mojom::SmartCardContextFactory` overrides:
  void CreateContext(CreateContextCallback) override;

  // `device::mojom::SmartCardContext` overrides:
  MOCK_METHOD(void, ListReaders, (ListReadersCallback callback), (override));
  MOCK_METHOD(
      void,
      GetStatusChange,
      (base::TimeDelta timeout,
       std::vector<device::mojom::SmartCardReaderStateInPtr> reader_states,
       GetStatusChangeCallback callback),
      (override));
  MOCK_METHOD(void, Cancel, (CancelCallback callback), (override));
  MOCK_METHOD(void,
              Connect,
              (const std::string& reader,
               device::mojom::SmartCardShareMode share_mode,
               device::mojom::SmartCardProtocolsPtr preferred_protocols,
               ConnectCallback callback),
              (override));

 private:
  mojo::ReceiverSet<device::mojom::SmartCardContextFactory> receivers_;
  mojo::ReceiverSet<device::mojom::SmartCardContext> context_receivers_;
};

}  // namespace content

#endif  // CONTENT_BROWSER_SMART_CARD_MOCK_SMART_CARD_CONTEXT_FACTORY_H_
