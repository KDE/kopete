/*
 * libjingle
 * Copyright 2004, Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <string>

#include "talk/base/gunit.h"
#include "talk/base/host.h"
#include "talk/base/logging.h"
#include "talk/base/natserver.h"
#include "talk/base/natsocketfactory.h"
#include "talk/base/network.h"
#include "talk/base/physicalsocketserver.h"
#include "talk/base/testclient.h"
#include "talk/base/virtualsocketserver.h"

using namespace talk_base;

bool CheckReceive(
    TestClient* client, bool should_receive, const char* buf, size_t size) {
  return (should_receive) ?
      client->CheckNextPacket(buf, size, 0) :
      client->CheckNoPacket();
}

TestClient* CreateTestClient(
      SocketFactory* factory, const SocketAddress& local_addr) {
  AsyncUDPSocket* socket = AsyncUDPSocket::Create(factory, local_addr);
  return new TestClient(socket);
}

// Tests that when sending from internal_addr to external_addrs through the
// NAT type specified by nat_type, all external addrs receive the sent packet
// and, if exp_same is true, all use the same mapped-address on the NAT.
void TestSend(
      SocketServer* internal, const SocketAddress& internal_addr,
      SocketServer* external, const SocketAddress external_addrs[4],
      NATType nat_type, bool exp_same) {
  Thread th_int(internal);
  Thread th_ext(external);

  SocketAddress server_addr = internal_addr;
  server_addr.SetPort(0);  // Auto-select a port
  NATServer* nat = new NATServer(
      nat_type, internal, server_addr, external, external_addrs[0]);
  NATSocketFactory* natsf = new NATSocketFactory(internal,
                                                 nat->internal_address());

  TestClient* in = CreateTestClient(natsf, internal_addr);
  TestClient* out[4];
  for (int i = 0; i < 4; i++)
    out[i] = CreateTestClient(external, external_addrs[i]);

  th_int.Start();
  th_ext.Start();

  const char* buf = "filter_test";
  size_t len = strlen(buf);

  in->SendTo(buf, len, out[0]->address());
  SocketAddress trans_addr;
  EXPECT_TRUE(out[0]->CheckNextPacket(buf, len, &trans_addr));

  for (int i = 1; i < 4; i++) {
    in->SendTo(buf, len, out[i]->address());
    SocketAddress trans_addr2;
    EXPECT_TRUE(out[i]->CheckNextPacket(buf, len, &trans_addr2));
    bool are_same = (trans_addr == trans_addr2);
    ASSERT_EQ(are_same, exp_same) << "same translated address";
  }

  th_int.Stop();
  th_ext.Stop();

  delete nat;
  delete natsf;
  delete in;
  for (int i = 0; i < 4; i++)
    delete out[i];
}

// Tests that when sending from external_addrs to internal_addr, the packet
// is delivered according to the specified filter_ip and filter_port rules.
void TestRecv(
      SocketServer* internal, const SocketAddress& internal_addr,
      SocketServer* external, const SocketAddress external_addrs[4],
      NATType nat_type, bool filter_ip, bool filter_port) {
  Thread th_int(internal);
  Thread th_ext(external);

  SocketAddress server_addr = internal_addr;
  server_addr.SetPort(0);  // Auto-select a port
  NATServer* nat = new NATServer(
      nat_type, internal, server_addr, external, external_addrs[0]);
  NATSocketFactory* natsf = new NATSocketFactory(internal,
                                                 nat->internal_address());

  TestClient* in = CreateTestClient(natsf, internal_addr);
  TestClient* out[4];
  for (int i = 0; i < 4; i++)
    out[i] = CreateTestClient(external, external_addrs[i]);

  th_int.Start();
  th_ext.Start();

  const char* buf = "filter_test";
  size_t len = strlen(buf);

  in->SendTo(buf, len, out[0]->address());
  SocketAddress trans_addr;
  EXPECT_TRUE(out[0]->CheckNextPacket(buf, len, &trans_addr));

  out[1]->SendTo(buf, len, trans_addr);
  EXPECT_TRUE(CheckReceive(in, !filter_ip, buf, len));

  out[2]->SendTo(buf, len, trans_addr);
  EXPECT_TRUE(CheckReceive(in, !filter_port, buf, len));

  out[3]->SendTo(buf, len, trans_addr);
  EXPECT_TRUE(CheckReceive(in, !filter_ip && !filter_port, buf, len));

  th_int.Stop();
  th_ext.Stop();

  delete nat;
  delete natsf;
  delete in;
  for (int i = 0; i < 4; i++)
    delete out[i];
}

// Tests that NATServer allocates bindings properly.
void TestBindings(
    SocketServer* internal, const SocketAddress& internal_addr,
    SocketServer* external, const SocketAddress external_addrs[4]) {
  TestSend(internal, internal_addr, external, external_addrs,
           NAT_OPEN_CONE, true);
  TestSend(internal, internal_addr, external, external_addrs,
           NAT_ADDR_RESTRICTED, true);
  TestSend(internal, internal_addr, external, external_addrs,
           NAT_PORT_RESTRICTED, true);
  TestSend(internal, internal_addr, external, external_addrs,
           NAT_SYMMETRIC, false);
}

// Tests that NATServer filters packets properly.
void TestFilters(
    SocketServer* internal, const SocketAddress& internal_addr,
    SocketServer* external, const SocketAddress external_addrs[4]) {
  TestRecv(internal, internal_addr, external, external_addrs,
           NAT_OPEN_CONE, false, false);
  TestRecv(internal, internal_addr, external, external_addrs,
           NAT_ADDR_RESTRICTED, true, false);
  TestRecv(internal, internal_addr, external, external_addrs,
           NAT_PORT_RESTRICTED, true, true);
  TestRecv(internal, internal_addr, external, external_addrs,
           NAT_SYMMETRIC, true, true);
}

TEST(NatTest, TestPhysical) {
  BasicNetworkManager network_manager;
  network_manager.StartUpdating();
  // Process pending messages so the network list is updated.
  Thread::Current()->ProcessMessages(0);

  std::vector<Network*> networks;
  network_manager.GetNetworks(&networks);
  if (networks.empty()) {
    LOG(LS_WARNING) << "Not enough network adapters for test.";
    return;
  }

  SocketAddress int_addr("127.0.0.1", 0);
  std::string ext_ip1 = "127.0.0.1";
  std::string ext_ip2 = networks[0]->ip().ToString();

  LOG(LS_INFO) << "selected ip " << ext_ip2;

  SocketAddress ext_addrs[4] = {
      SocketAddress(ext_ip1, 0),
      SocketAddress(ext_ip2, 0),
      SocketAddress(ext_ip1, 0),
      SocketAddress(ext_ip2, 0)
  };

  PhysicalSocketServer* int_pss = new PhysicalSocketServer();
  PhysicalSocketServer* ext_pss = new PhysicalSocketServer();

  TestBindings(int_pss, int_addr, ext_pss, ext_addrs);
  TestFilters(int_pss, int_addr, ext_pss, ext_addrs);
}

class TestVirtualSocketServer : public VirtualSocketServer {
 public:
  explicit TestVirtualSocketServer(SocketServer* ss)
      : VirtualSocketServer(ss) {}
  // Expose this publicly
  IPAddress GetNextIP(int af) { return VirtualSocketServer::GetNextIP(af); }
};

TEST(NatTest, TestVirtual) {
  TestVirtualSocketServer* int_vss = new TestVirtualSocketServer(
      new PhysicalSocketServer());
  TestVirtualSocketServer* ext_vss = new TestVirtualSocketServer(
      new PhysicalSocketServer());

  // TODO: IPv6ize this test when the NAT stuff is v6ed.
  SocketAddress int_addr, ext_addrs[4];
  int_addr.SetIP(int_vss->GetNextIP(int_addr.ipaddr().family()));
  ext_addrs[0].SetIP(ext_vss->GetNextIP(int_addr.ipaddr().family()));
  ext_addrs[1].SetIP(ext_vss->GetNextIP(int_addr.ipaddr().family()));
  ext_addrs[2].SetIP(ext_addrs[0].ipaddr());
  ext_addrs[3].SetIP(ext_addrs[1].ipaddr());

  TestBindings(int_vss, int_addr, ext_vss, ext_addrs);
  TestFilters(int_vss, int_addr, ext_vss, ext_addrs);
}

// TODO: Finish this test
class NatTcpTest : public testing::Test, public sigslot::has_slots<> {
 public:
  NatTcpTest() : connected_(false) {}
  virtual void SetUp() {
    int_vss_ = new TestVirtualSocketServer(new PhysicalSocketServer());
    ext_vss_ = new TestVirtualSocketServer(new PhysicalSocketServer());
    nat_ = new NATServer(NAT_OPEN_CONE, int_vss_, SocketAddress(),
                         ext_vss_, SocketAddress());
    natsf_ = new NATSocketFactory(int_vss_, nat_->internal_address());
  }
  void OnConnectEvent(AsyncSocket* socket) {
    connected_ = true;
  }
  void OnAcceptEvent(AsyncSocket* socket) {
    accepted_ = server_->Accept(NULL);
  }
  void OnCloseEvent(AsyncSocket* socket, int error) {
  }
  void ConnectEvents() {
    server_->SignalReadEvent.connect(this, &NatTcpTest::OnAcceptEvent);
    client_->SignalConnectEvent.connect(this, &NatTcpTest::OnConnectEvent);
  }
  TestVirtualSocketServer* int_vss_;
  TestVirtualSocketServer* ext_vss_;
  NATServer* nat_;
  NATSocketFactory* natsf_;
  AsyncSocket* client_;
  AsyncSocket* server_;
  AsyncSocket* accepted_;
  bool connected_;
};

TEST_F(NatTcpTest, DISABLED_TestConnectOut) {
  server_ = ext_vss_->CreateAsyncSocket(SOCK_STREAM);
  server_->Bind(SocketAddress());
  server_->Listen(5);

  client_ = int_vss_->CreateAsyncSocket(SOCK_STREAM);
  EXPECT_GE(0, client_->Bind(SocketAddress()));
  EXPECT_GE(0, client_->Connect(server_->GetLocalAddress()));

  ConnectEvents();

  EXPECT_TRUE_WAIT(connected_, 1000);
  EXPECT_EQ(client_->GetRemoteAddress(), server_->GetLocalAddress());
  EXPECT_EQ(client_->GetRemoteAddress(), accepted_->GetLocalAddress());
  EXPECT_EQ(client_->GetLocalAddress(), accepted_->GetRemoteAddress());

  client_->Close();
}
//#endif
