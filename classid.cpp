#include <iostream>
#include <string>

#define BEGIN_REQUEST( NAME ) \
    struct NAME : public Base { \
        static Key staticKey( ) { return details::IDGenerator< NAME >::key( ); } \
        NAME( ) : Base( NAME::staticKey( ) ) { }

#define END_REQUEST };

using Key = void *;

namespace details {

template< typename T > struct IDGenerator {
    static Key key( ) {
        static IDGenerator< T > singleton;
        return &singleton;
    }
};

} // namespace details.

class Base {
public:
    Base( const Base & ) = delete;
    Base( Base && ) = delete;

    Key key( ) const noexcept { return m_key; }

protected:
    virtual ~Base( ) = default;
    Base( Key k ) : m_key( k ) { }

private:
    Key m_key;
};

BEGIN_REQUEST( Store )
    std::string mediaID;
    bool store;
END_REQUEST

BEGIN_REQUEST( Shutdown )
END_REQUEST

int main( ) {
    std::cout << "Store::staticKey: " << Store::staticKey( ) << '\n';
    std::cout << "Shutdown::staticKey: " << Shutdown::staticKey( ) << '\n';

    Store store;
    Shutdown shutdown;

    std::cout << "Store::key( ): " << store.key( ) << '\n';
    std::cout << "Shutdown::key( ): " << shutdown.key( ) << '\n';

    return 0;
}
