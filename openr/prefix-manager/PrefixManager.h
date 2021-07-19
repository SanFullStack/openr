/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/IPAddress.h>
#include <folly/Optional.h>
#include <folly/futures/Future.h>

#include <openr/common/AsyncThrottle.h>
#include <openr/common/OpenrEventBase.h>
#include <openr/common/Types.h>
#include <openr/common/Util.h>
#include <openr/config-store/PersistentStore.h>
#include <openr/config/Config.h>
#include <openr/decision/RouteUpdate.h>
#include <openr/if/gen-cpp2/Network_types.h>
#include <openr/if/gen-cpp2/Types_types.h>
#include <openr/kvstore/KvStoreClientInternal.h>
#include <openr/messaging/Queue.h>
#include <openr/policy/PolicyManager.h>

namespace openr {

namespace detail {

class PrefixManagerPendingUpdates {
 public:
  explicit PrefixManagerPendingUpdates() : changedPrefixes_{} {}

  std::unordered_set<folly::CIDRNetwork> const&
  getChangedPrefixes() const {
    return changedPrefixes_;
  }

  void reset();

  void applyPrefixChange(const folly::small_vector<folly::CIDRNetwork>& change);

  void
  removePrefixChange(const folly::CIDRNetwork& prefix) {
    changedPrefixes_.erase(prefix);
  }

 private:
  // track prefixes that have changed within this batch
  // ATTN: this collection contains:
  //  - newly added prefixes
  //  - updated prefixes
  //  - prefixes being withdrawn
  std::unordered_set<folly::CIDRNetwork> changedPrefixes_{};
};

} // namespace detail

class PrefixManager final : public OpenrEventBase {
 public:
  PrefixManager(
      // producer queue
      messaging::ReplicateQueue<DecisionRouteUpdate>& staticRouteUpdatesQueue,
      messaging::ReplicateQueue<KeyValueRequest>& kvRequestQueue,
      // consumer queue
      messaging::RQueue<PrefixEvent> prefixUpdatesQueue,
      messaging::RQueue<DecisionRouteUpdate> fibRouteUpdatesQueue,
      // config
      std::shared_ptr<const Config> config,
      // raw ptr for modules
      KvStore* kvStore);

  ~PrefixManager();

  /**
   * Override stop method of OpenrEventBase
   */
  void stop() override;

  // disable copying
  PrefixManager(PrefixManager const&) = delete;
  PrefixManager& operator=(PrefixManager const&) = delete;

  /*
   * Public API for PrefixManager operations:
   *
   * Write APIs - will schedule syncKvStoreThrottled_ to update kvstore,
   * @return true if there are changes else false
   *  - add prefixes
   *  - withdraw prefixes
   *  - withdraw prefixes by type
   *  - sync prefixes by type: replace all prefixes of @type w/ @prefixes
   *
   *
   * Read APIs - dump internal prefixDb
   *  - dump all prefixes
   *  - dump all prefixes by type
   */
  folly::SemiFuture<bool> advertisePrefixes(
      std::vector<thrift::PrefixEntry> prefixes);

  folly::SemiFuture<bool> withdrawPrefixes(
      std::vector<thrift::PrefixEntry> prefixes);

  folly::SemiFuture<bool> withdrawPrefixesByType(thrift::PrefixType prefixType);

  folly::SemiFuture<bool> syncPrefixesByType(
      thrift::PrefixType prefixType, std::vector<thrift::PrefixEntry> prefixes);

  folly::SemiFuture<std::unique_ptr<std::vector<thrift::PrefixEntry>>>
  getPrefixes();

  folly::SemiFuture<std::unique_ptr<std::vector<thrift::PrefixEntry>>>
  getPrefixesByType(thrift::PrefixType prefixType);

  folly::SemiFuture<std::unique_ptr<std::vector<thrift::AdvertisedRouteDetail>>>
  getAdvertisedRoutesFiltered(thrift::AdvertisedRouteFilter filter);

  folly::SemiFuture<std::unique_ptr<std::vector<thrift::OriginatedPrefixEntry>>>
  getOriginatedPrefixes();

  /**
   * Helper functinon used in getAreaAdvertisedRoutes()
   * Filter routes with 1. <type> attribute
   */
  void filterAndAddAreaRoute(
      std::vector<thrift::AdvertisedRoute>& routes,
      const std::string& area,
      const thrift::RouteFilterType& routeFilterType,
      std::unordered_map<thrift::PrefixType, PrefixEntry> const& prefixEntries,
      apache::thrift::optional_field_ref<thrift::PrefixType&> const&
          typeFilter);
  /**
   * Util function to convert thrift::OriginatedPrefix to thrift::PrefixEntry
   * @param prefix: the OriginatedPrefix to be converted
   * @param tType: type of this prefix, can be CONFIG or VIP
   */
  thrift::PrefixEntry toPrefixEntryThrift(
      const thrift::OriginatedPrefix& prefix, const thrift::PrefixType& tType);

