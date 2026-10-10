
#ifndef WIFI_MGT_HEADER_H
#define WIFI_MGT_HEADER_H

#include "non-inheritance.h"
#include "supported-rates.h"

#include "ns3/eht-capabilities.h"
#include "ns3/header.h"
#include "ns3/multi-link-element.h"

#include <algorithm>
#include <iterator>
#include <numeric>
#include <optional>
#include <utility>
#include <vector>

namespace ns3 {

namespace internal {

template <class T> struct GetStoredIe {
  typedef std::optional<T> type;
};

template <class T> struct GetStoredIe<std::optional<T>> {
  typedef std::optional<T> type;
};

template <class T> struct GetStoredIe<std::vector<T>> {
  typedef std::vector<T> type;
};

template <class T> using GetStoredIeT = typename GetStoredIe<T>::type;

} // namespace internal

template <typename Derived, typename Tuple> class WifiMgtHeader;

template <typename Derived, typename... Elems>
class WifiMgtHeader<Derived, std::tuple<Elems...>> : public Header {
public:
  template <typename T,
            std::enable_if_t<(std::is_same_v<std::vector<T>, Elems> + ...) == 0,
                             int> = 0>
  std::optional<T> &Get();

  template <typename T,
            std::enable_if_t<(std::is_same_v<std::vector<T>, Elems> + ...) == 0,
                             int> = 0>
  const std::optional<T> &Get() const;

  template <typename T,
            std::enable_if_t<(std::is_same_v<std::vector<T>, Elems> + ...) == 1,
                             int> = 0>
  std::vector<T> &Get();

  template <typename T,
            std::enable_if_t<(std::is_same_v<std::vector<T>, Elems> + ...) == 1,
                             int> = 0>
  const std::vector<T> &Get() const;

  void Print(std::ostream &os) const final;
  uint32_t GetSerializedSize() const final;
  void Serialize(Buffer::Iterator start) const final;
  uint32_t Deserialize(Buffer::Iterator start) final;

protected:
  template <typename IE>
  void InitForDeserialization(std::optional<IE> &optElem);

  void InitForDeserialization(std::optional<EhtCapabilities> &optElem);

  void PrintImpl(std::ostream &os) const;
  uint32_t GetSerializedSizeImpl() const;
  void SerializeImpl(Buffer::Iterator start) const;
  uint32_t DeserializeImpl(Buffer::Iterator start);

  template <typename T>
  Buffer::Iterator DoDeserialize(std::optional<T> &elem,
                                 Buffer::Iterator start);

  template <typename T>
  Buffer::Iterator DoDeserialize(std::vector<T> &elems, Buffer::Iterator start);

  using Elements = std::tuple<internal::GetStoredIeT<Elems>...>;

