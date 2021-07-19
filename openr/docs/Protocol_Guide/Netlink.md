# Netlink C++ Interface

## Introduction

---

This library provides C++ APIs to interact with kernel for networking state via
netlink protocol. The functionality is intentionally kept minimal to the needs
of Open/R, but it can be extended. Below we describe the organization of code in
this directory. Refer to inline code documentation in header files for more
information.

## Deep Dive

---

### NetlinkTypes

Defines C++ classes to represent various routing information e.g. Address, Link,
Route etc. This serves as data representation between application code and
netlink C++ library.

### NetlinkMessageBase

Defines base class that facilitates serialization and de-serialization of
message exchange with linux kernel.

Reference: http://man7.org/linux/man-pages/man7/netlink.7.html

### Addr/Link/Neighbor/Route

Specialization of netlink message for `NETLINK_ROUTE` (aka rtnetlink) family. It
implements Addresses, Link, Neighbor and Route (IP, IPv6 & MPLS).

Reference: http://man7.org/linux/man-pages/man7/rtnetlink.7.html

For example, a sample message layout of `RTM_NEWROUTE` will be like:

![Layout of sample RTM_NEWROUTE msg](https://user-images.githubusercontent.com/51382140/113812173-799dd080-9722-11eb-80d3-220dc3907fab.png)

### NetlinkProtocolSocket

Implements send and receive netlink messages with the use of `AF_NETLINK` socket
type. Also provides public APIs to get/add/del for Addresses, Link, Neighbor and
Routes.

### Unit Testing

Netlink code is exhaustively unit-tested. However to run unit-tests, you'll need
to run with `sudo`. Temporary route, link, address and neighbor entries will be
created in the network. Hence it is best to run inside the container

For MPLS route unit-tests you'll need to have MPLS enabled kernel with following
sysctl configuration parameters.

Some unit-tests tries to add upto 100k routes and can consume significant amount
of memory and time to run. So please be patient or disable those tests if you
don't want your system to scale to that number.

```
// Install kernel modules
sudo modprobe mpls_router
sudo modprobe mpls_iptunnel
sudo modprobe mpls_gso

// Configure sysctl
sysctl -w net.mpls.platform_labels=65536
```
