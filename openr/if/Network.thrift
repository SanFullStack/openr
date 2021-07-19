/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

namespace cpp openr.thrift
namespace cpp2 openr.thrift
namespace go openr.Network
namespace py openr.Network
namespace py3 openr.thrift
namespace php Openr
namespace lua openr.Network
namespace wiki Open_Routing.Thrift_APIs.Network

// Using the defaults from here:
// https://en.wikipedia.org/wiki/Administrative_distance
enum AdminDistance {
  DIRECTLY_CONNECTED = 0,
  STATIC_ROUTE = 1,
  EBGP = 20,
  IBGP = 200,
  NETLINK_LISTENER = 225,
  MAX_ADMIN_DISTANCE = 255,
}

enum MplsActionCode {
  PUSH = 0,
  SWAP = 1,
  PHP = 2, # Pen-ultimate hop popping => POP and FORWARD
  POP_AND_LOOKUP = 3,
  NOOP = 4,
}

// For mimicing FBOSS agent thrift interfaces
enum PortAdminState {
  DISABLED = 0,
  ENABLED = 1,
}

// For mimicing FBOSS agent thrift interfaces
enum PortOperState {
  DOWN = 0,
  UP = 1,
}

struct MplsAction {
  1: MplsActionCode action;
  2: optional i32 swapLabel; // Required if action == SWAP
  // front() (index=0) in list will be bottom of stack and back()
  // element is top of the stack
  3: optional list<i32> pushLabels; // Required if action == PUSH
} (cpp.minimize_padding)

struct BinaryAddress {
  1: binary addr;
  3: optional string ifName;
} (cpp.minimize_padding)

struct IpPrefix {
  1: BinaryAddress prefixAddress;
  2: i16 prefixLength;
} (cpp.minimize_padding)

struct NextHopThrift {
  1: BinaryAddress address;
  // Default weight of 0 represents an ECMP route.
  // This default is chosen for two reasons:
  // 1) We rely on the arithmetic properties of 0 for ECMP vs UCMP route
  //    resolution calculations. A 0 weight next hop being present at a variety
  //    of layers in a route resolution tree will cause the entire route
  //    resolution to use ECMP.
  // 2) A client which does not set a value will result in
  //    0 being populated even with strange behavior in the client language
  //    which is consistent with C++
  2: i32 weight = 0;
  // MPLS encapsulation information for IP->MPLS and MPLS routes
  3: optional MplsAction mplsAction;

  // Metric (aka cost) associated with this nexthop
  51: i32 metric = 0;

  //
  // TODO: Define internal representation of NextHop within Open/R. We shouldn't
  // expose internal attributes to outside. `metric` though internal as of now,
  // is going to be useful when FBOSS supports it underneath
  //

  // Area field associated with next-hop. This is derived from an adjacency,
  // from where the transport address is also derived. This can be none for
  // imported routes.
  53: optional string area;

  // Name of next-hop device
  54: optional string neighborNodeName;
} (cpp.minimize_padding)

struct ClientAndNextHops {
  1: required i32 clientId;
  3: required list<NextHopThrift> nextHops;
}

struct RouteDetails {
  1: IpPrefix dest;
  2: string action;
  4: list<ClientAndNextHops> nextHopMulti;
  5: bool isConnected;
  6: optional AdminDistance adminDistance;
  7: list<NextHopThrift> nextHops;
} (cpp.minimize_padding)

struct MplsRoute {
  1: i32 topLabel;
  3: optional AdminDistance adminDistance;
  4: list<NextHopThrift> nextHops;
} (cpp.minimize_padding)

enum PrefixType {
  LOOPBACK = 1,
  DEFAULT = 2,
  BGP = 3,
  PREFIX_ALLOCATOR = 4,
  BREEZE = 5, // Prefixes injected via breeze
  RIB = 6,
  SLO_PREFIX_ALLOCATOR = 7,
  CONFIG = 8, // Route Origination
  VIP = 9, // VIP injected by vip service

  // Placeholder Types
  TYPE_1 = 21,
  TYPE_2 = 22,
  TYPE_3 = 23,
  TYPE_4 = 24,
  TYPE_5 = 25,
}

struct UnicastRoute {
  1: IpPrefix dest;
  3: optional AdminDistance adminDistance;
  4: list<NextHopThrift> nextHops;
} (cpp.minimize_padding)

// For mimicing FBOSS agent thrift interfaces
struct LinkNeighborThrift {
  1: i32 localPort;
  2: i32 localVlan;
  11: string printablePortId;
  12: optional string systemName;
} (cpp.minimize_padding)

struct PortCounters {
  // avoid typechecker error here as bytes is a py3 reserved keyword
  1: i64 bytes (py3.name = "bytes_");
  2: i64 ucastPkts;
}

// For mimicing FBOSS agent thrift interfaces
struct PortInfoThrift {
  1: i32 portId;
  2: i64 speedMbps;
  3: PortAdminState adminState;
  4: PortOperState operState;
  10: PortCounters output;
  11: PortCounters input;
  12: string name;
} (cpp.minimize_padding)

// For mimicing FBOSS agent thrift interface
struct PortStatus {
  1: bool enabled;
  2: bool up;
  5: i64 speedMbps;
// NOTE: Some fields are omitted intentionally as it is not supported
// in Emulation!
} (cpp.minimize_padding)

// For mimicing FBOSS agent thrift interface
struct AggregatePortThrift {
// NOTE: Left empty as we're not exposing any data
}
