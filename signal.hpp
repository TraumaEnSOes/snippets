#include <functional>

template< typename... ARGS > class Signal {
public:
    Signal( ) = default;
    Signal( const Signal< ARGS... > & ) = delete;
    Signal &operator=( const Signal< ARGS... > & ) = delete;

    operator bool( ) const noexcept { return m_callback.operator bool( ); }
    bool operator!( ) const noexcept { return !operator bool( ); }

    template< typename F > void connect( F &&fn ) {
        m_callback = std::move( fn );
    }
    template< typename RET, typename CLASS > void connect( CLASS *instance, RET (CLASS::*fn)( ARGS... ) ) {
        m_callback = [instance, fn]( ARGS... args ) -> void { (instance->*fn)( std::forward< ARGS >( args )... ); };
    }
    void connect( Signal< ARGS... > &other ) {
        m_callback = [&other]( ARGS... args ) -> void { other.emit( std::forward< ARGS >( args )... ); };
    }
    void connect( std::nullptr_t ) { m_callback = nullptr; }

    void emit( ARGS... args ) {
        if( m_callback ) {
            m_callback( std::forward< ARGS >( args )... );
        }
    }

private:
    std::function< void( ARGS... ) > m_callback;
};

template< > class Signal< void > {
public:
    Signal( ) = default;
    Signal( const Signal< > & ) = delete;
    Signal &operator=( const Signal< > & ) = delete;

    operator bool( ) const noexcept { return m_callback.operator bool( ); }
    bool operator!( ) const noexcept { return !operator bool( ); }

    template< typename F > void connect( F &&fn ) {
        m_callback = std::move( fn );
    }
    template< typename RET, typename CLASS > void connect( CLASS *instance, RET (CLASS::*fn)( ) ) {
        m_callback = [instance, fn]( ) -> void { (instance->*fn)( ); };
    }
    void connect( Signal< > &other ) {
        m_callback = [&other]( ) -> void { other.emit( ); };
    }
    void connect( std::nullptr_t ) { m_callback = nullptr; }

    void emit( ) {
        if( m_callback ) {
            m_callback( );
        }
    }

private:
    std::function< void( ) > m_callback;
};
