#ifndef _FGE_C_TUNNEL_HPP_INCLUDED
#define _FGE_C_TUNNEL_HPP_INCLUDED

#include <vector>

namespace fge
{

template <class T>
class TunnelGate;

template <class T>
class Tunnel
{
public:
    Tunnel();
    Tunnel(fge::Tunnel<T>& r) = delete;
    Tunnel(fge::Tunnel<T>&& r);
    ~Tunnel();

    fge::Tunnel<T>& operator =(fge::Tunnel<T>& r) = delete;
    fge::Tunnel<T>& operator =(fge::Tunnel<T>&& r);

    bool knock(fge::TunnelGate<T>* gate, bool anonymous=false);
    bool addGate(fge::TunnelGate<T>* gate, bool anonymous=false);

    bool isAnonymous(const fge::TunnelGate<T>* gate) const;

    void closeGate(std::size_t index);
    void closeAnonymousGate(std::size_t index);
    void closeGate(fge::TunnelGate<T>* gate);
    void closeAll();

    T* get(std::size_t index) const;
    T* getAnonymous(std::size_t index) const;
    std::size_t getGatesSize() const;
    std::size_t getAnonymousGatesSize() const;

    T* operator[](std::size_t index) const;

private:
    std::vector<fge::TunnelGate<T>* > g_gates;
    std::vector<fge::TunnelGate<T>* > g_anonymousGates;
};

template <class T>
class TunnelGate
{
public:
    TunnelGate();
    TunnelGate(const fge::TunnelGate<T>& gate);
    TunnelGate(fge::TunnelGate<T>&& gate);
    TunnelGate(T* data);
    ~TunnelGate();

    fge::TunnelGate<T>& operator=(const fge::TunnelGate<T>& gate);
    fge::TunnelGate<T>& operator=(fge::TunnelGate<T>&& gate);

    bool openTo(fge::Tunnel<T>* tunnel, bool annonymous=false);
    void close();
    bool isOpen() const;

    void setData(T* val);
    const T* getData() const;
    T* getData();

    void setLock(bool val);
    bool isLocked() const;

    fge::Tunnel<T>* getTunnel() const;

private:
    friend class Tunnel<T>;

    T* g_data;
    fge::Tunnel<T>* g_tunnel;
    bool g_locked;
};

}//end fge

#include <FastEngine/C_tunnel.inl>

#endif // _FGE_C_TUNNEL_HPP_INCLUDED