  Elements m_elements;
};

template <class T> struct CanBeInPerStaProfile : std::true_type {};

template <class T>
inline constexpr bool CanBeInPerStaProfileV = CanBeInPerStaProfile<T>::value;

template <typename Derived, typename Tuple> class MgtHeaderInPerStaProfile;

template <typename Derived, typename... Elems>
class MgtHeaderInPerStaProfile<Derived, std::tuple<Elems...>>
    : public WifiMgtHeader<Derived, std::tuple<Elems...>> {
public:
  uint32_t GetSerializedSizeInPerStaProfile(const Derived &frame) const;

  void SerializeInPerStaProfile(Buffer::Iterator start,
                                const Derived &frame) const;

  uint32_t DeserializeFromPerStaProfile(Buffer::Iterator start, uint16_t length,
                                        const Derived &frame);

  void CopyIesFromContainingFrame(const Derived &frame);

protected:
  using WifiMgtHeader<Derived, std::tuple<Elems...>>::InitForDeserialization;

  uint32_t GetSerializedSizeInPerStaProfileImpl(const Derived &frame) const;

  void SerializeInPerStaProfileImpl(Buffer::Iterator start,
                                    const Derived &frame) const;

  uint32_t DeserializeFromPerStaProfileImpl(Buffer::Iterator start,
                                            uint16_t length,
                                            const Derived &frame);

  void SetMleContainingFrame() const;

  void InitForDeserialization(std::optional<MultiLinkElement> &optElem);

private:
  using WifiMgtHeader<Derived, std::tuple<Elems...>>::DoDeserialize;
  using WifiMgtHeader<Derived, std::tuple<Elems...>>::m_elements;

  std::optional<NonInheritance> m_nonInheritance;
};

template <typename Derived, typename... Elems>
template <
    typename T,
    std::enable_if_t<(std::is_same_v<std::vector<T>, Elems> + ...) == 0, int>>
std::optional<T> &WifiMgtHeader<Derived, std::tuple<Elems...>>::Get() {
  return std::get<std::optional<T>>(m_elements);
}

template <typename Derived, typename... Elems>
template <
    typename T,
    std::enable_if_t<(std::is_same_v<std::vector<T>, Elems> + ...) == 0, int>>
const std::optional<T> &
WifiMgtHeader<Derived, std::tuple<Elems...>>::Get() const {
  return std::get<std::optional<T>>(m_elements);
}

template <typename Derived, typename... Elems>
template <
    typename T,
    std::enable_if_t<(std::is_same_v<std::vector<T>, Elems> + ...) == 1, int>>
std::vector<T> &WifiMgtHeader<Derived, std::tuple<Elems...>>::Get() {
  return std::get<std::vector<T>>(m_elements);
}

template <typename Derived, typename... Elems>
template <
    typename T,
    std::enable_if_t<(std::is_same_v<std::vector<T>, Elems> + ...) == 1, int>>
const std::vector<T> &
WifiMgtHeader<Derived, std::tuple<Elems...>>::Get() const {
  return std::get<std::vector<T>>(m_elements);
}

template <typename Derived, typename... Elems>
template <typename IE>
void WifiMgtHeader<Derived, std::tuple<Elems...>>::InitForDeserialization(
    std::optional<IE> &optElem) {
  optElem.emplace();
}

template <typename Derived, typename... Elems>
void WifiMgtHeader<Derived, std::tuple<Elems...>>::InitForDeserialization(
    std::optional<EhtCapabilities> &optElem) {
  NS_ASSERT(Get<SupportedRates>());
  auto rates = AllSupportedRates{*Get<SupportedRates>(), std::nullopt};
  const bool is2_4Ghz = rates.IsSupportedRate(1000000);
  auto &heCapabilities = Get<HeCapabilities>();
  if (heCapabilities) {
    optElem.emplace(is2_4Ghz, heCapabilities.value());
  } else {
    optElem.emplace();
  }
}

template <typename Derived, typename... Elems>
void MgtHeaderInPerStaProfile<Derived, std::tuple<Elems...>>::
    InitForDeserialization(std::optional<MultiLinkElement> &optElem) {
  optElem.emplace(*static_cast<const Derived *>(this));
}

namespace internal {

template <typename T>
uint16_t DoGetSerializedSize(const std::optional<T> &elem) {
  return elem.has_value() ? elem->GetSerializedSize() : 0;
}

template <typename T>
uint16_t DoGetSerializedSize(const std::vector<T> &elems) {
  return std::accumulate(
      elems.cbegin(), elems.cend(), 0,
      [](uint16_t a, const auto &b) { return b.GetSerializedSize() + a; });
}

} // namespace internal

template <typename Derived, typename... Elems>
uint32_t
WifiMgtHeader<Derived, std::tuple<Elems...>>::GetSerializedSize() const {
  return static_cast<const Derived *>(this)->GetSerializedSizeImpl();
}

template <typename Derived, typename... Elems>
uint32_t
WifiMgtHeader<Derived, std::tuple<Elems...>>::GetSerializedSizeImpl() const {
  return std::apply(
      [&](auto &...elems) {
        return (internal::DoGetSerializedSize(elems) + ...);
      },
      m_elements);
}

namespace internal {

template <typename T>
Buffer::Iterator DoSerialize(const std::optional<T> &elem,
                             Buffer::Iterator start) {
  return elem.has_value() ? elem->Serialize(start) : start;
}

template <typename T>
Buffer::Iterator DoSerialize(const std::vector<T> &elems,
                             Buffer::Iterator start) {
  return std::accumulate(
      elems.cbegin(), elems.cend(), start,
      [](Buffer::Iterator i, const auto &a) { return a.Serialize(i); });
}

} // namespace internal

template <typename Derived, typename... Elems>
void WifiMgtHeader<Derived, std::tuple<Elems...>>::Serialize(
    Buffer::Iterator start) const {
  static_cast<const Derived *>(this)->SerializeImpl(start);
}

template <typename Derived, typename... Elems>
void WifiMgtHeader<Derived, std::tuple<Elems...>>::SerializeImpl(
    Buffer::Iterator start) const {
  auto i = start;
  std::apply(
      [&](auto &...elems) { ((i = internal::DoSerialize(elems, i)), ...); },
      m_elements);
}

template <typename Derived, typename... Elems>
template <typename T>
Buffer::Iterator WifiMgtHeader<Derived, std::tuple<Elems...>>::DoDeserialize(
    std::optional<T> &elem, Buffer::Iterator start) {
  auto i = start;
  static_cast<Derived *>(this)->InitForDeserialization(elem);
  i = elem->DeserializeIfPresent(i);
  if (i.GetDistanceFrom(start) == 0) {
    elem.reset();
  }
  return i;
}

template <typename Derived, typename... Elems>
template <typename T>
Buffer::Iterator WifiMgtHeader<Derived, std::tuple<Elems...>>::DoDeserialize(
    std::vector<T> &elems, Buffer::Iterator start) {
  auto i = start;
  do {
    auto tmp = i;
    std::optional<T> item;
    static_cast<Derived *>(this)->InitForDeserialization(item);
    i = item->DeserializeIfPresent(i);
    if (i.GetDistanceFrom(tmp) == 0) {
      break;
    }
    elems.push_back(std::move(*item));
  } while (true);
  return i;
}

template <typename Derived, typename... Elems>
uint32_t WifiMgtHeader<Derived, std::tuple<Elems...>>::Deserialize(
    Buffer::Iterator start) {
  return static_cast<Derived *>(this)->DeserializeImpl(start);
}

template <typename Derived, typename... Elems>
uint32_t WifiMgtHeader<Derived, std::tuple<Elems...>>::DeserializeImpl(
    Buffer::Iterator start) {
  auto i = start;

  std::apply(
      [&](internal::GetStoredIeT<Elems> &...elems) {
        (
            [&] {
              if constexpr (std::is_same_v<
                                std::remove_reference_t<decltype(elems)>,
                                Elems>) {
                i = DoDeserialize(elems, i);
              } else {
                static_cast<Derived *>(this)->InitForDeserialization(elems);
                i = elems->Deserialize(i);
              }
            }(),
            ...);
      },
      m_elements);

  return i.GetDistanceFrom(start);
}

namespace internal {

template <typename T>
void DoPrint(const std::optional<T> &elem, std::ostream &os) {
  if (elem.has_value()) {
    os << *elem << " , ";
  }
}

template <typename T>
void DoPrint(const std::vector<T> &elems, std::ostream &os) {
  std::copy(elems.cbegin(), elems.cend(), std::ostream_iterator<T>(os, " , "));
}

} // namespace internal

template <typename Derived, typename... Elems>
void WifiMgtHeader<Derived, std::tuple<Elems...>>::Print(
    std::ostream &os) const {
  static_cast<const Derived *>(this)->PrintImpl(os);
}

template <typename Derived, typename... Elems>
void WifiMgtHeader<Derived, std::tuple<Elems...>>::PrintImpl(
    std::ostream &os) const {
  std::apply([&](auto &...elems) { ((internal::DoPrint(elems, os)), ...); },
             m_elements);
}

namespace internal {

template <typename T, typename Derived>
bool MustBeSerializedInPerStaProfile(const std::optional<T> &elem,
                                     const Derived &frame) {
  if (!CanBeInPerStaProfileV<T>) {
    return false;
  }

  if (auto &outsideIe = frame.template Get<T>();
      outsideIe.has_value() && elem.has_value() &&
      !(outsideIe.value() == elem.value())) {
    return true;
  }

  if (!frame.template Get<T>().has_value() && elem.has_value()) {
    return true;
  }

  return false;
}

template <typename T, typename Derived>
bool MustBeSerializedInPerStaProfile(const std::vector<T> &elems,
                                     const Derived &frame) {
  if (!CanBeInPerStaProfileV<T>) {
    return false;
  }

  if (auto &outsideIe = frame.template Get<T>();
      !outsideIe.empty() && !elems.empty() && !(outsideIe == elems)) {
    return true;
  }

  if (frame.template Get<T>().empty() && !elems.empty()) {
    return true;
  }

  return false;
}

template <typename T, typename Derived>
std::optional<std::pair<uint8_t, uint8_t>>
MustBeListedInNonInheritance(const std::optional<T> &elem,
                             const Derived &frame) {
  if (auto &outsideIe = frame.template Get<T>();
      CanBeInPerStaProfileV<T> && outsideIe.has_value() && !elem.has_value()) {
    return {{outsideIe->ElementId(), outsideIe->ElementIdExt()}};
  }
  return std::nullopt;
}

template <typename T, typename Derived>
std::optional<std::pair<uint8_t, uint8_t>>
MustBeListedInNonInheritance(const std::vector<T> &elems,
                             const Derived &frame) {
  if (auto &outsideIe = frame.template Get<T>();
      CanBeInPerStaProfileV<T> && !outsideIe.empty() && elems.empty()) {
    return {{outsideIe.front().ElementId(), outsideIe.front().ElementIdExt()}};
  }
  return std::nullopt;
}

} // namespace internal

template <typename Derived, typename... Elems>
uint32_t MgtHeaderInPerStaProfile<Derived, std::tuple<Elems...>>::
    GetSerializedSizeInPerStaProfile(const Derived &frame) const {
  return static_cast<const Derived *>(this)
      ->GetSerializedSizeInPerStaProfileImpl(frame);
}

template <typename Derived, typename... Elems>
uint32_t MgtHeaderInPerStaProfile<Derived, std::tuple<Elems...>>::
    GetSerializedSizeInPerStaProfileImpl(const Derived &frame) const {
  uint32_t size = 0;
  std::optional<NonInheritance> nonInheritance;

  std::apply(
      [&](auto &...elems) {
        (
            [&] {
              if (internal::MustBeSerializedInPerStaProfile(elems, frame)) {
                size += internal::DoGetSerializedSize(elems);
              } else if (auto idPair = internal::MustBeListedInNonInheritance(
                             elems, frame)) {
                if (!nonInheritance) {
                  nonInheritance.emplace();
                }
                nonInheritance->Add(idPair->first, idPair->second);
              }
            }(),
            ...);
      },
      m_elements);

  if (nonInheritance) {
    size += nonInheritance->GetSerializedSize();
  }
  return size;
}

template <typename Derived, typename... Elems>
void MgtHeaderInPerStaProfile<Derived, std::tuple<Elems...>>::
    SerializeInPerStaProfile(Buffer::Iterator start,
                             const Derived &frame) const {
  static_cast<const Derived *>(this)->SerializeInPerStaProfileImpl(start,
                                                                   frame);
}

template <typename Derived, typename... Elems>
void MgtHeaderInPerStaProfile<Derived, std::tuple<Elems...>>::
    SerializeInPerStaProfileImpl(Buffer::Iterator start,
                                 const Derived &frame) const {
  auto i = start;
  std::optional<NonInheritance> nonInheritance;

  std::apply(
      [&](auto &...elems) {
        (
            [&] {
              if (internal::MustBeSerializedInPerStaProfile(elems, frame)) {
                i = internal::DoSerialize(elems, i);
              } else if (auto idPair = internal::MustBeListedInNonInheritance(
                             elems, frame)) {
                if (!nonInheritance) {
                  nonInheritance.emplace();
                }
                nonInheritance->Add(idPair->first, idPair->second);
              }
            }(),
            ...);
      },
      m_elements);

  if (nonInheritance) {
    nonInheritance->Serialize(i);
  }
}

namespace internal {

template <typename T, typename Derived>
void DoCopyIeFromContainingFrame(std::optional<T> &elem, const Derived &frame) {
  if (auto &outsideIe = frame.template Get<T>();
      CanBeInPerStaProfileV<T> && outsideIe.has_value() && !elem.has_value()) {
    elem = outsideIe.value();
  }
}

template <typename T, typename Derived>
void DoCopyIeFromContainingFrame(std::vector<T> &elems, const Derived &frame) {
  if (auto &outsideIe = frame.template Get<T>();
      CanBeInPerStaProfileV<T> && !outsideIe.empty() && elems.empty()) {
    elems = outsideIe;
  }
}

} // namespace internal

template <typename Derived, typename... Elems>
uint32_t MgtHeaderInPerStaProfile<Derived, std::tuple<Elems...>>::
    DeserializeFromPerStaProfile(Buffer::Iterator start, uint16_t length,
                                 const Derived &frame) {
  return static_cast<Derived *>(this)->DeserializeFromPerStaProfileImpl(
      start, length, frame);
}

template <typename Derived, typename... Elems>
uint32_t MgtHeaderInPerStaProfile<Derived, std::tuple<Elems...>>::
    DeserializeFromPerStaProfileImpl(Buffer::Iterator start, uint16_t length,
                                     const Derived &frame) {
  auto i = start;

  std::apply(
      [&](auto &...elems) {
        (
            [&] {
              if (i.GetDistanceFrom(start) < length) {
                i = static_cast<Derived *>(this)->DoDeserialize(elems, i);
                internal::DoCopyIeFromContainingFrame(elems, frame);
              }
            }(),
            ...);
      },
      m_elements);

  m_nonInheritance.reset();
  i = DoDeserialize(m_nonInheritance, i);

  auto distance = i.GetDistanceFrom(start);
  NS_ASSERT_MSG(distance == length,
                "Bytes read (" << distance << ") not matching expected number ("
                               << length << ")");
  return distance;
}

namespace internal {

template <typename T>
void RemoveIfNotInherited(std::optional<T> &elem,
                          const NonInheritance &nonInheritance) {
  if (elem.has_value() &&
      nonInheritance.IsPresent(elem->ElementId(), elem->ElementIdExt())) {
    elem.reset();
  }
}

template <typename T>
void RemoveIfNotInherited(std::vector<T> &elem,
                          const NonInheritance &nonInheritance) {
  if (!elem.empty() && nonInheritance.IsPresent(elem.front().ElementId(),
                                                elem.front().ElementIdExt())) {
    elem.clear();
  }
}

} // namespace internal

template <typename Derived, typename... Elems>
void MgtHeaderInPerStaProfile<Derived, std::tuple<Elems...>>::
    CopyIesFromContainingFrame(const Derived &frame) {
  std::apply(
      [&](auto &...elems) {
        ((internal::DoCopyIeFromContainingFrame(elems, frame)), ...);
      },
      m_elements);

  if (m_nonInheritance) {
    std::apply(
        [&](auto &...elems) {
          ((internal::RemoveIfNotInherited(elems, *m_nonInheritance)), ...);
        },
        m_elements);
  }
}

template <typename Derived, typename... Elems>
void MgtHeaderInPerStaProfile<
    Derived, std::tuple<Elems...>>::SetMleContainingFrame() const {
  if (auto &mle = WifiMgtHeader<Derived, std::tuple<Elems...>>::template Get<
          MultiLinkElement>()) {
    mle->m_containingFrame = *static_cast<const Derived *>(this);
  }
}

} // namespace ns3

#endif