  /*
   * Dump post filter (policy) routes for given area.
   *
   * @param rejected
   * - true: return a list of accepted post policy routes
   * - false: return a list of rejected routes
   */
  folly::SemiFuture<std::unique_ptr<std::vector<thrift::AdvertisedRoute>>>
  getAreaAdvertisedRoutes(
      std::string areaName,
      thrift::RouteFilterType routeFilterType,
      thrift::AdvertisedRouteFilter filter);

  /**
   * Helper functinon used in getAdvertisedRoutesFiltered()
   * Filter routes with 1. <type> attribute
   */
  static void filterAndAddAdvertisedRoute(
      std::vector<thrift::AdvertisedRouteDetail>& routes,
      apache::thrift::optional_field_ref<thrift::PrefixType&> const& typeFilter,
      folly::CIDRNetwork const& prefix,
      std::unordered_map<thrift::PrefixType, PrefixEntry> const& prefixEntries);

 private:
  /*
   * Private helpers to update `prefixMap_`
   *
   * Called upon:
   * - public write APIs
   * - request from replicate queue
   *
   * modify prefix db and schedule syncKvStoreThrottled_ to update kvstore
   * @return true if the db is modified
   */
  bool advertisePrefixesImpl(
      std::vector<thrift::PrefixEntry>&& tPrefixEntries,
      const std::unordered_set<std::string>& dstAreas);
  bool advertisePrefixesImpl(
      std::vector<PrefixEntry>&& tPrefixEntries,
      const std::unordered_set<std::string>& dstAreas);
  bool advertisePrefixesImpl(const std::vector<PrefixEntry>& prefixEntries);
  bool withdrawPrefixesImpl(
      const std::vector<thrift::PrefixEntry>& tPrefixEntries);
  bool withdrawPrefixEntriesImpl(const std::vector<PrefixEntry>& prefixEntries);
  bool withdrawPrefixesByTypeImpl(thrift::PrefixType type);
  bool syncPrefixesByTypeImpl(
      thrift::PrefixType type,
      const std::vector<thrift::PrefixEntry>& tPrefixEntries,
      const std::unordered_set<std::string>& dstAreas);

  /*
   * One prefixEntry is ready to be advertised iff
   * - If prependLabel is set, the associated label route should have been
   *   programmed locally.
   * - [TODO] The associated unicast route should have been programmed locally.
   */
  bool prefixEntryReadyToBeAdvertised(const PrefixEntry& prefixEntry);

  /*
   * Util function to interact with KvStore to advertise/withdraw prefixes
   * ATTN: syncKvStore() has throttled version `syncKvStoreThrottled_` to
   *       batch processing updates
   */
  void syncKvStore();

  std::unordered_set<std::string> updateKvStoreKeyHelper(
      const PrefixEntry& entry);

  void deleteKvStoreKeyHelper(
      const std::unordered_set<std::string>& deletedKeys);

  /*
   * [Route Origination/Aggregation]
   *
   * Methods implement utility function for route origination/aggregation.
   */

  /*
   * Read routes from OpenrConfig and stored in `OriginatedPrefixDb_`
   * for potential advertisement/withdrawn based on min_supporting_route
   * ref-count.
   */
  void buildOriginatedPrefixDb(
      const std::vector<thrift::OriginatedPrefix>& prefixes);

  /*
   * Util function to process programmed route update from Fib and populate
   * aggregates to advertise.
   */
  void aggregatesToAdvertise(const folly::CIDRNetwork& prefix);

  /*
   * Util function to process programmed route update from Fib and populate
   * aggregates to withdraw.
   */
  void aggregatesToWithdraw(const folly::CIDRNetwork& prefix);

  /*
   * Util function to iterate through originatedPrefixDb_ for route
   * advertisement/withdrawn
   */
  void processOriginatedPrefixes();

  // Process Fib route update.
  void processFibRouteUpdates(DecisionRouteUpdate&& fibRouteUpdate);

  // Store programmed routes update from FIB, which are later used to check
  // whether prefixes are ready to be injected into KvStore.
  void storeProgrammedRoutes(const DecisionRouteUpdate& fibRouteUpdates);

  // For one node locating in multiple areas, it should redistribute prefixes
  // received from one area into other areas, performing similar role as border
  // routers in BGP.
  void redistributePrefixesAcrossAreas(DecisionRouteUpdate& fibRouteUpdate);

  // get all areaIds
  std::unordered_set<std::string> allAreaIds();

  /*
   * Private variables/Structures
   */

  // this node name
  const std::string nodeId_;

  // map from area id to area policy
  std::unordered_map<std::string, std::optional<std::string>> areaToPolicy_;

  // TTL for a key in the key value store
  const std::chrono::milliseconds ttlKeyInKvStore_{0};

  // queue to publish originated route updates to decision
  messaging::ReplicateQueue<DecisionRouteUpdate>& staticRouteUpdatesQueue_;

  // queue to send key-value update requests to KvStore
  messaging::ReplicateQueue<KeyValueRequest>& kvRequestQueue_;

  // V4 prefix over V6 nexthop enabled
  const bool v4OverV6Nexthop_{false};

  // module ptr to interact with KvStore
  KvStore* kvStore_{nullptr};

