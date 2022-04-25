namespace fge
{

///Tunnel
template <class T>
Tunnel<T>::Tunnel(fge::Tunnel<T>&& r) noexcept :
    g_gates(std::move(r.g_gates)),
    g_anonymousGates(std::move(r.g_anonymousGates))
{
    for ( std::size_t i=0; i<this->g_gates.size(); ++i )
    {
        this->g_gates[i]->g_tunnel = this;
    }
    for ( std::size_t i=0; i<this->g_anonymousGates.size(); ++i )
    {
        this->g_anonymousGates[i]->g_tunnel = this;
    }
}
template <class T>
Tunnel<T>::~Tunnel()
{
    this->closeAll();
}

template <class T>
fge::Tunnel<T>& Tunnel<T>::operator =(fge::Tunnel<T>&& r) noexcept
{
    this->g_gates = std::move(r.g_gates);
    this->g_anonymousGates = std::move(r.g_anonymousGates);

    for ( std::size_t i=0; i<this->g_gates.size(); ++i )
    {
        this->g_gates[i]->g_tunnel = this;
    }
    for ( std::size_t i=0; i<this->g_anonymousGates.size(); ++i )
    {
        this->g_anonymousGates[i]->g_tunnel = this;
    }
}

template <class T>
bool Tunnel<T>::knock(fge::TunnelGate<T>* gate, bool anonymous)
{
    if ( (!gate->isLocked()) || gate->status() )
    {
        return false;
    }

    gate->g_tunnel = this;
    if (anonymous)
    {
        this->g_anonymousGates.push_back(gate);
        return true;
    }
    this->g_gates.push_back(gate);
    return true;
}
template <class T>
bool Tunnel<T>::addGate(fge::TunnelGate<T>* gate, bool anonymous)
{
    if ( gate->isOpen() )
    {
        return false;
    }

    gate->g_tunnel = this;
    if (anonymous)
    {
        this->g_anonymousGates.push_back(gate);
        return true;
    }
    this->g_gates.push_back(gate);
    return true;
}

template <class T>
bool Tunnel<T>::isAnonymous(const fge::TunnelGate<T>* gate) const
{
    for ( std::size_t i=0; i<this->g_anonymousGates.size(); ++i )
    {
        if ( this->g_anonymousGates[i] == gate )
        {
            return true;
        }
    }
    return false;
}

template <class T>
void Tunnel<T>::closeGate(std::size_t index)
{
    if (index >= this->g_gates.size())
    {
        return;
    }
    this->g_gates[index]->g_tunnel = nullptr;
    this->g_gates.erase( this->g_gates.begin()+index );
}
template <class T>
void Tunnel<T>::closeAnonymousGate(std::size_t index)
{
    if (index >= this->g_anonymousGates.size())
    {
        return;
    }
    this->g_anonymousGates[index]->g_tunnel = nullptr;
    this->g_anonymousGates.erase( this->g_anonymousGates.begin()+index );
}
template <class T>
void Tunnel<T>::closeGate(fge::TunnelGate<T>* gate)
{
    for ( std::size_t i=0; i<this->g_anonymousGates.size(); ++i )
    {
        if (this->g_anonymousGates[i] == gate)
        {
            this->g_anonymousGates[i]->g_tunnel = nullptr;
            this->g_anonymousGates.erase( this->g_anonymousGates.begin()+i );
            return;
        }
    }
    for ( std::size_t i=0; i<this->g_gates.size(); ++i )
    {
        if (this->g_gates[i] == gate)
        {
            this->g_gates[i]->g_tunnel = nullptr;
            this->g_gates.erase( this->g_gates.begin()+i );
            return;
        }
    }
}
template <class T>
void Tunnel<T>::closeAll()
{
    for ( std::size_t i=0; i<this->g_anonymousGates.size(); ++i )
    {
        this->g_anonymousGates[i]->g_tunnel = nullptr;
    }
    for ( std::size_t i=0; i<this->g_gates.size(); ++i )
    {
        this->g_gates[i]->g_tunnel = nullptr;
    }
    this->g_anonymousGates.clear();
    this->g_gates.clear();
}

template <class T>
T* Tunnel<T>::get(std::size_t index) const
{
    if (index >= this->g_gates.size())
    {
        return nullptr;
    }
    return this->g_gates[index]->g_data;
}
template <class T>
T* Tunnel<T>::getAnonymous(std::size_t index) const
{
    if (index >= this->g_anonymousGates.size())
    {
        return nullptr;
    }
    return this->g_anonymousGates[index]->g_data;
}
template <class T>
std::size_t Tunnel<T>::getGatesSize() const
{
    return this->g_gates.size();
}
template <class T>
std::size_t Tunnel<T>::getAnonymousGatesSize() const
{
    return this->g_anonymousGates.size();
}

template <class T>
T* Tunnel<T>::operator[](std::size_t index) const
{
    if (index >= this->g_gates.size())
    {
        return nullptr;
    }
    return this->g_gates[index]->g_data;
}

///TunnelGate

template <class T>
TunnelGate<T>::TunnelGate() :
    g_data(nullptr),
    g_tunnel(nullptr),
    g_locked(false)
{
}
template <class T>
TunnelGate<T>::TunnelGate(const fge::TunnelGate<T>& gate) :
    g_data(gate.g_data),
    g_tunnel(nullptr),
    g_locked(gate.g_locked)
{
    if (gate.g_tunnel)
    {
        gate.g_tunnel->addGate(this, gate.g_tunnel->isAnonymous(&gate));
    }
}
template <class T>
TunnelGate<T>::TunnelGate(fge::TunnelGate<T>&& gate) noexcept :
    g_data(gate.g_data),
    g_tunnel(nullptr),
    g_locked(gate.g_locked)
{
    if (gate.g_tunnel)
    {
        gate.g_tunnel->addGate(this, gate.g_tunnel->isAnonymous(&gate));
        gate.g_tunnel->closeGate(&gate);
    }
}
template <class T>
TunnelGate<T>::TunnelGate(T* data) :
    g_data(data),
    g_tunnel(nullptr),
    g_locked(false)
{
}
template <class T>
TunnelGate<T>::~TunnelGate()
{
    this->close();
}

template <class T>
fge::TunnelGate<T>& TunnelGate<T>::operator=(const fge::TunnelGate<T>& gate)
{
    this->g_data = nullptr;
    this->g_tunnel = nullptr;
    this->g_locked = gate.g_locked;

    if (gate.g_tunnel)
    {
        gate.g_tunnel->addGate(this, gate.g_tunnel->isAnonymous(&gate));
    }
    return *this;
}
template <class T>
fge::TunnelGate<T>& TunnelGate<T>::operator=(fge::TunnelGate<T>&& gate) noexcept
{
    this->g_data = nullptr;
    this->g_tunnel = nullptr;
    this->g_locked = gate.g_locked;

    if (gate.g_tunnel)
    {
        gate.g_tunnel->addGate(this, gate.g_tunnel->isAnonymous(&gate));
        gate.g_tunnel->closeGate(&gate);
    }
    return *this;
}

template <class T>
bool TunnelGate<T>::openTo(fge::Tunnel<T>* tunnel, bool anonymous)
{
    return tunnel->addGate(this, anonymous);
}
template <class T>
void TunnelGate<T>::close()
{
    if ( this->g_tunnel )
    {
        this->g_tunnel->closeGate(this);
    }
}
template <class T>
bool TunnelGate<T>::isOpen() const
{
    return this->g_tunnel != nullptr;
}

template <class T>
void TunnelGate<T>::setData(T* val)
{
    this->g_data = val;
}
template <class T>
const T* TunnelGate<T>::getData() const
{
    return this->g_data;
}
template <class T>
T* TunnelGate<T>::getData()
{
    return this->g_data;
}

template <class T>
void TunnelGate<T>::setLock(bool val)
{
    this->g_locked = val;
}
template <class T>
bool TunnelGate<T>::isLocked() const
{
    return this->g_locked;
}

template <class T>
fge::Tunnel<T>* TunnelGate<T>::getTunnel() const
{
    return this->g_tunnel;
}

}//end fge
