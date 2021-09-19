#include "Url.h"

#include <algorithm>
#include <sstream>

using Iterator = std::string::const_iterator;

enum State {
    Schema,
    Host,
    Port,
    Path,
    QueryKey,
    QueryValue,
    Section
};

static inline std::runtime_error mkInvalidCharError( Iterator idx0, Iterator iter ) {
    std::stringstream ss;
    ss << "URL: Invalid char '";
    ss << *iter;
    ss << "' at position ";
    ss << std::distance( idx0, iter );

    return std::runtime_error( ss.str( ) );
}

static Iterator setSingle( Iterator begin, Iterator end, std::string &target, const char *badMessage ) {
    if( !target.empty( ) ) { throw std::runtime_error( badMessage ); }
    if( begin == end ) { throw std::runtime_error( badMessage ); }

    target = std::string( begin, end );

    return end + 1;
}

static Iterator addValue( Iterator begin, Iterator end, const std::string &key, std::map< std::string, std::string > &target, const char *badMessage ) {
    if( key.empty( ) ) { throw std::runtime_error( badMessage ); }
    auto iter = target.find( key );
    if( iter != target.end( ) ) { throw std::runtime_error( badMessage ); } // key duplicada.

    if( begin == end ) {
        target[key] = std::string( );
    } else {
        target[key] = std::string( begin, end );
    }

    return end + 1;
}

void Url::parse( Iterator begin, Iterator end ) {
    m_raw = std::string( begin, end );
    Iterator idx0 = begin;
    Iterator curr = begin;
    State state = State::Schema;
    std::string key;

    while( curr != end ) {
        char ch = *curr;

        switch( ch ) {
        case ' ': // Espacio. Caracter inválido en cualquier posición.
            throw mkInvalidCharError( idx0, curr );
            break;

        case ':': // schema, port.
            if( state == State::Schema ) {
                curr = setSchema( begin, curr, end );
                begin = curr;
                state = State::Host;
                continue;
            } else if( state == State::Host ) {
                curr = setSingle( begin, curr, m_host, "URL: Bad host" );
                begin = curr;
                state = State::Port;
                continue;                
            }

            throw mkInvalidCharError( idx0, curr );

        case '@': // auth inner host.
            if( state == State::Host ) {
                curr = setSingle( begin, curr, m_auth, "URL: Bad auth" );
                begin = curr;
                continue;
            }

            throw mkInvalidCharError( idx0, curr );

        case '/':
            if( state == Host ) {
                state = State::Path;
                curr = setSingle( begin, curr, m_host, "URL: Bad host" );
                begin = curr;
                continue;
            } else if( state == State::Port ) {
                state = State::Path;
                curr = setPort( begin, curr );
                begin = curr;
                continue;
            } else if( state == State::Path ) {
                curr = appendPath( begin, curr );
                begin = curr;
                continue;
            }

            throw mkInvalidCharError( idx0, curr );

        case ';': // params.
            throw mkInvalidCharError( idx0, curr );

        case '?': // query.
            if( state == Host ) {
                state = State::QueryKey;
                curr = setSingle( begin, curr, m_host, "URL: Bad host" );
                begin = curr;
                continue;
            } else if( state == State::Port ) {
                state = State::QueryKey;
                curr = setPort( begin, curr );
                begin = curr;
                continue;
            } else if( state == State::Path ) {
                state = State::QueryKey;
                curr = appendPath( begin, curr );
                begin = curr;
                continue;
            }

            throw mkInvalidCharError( idx0, curr );

        case '=':
            if( state == State::QueryKey ) {
                state = State::QueryValue;
                key = std::string( begin, curr );
                ++curr;
                begin = curr;
                continue;
            }

            throw mkInvalidCharError( idx0, curr );

        case '&': // query separator.
            if( state == State::QueryValue ) {
                state = QueryKey;
                curr = addValue( begin, curr, key, m_query, "URL: Bad query" );
                begin = curr;
                continue;
            }

            throw mkInvalidCharError( idx0, curr );

        case '#': // section;
            if( state == State::Host ) {
                state = State::Section;
                setSingle( begin, curr, m_host, "URL: Bad host" );
                begin = curr;
                continue;
            } else if( state == State::Port ) {
                state = State::Section;
                setPort( begin, curr );
                begin = curr;
                continue;
            } else if( state == State::Path ) {
                state = State::Section;
                appendPath( begin, curr );
                begin = curr;
                continue;
            } else if( state == State::QueryKey ) {
                state = State::Section;
                key = std::string( begin, curr );
                addValue( end, curr, key, m_query, "URL: Bad query" );
                begin = curr;
                continue;
            } else if( state == State::QueryValue ) {
                state = State::Section;
                addValue( begin, curr, key, m_query, "URL: Bad query" );
                begin = curr;
                continue;
            } else if( state == State::Section ) {
                ++curr;
                continue;
            }

            throw mkInvalidCharError( idx0, curr );

        default:
            ++curr;
        }
    }

    switch( state ) {
    case State::Schema:
        throw std::runtime_error( "URL: Bad Url" );

    case State::Host:
        setSingle( begin, end, m_host, "URL: Bad host" );
        break;

    case State::Port:
        setPort( begin, end );
        break;

    case State::Path:
        appendPath( begin, end );
        break;

    case State::QueryKey:
        key = std::string( begin, end );
        addValue( end, end, key, m_query, "URL: Bad query" );
        break;

    case State::QueryValue:
        addValue( begin, end, key, m_query, "URL: Bad query" );
        break;

    case State::Section:
        setSingle( begin, end, m_section, "URL: Bad section" );
    }

    if( m_path.empty( ) ) {
        m_path.emplace_back( );
    }
}

Iterator Url::setSchema( Iterator begin, Iterator end, Iterator final ) {
    static const char *BadMessage = "URL: Bad schema";

    if( !m_schema.empty( ) ) { throw std::runtime_error( BadMessage ); }
    if( begin == end ) { throw std::runtime_error( BadMessage ); }

    auto sep = end + 1;
    if( ( sep == final ) || ( *sep != '/' ) ) { throw std::runtime_error( BadMessage ); }
    ++sep;
    if( ( sep == final ) || ( *sep != '/' ) ) { throw std::runtime_error( BadMessage ); }

    m_schema = std::string( begin, end );

    return sep + 1;
}

Iterator Url::setPort( Iterator begin, Iterator end ) {
    static const char *BadMessage = "URL: Bad port";

    if( m_port != -1 ) { throw std::runtime_error( BadMessage ); }
    if( begin == end ) { throw std::runtime_error( BadMessage ); }

    if( std::any_of( begin, end, []( char ch ) -> bool {
        return ( ch < '0' ) || ( ch > '9' );
    } ) ) {
        throw std::runtime_error( BadMessage );
    }

    if( std::distance( begin, end ) > 5 ) {
        throw std::runtime_error( BadMessage );
    }

    int result = sscanf( &( *begin ), "%i", &m_port );

    if( result == 0 ) {
        throw std::runtime_error( BadMessage );
    }

    if( ( m_port < 1 ) || ( m_port > 65535 ) ) {
        throw std::runtime_error( BadMessage );
    }

    return end + 1;
}

Iterator Url::appendPath( Iterator begin, Iterator end ) {
    if( begin == end ) {
        m_path.emplace_back( );
    } else {
        m_path.emplace_back( begin, end );
    }

    return end + 1;
}
