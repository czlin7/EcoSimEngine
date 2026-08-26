#pragma once

#include <string>
#include <tuple>

#include "EcoSimEngine/component/Components.hpp"
#include "EcoSimEngine/component/ComponentIndices.hpp"
#include "EcoSimEngine/component/ComponentType.hpp"

class EntityManager;

class Entity
{
  friend class EntityManager;

  bool m_active{true};
  const size_t m_id{0};
  const std::string m_tag{"default"};
  Signature m_signature{}; // bitset to track which components are present

  // constructor is private, so we can never create entities
  //   outside the EntityManager which had friend access
  Entity(size_t id, std::string tag)
      : m_id{id}, m_tag{std::move(tag)}
  {
  }

  void destroy() noexcept
  {
    m_active = false;
  }

public:
  [[nodiscard]] size_t id() const noexcept
  {
    return m_id;
  }

  [[nodiscard]] bool isActive() const noexcept
  {
    return m_active;
  }

  [[nodiscard]] const std::string &tag() const noexcept
  {
    return m_tag;
  }

  [[nodiscard]] const Signature &signature() const noexcept
  {
    return m_signature;
  }

  // mutable access for managers
  Signature &signature() noexcept { return m_signature; }
};
