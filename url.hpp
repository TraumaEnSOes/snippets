#ifndef URL_HPP
#define URL_HPP

#include <exception>
#include <map>
#include <string>
#include <vector>

class Url {
public:
    Url( ) = default;
    Url( const Url & ) = default;
    Url( Url && ) = default;
    Url( const std::string &str ) {
        std::exception_ptr errPtr;

        try {
            parse( str.cbegin( ), str.cend( ) );
        } catch( ... ) {
            errPtr = std::current_exception( );
        }

        if( errPtr ) {
            clear( );
            std::rethrow_exception( errPtr );
        }
    }
    Url &operator=( const Url & ) = default;
    Url &operator=( Url && ) = default;
    Url &operator=( const std::string &str ) {
        std::exception_ptr errPtr;

        clear( );

        try {
            parse( str.cbegin( ), str.cend( ) );
        } catch( ... ) {
            errPtr = std::current_exception( );
        }

        if( errPtr ) {
            clear( );
            std::rethrow_exception( errPtr );
        }

        return *this;
    }

    bool empty( ) const { return m_path.empty( ); }
    bool operator!( ) const { return m_path.empty( ); }
    operator bool( ) const { return !m_path.empty( ); }

    const std::string raw( ) const { return m_raw; }
    const std::string &schema( ) const { return m_schema; }
    const std::string &auth( ) const { return m_auth; }
    const std::string &host( ) const { return m_host; }
    int port( ) const { return m_port; }
    const std::vector< std::string > &path( ) const { return m_path; }
    const std::map< std::string, std::string > &query( ) const { return m_query; }
    const std::string &section( ) const { return m_section; }

    void clear( ) {
        m_raw.clear( );
        m_schema.clear( );
        m_auth.clear( );
        m_host.clear( );
        m_port = -1;
        m_path.clear( );
        m_query.clear( );
        m_section.clear( );
    }

private:
    using Iterator = std::string::const_iterator;

    void parse( Iterator begin, Iterator end );
    Iterator setSchema( Iterator begin, Iterator end, Iterator final );
    Iterator setPort( Iterator begin, Iterator end );
    Iterator appendPath( Iterator begin, Iterator end );

    std::string m_raw;
    int m_port = -1;
    std::string m_schema;
    std::string m_auth;
    std::string m_host;
    std::vector< std::string > m_path;
    std::map< std::string, std::string > m_query;
    std::string m_section;
};

#endif
