/*
 * Copyright 2026 Guillaume Guillet
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _FGE_C_TUNNEL_HPP_INCLUDED
#define _FGE_C_TUNNEL_HPP_INCLUDED

#include <vector>

namespace fge
{

template<class T>
class TunnelGate;

template<class T>
class Tunnel
{
public:
    Tunnel() = default;
    Tunnel(fge::Tunnel<T>& r) = delete;
    Tunnel(fge::Tunnel<T>&& r) noexcept;
    ~Tunnel();

    fge::Tunnel<T>& operator=(fge::Tunnel<T>& r) = delete;
    fge::Tunnel<T>& operator=(fge::Tunnel<T>&& r) noexcept;

    bool knock(fge::TunnelGate<T>& gate, bool anonymous = false);
    bool addGate(fge::TunnelGate<T>& gate, bool anonymous = false);

    bool isAnonymous(fge::TunnelGate<T> const& gate) const;

    void closeGate(std::size_t index);
    void closeAnonymousGate(std::size_t index);
    void closeGate(fge::TunnelGate<T>& gate);
    void closeAll();

    T* get(std::size_t index) const;
    T* getAnonymous(std::size_t index) const;
    [[nodiscard]] std::size_t getGatesSize() const;
    [[nodiscard]] std::size_t getAnonymousGatesSize() const;

    T* operator[](std::size_t index) const;

private:
    std::vector<fge::TunnelGate<T>*> g_gates;
    std::vector<fge::TunnelGate<T>*> g_anonymousGates;
};

template<class T>
class TunnelGate
{
public:
    TunnelGate();
    TunnelGate(fge::TunnelGate<T> const& gate);
    TunnelGate(fge::TunnelGate<T>&& gate) noexcept;
    explicit TunnelGate(T* data);
    ~TunnelGate();

    fge::TunnelGate<T>& operator=(fge::TunnelGate<T> const& gate);
    fge::TunnelGate<T>& operator=(fge::TunnelGate<T>&& gate) noexcept;

    bool openTo(fge::Tunnel<T>& tunnel, bool anonymous = false);
    void close();
    [[nodiscard]] bool isOpen() const;

    void setData(T* val);
    T const* getData() const;
    T* getData();

    void setLock(bool val);
    [[nodiscard]] bool isLocked() const;

    fge::Tunnel<T>* getTunnel() const;

private:
    friend class Tunnel<T>;

    T* g_data;
    fge::Tunnel<T>* g_tunnel;
    bool g_locked;
};

} // namespace fge

#include <FastEngine/C_tunnel.inl>

#endif // _FGE_C_TUNNEL_HPP_INCLUDED