  // Throttled version of syncKvStore. It batches up multiple calls and
  // send them in one go!
  std::unique_ptr<AsyncThrottle> syncKvStoreThrottled_;
  std::unique_ptr<folly::AsyncTimeout> initialSyncKvStoreTimer_;

  // kvStoreClient for persisting our prefix db
  std::unique_ptr<KvStoreClientInternal> kvStoreClient_{nullptr};

  // The current prefix db this node is advertising. In-case if multiple entries
  // exists for a given prefix, best-route-selection process would select the
  // ones with the best metric. Lowest prefix-type is used as a tie-breaker for
  // advertising the best selected routes to KvStore.
  std::unordered_map<
      folly::CIDRNetwork,
      std::unordered_map<thrift::PrefixType, PrefixEntry>>
      prefixMap_;
  // The collection of prefixes advertised to KvStore.
  std::unordered_set<folly::CIDRNetwork> advertisedPrefixes_;

  // the serializer/deserializer helper we'll be using
  apache::thrift::CompactSerializer serializer_;

  // This collection serves for easy clean up when certain prefix is withdrawn.
  // `PrefixManager` will advertise prefixes based on best-route-selection
  // process. With multi-area support, one prefixes will map to multiple
  // key-advertisement in `KvStore`.
  struct AdervertiseStatus {
    bool installedToFib{false};
    std::unordered_set<std::string> keys;
  };
  std::unordered_map<folly::CIDRNetwork, AdervertiseStatus> keysInKvStore_{};
  // store pending updates from advertise/withdraw operation
  detail::PrefixManagerPendingUpdates pendingUpdates_;

  std::unique_ptr<PolicyManager> policyManager_{nullptr};

  /*
   * [Route Origination/Aggregation]
   *
   * Local-originated prefixes will be advertise/withdrawn from
   * `Prefix-Manager` by calculating ref-count of supporting-
   * route from `Decision`.
   *  --------               ---------
   *           ------------>
   *  Decision               PrefixMgr
   *           <------------
   *  --------               ---------
   */

  // struct to represent local-originiated route
  struct OriginatedRoute {
    // thrift structure read from config
    thrift::OriginatedPrefix originatedPrefix;

    // unicastEntry contains nexthops info
    RibUnicastEntry unicastEntry;

    // supporting routes for this originated prefix
    std::unordered_set<folly::CIDRNetwork> supportingRoutes{};

    // flag indicates is local route has been originated
    bool isAdvertised{false};

    OriginatedRoute(
        const thrift::OriginatedPrefix& originatedPrefix,
        const RibUnicastEntry& unicastEntry,
        const std::unordered_set<folly::CIDRNetwork>& supportingRoutes)
        : originatedPrefix(originatedPrefix),
          unicastEntry(unicastEntry),
          supportingRoutes(supportingRoutes) {}

    /*
     * Util function for route-agg check
     */
    bool
    shouldInstall() const {
      return originatedPrefix.install_to_fib_ref().value_or(false);
    }

    bool
    shouldAdvertise() const {
      const auto& minSupportingRouteCnt =
          *originatedPrefix.minimum_supporting_routes_ref();
      return (not isAdvertised) and
          (supportingRoutes.size() >= minSupportingRouteCnt);
    }

    bool
    shouldWithdraw() const {
      const auto& minSupportingRouteCnt =
          *originatedPrefix.minimum_supporting_routes_ref();
      return isAdvertised and (supportingRoutes.size() < minSupportingRouteCnt);
    }
  };

  /*
   * prefer local originated prefix over BGP originated prefix
   * Turned on/off via thrift::OpenrConfig::prefer_openr_originated_config
   */
  bool preferOpenrOriginatedRoutes_{false};

  /*
   * TODO: temp flag to indicate usage of new prefix key format
   */
  bool enableNewPrefixFormat_{false};

  /*
   * prefixes to be originated from prefix-manager
   * ATTN: to support quick information retrieval, cache the mapping:
   *
   *  OriginatedPrefix -> set of FIB prefixEntry(i.e. supporting routes)
   */
  std::unordered_map<folly::CIDRNetwork, OriginatedRoute> originatedPrefixDb_;

  /*
   * prefixes received from OpenR/Fib.
   * ATTN: to avoid loop through ALL entries inside `originatedPrefixes`,
   *       cache the reverse mapping:
   *
   *  FIB prefixEntry -> vector of OriginatedPrefix(i.e. supported routes)
   */
  std::unordered_map<folly::CIDRNetwork, std::vector<folly::CIDRNetwork>>
      ribPrefixDb_;

  /*
   * MPLS labels with label routes already programmed by FIB. For one prefix
   * with prepend label, PrefixManager needs to make sure the associated label
   * routes is programmed before advertisement.
   */
  std::unordered_set<int32_t> programmedLabels_;

  // Enable FIB-ACK to enforce route program ordering, aka, to-add routes should
  // be programmed at originator ahead of being advertised to peers.
  bool fibAckEnabled_{false};

  // Send update requests to KvStore via queue
  bool enableKvStoreRequestQueue_{false};
}; // PrefixManager

} // namespace openr
