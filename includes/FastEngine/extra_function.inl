namespace fge
{

///Utility
char UnicodeToChar(uint32_t unicode)
{
    return (unicode<128) ? static_cast<char>(unicode) : 0;
}

///Position/Rectangle
template<typename T>
sf::Rect<T> ToRect (const sf::Vector2<T>& pos1, const sf::Vector2<T>& pos2)
{
    return sf::Rect<T>( (pos1.x<pos2.x)?pos1.x:pos2.x, (pos1.y<pos2.y)?pos1.y:pos2.y, (pos1.x>pos2.x)?(pos1.x-pos2.x):(pos2.x-pos1.x), (pos1.y>pos2.y)?(pos1.y-pos2.y):(pos2.y-pos1.y) );
}
template<typename T>
sf::Rect<T> ToRect (const std::vector<sf::Vector2<T> >& pos)
{
    float smallestX = std::numeric_limits<float>::max();
    float biggestX = std::numeric_limits<float>::lowest();
    float smallestY = std::numeric_limits<float>::max();
    float biggestY = std::numeric_limits<float>::lowest();

    for (std::size_t i=0; i<pos.size(); ++i)
    {
        if ( pos[i].x < smallestX )
        {
            smallestX = pos[i].x;
        }
        if ( pos[i].x > biggestX )
        {
            biggestX = pos[i].x;
        }
        if ( pos[i].y < smallestY )
        {
            smallestY = pos[i].y;
        }
        if ( pos[i].y > biggestY )
        {
            biggestY = pos[i].y;
        }
    }

    return sf::Rect<T>(smallestX, smallestY, biggestX-smallestX, biggestY-smallestY);
}
template<typename T>
sf::Rect<T> ToRect(const sf::Vector2<T>* pos, std::size_t size)
{
    float smallestX = std::numeric_limits<float>::max();
    float biggestX = std::numeric_limits<float>::lowest();
    float smallestY = std::numeric_limits<float>::max();
    float biggestY = std::numeric_limits<float>::lowest();

    for (std::size_t i=0; i<size; ++i)
    {
        if ( pos[i].x < smallestX )
        {
            smallestX = pos[i].x;
        }
        if ( pos[i].x > biggestX )
        {
            biggestX = pos[i].x;
        }
        if ( pos[i].y < smallestY )
        {
            smallestY = pos[i].y;
        }
        if ( pos[i].y > biggestY )
        {
            biggestY = pos[i].y;
        }
    }

    return sf::Rect<T>(smallestX, smallestY, biggestX-smallestX, biggestY-smallestY);
}

///Reach
template<typename T>
T ReachValue( T value, T target, T speed, float deltaTime )
{
    float travelDistance = static_cast<float>(speed) * deltaTime;
    float direction = (static_cast<float>(target)-static_cast<float>(value)) / std::abs(static_cast<float>(target)-static_cast<float>(value));
    T actualDistance = std::abs(target-value);

    if (travelDistance >= actualDistance)
    {//We already reached the target
        return target;
    }
    return static_cast<T>(static_cast<float>(value) + direction*travelDistance);
}

///2D Math
template<typename T>
sf::Vector2f NormalizeVector2(const sf::Vector2<T>& vec)
{
    return vec / std::sqrt(vec.x*vec.x + vec.y*vec.y);
}

template<typename TIterator>
TIterator GetNearestVector(const sf::Vector2f& vec, const TIterator& pointsBegin, const TIterator& pointsEnd)
{
    TIterator bestNearestPoint = pointsEnd;
    float bestNearestDistance;

    for (TIterator it=pointsBegin; it!=pointsEnd; ++it)
    {
        float distance = fge::GetDistanceBetween(vec, *it);
        if (bestNearestPoint == pointsEnd)
        {
            bestNearestPoint = it;
            bestNearestDistance = distance;
        }
        else
        {
            if (distance < bestNearestDistance)
            {
                bestNearestPoint = it;
                bestNearestDistance = distance;
            }
        }
    }

    return bestNearestPoint;
}

///Color
sf::Color SetAlpha(const sf::Color& color, uint8_t alpha)
{
    sf::Color buff(color);
    buff.a = alpha;
    return buff;
}
sf::Color SetRed(const sf::Color& color, uint8_t red)
{
    sf::Color buff(color);
    buff.r = red;
    return buff;
}
sf::Color SetGreen(const sf::Color& color, uint8_t green)
{
    sf::Color buff(color);
    buff.g = green;
    return buff;
}
sf::Color SetBlue(const sf::Color& color, uint8_t blue)
{
    sf::Color buff(color);
    buff.b = blue;
    return buff;
}

sf::Color&& SetAlpha(sf::Color&& color, uint8_t alpha)
{
    color.a = alpha;
    return std::move(color);
}
sf::Color&& SetRed(sf::Color&& color, uint8_t red)
{
    color.r = red;
    return std::move(color);
}
sf::Color&& SetGreen(sf::Color&& color, uint8_t green)
{
    color.g = green;
    return std::move(color);
}
sf::Color&& SetBlue(sf::Color&& color, uint8_t blue)
{
    color.b = blue;
    return std::move(color);
}

}//end fge
